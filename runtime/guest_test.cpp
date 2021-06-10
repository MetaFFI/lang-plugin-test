#include "guest_test.h"
#include <include/args_helpers.h>
#include <string>
#include <cstdio>


void* xllr_handle = nullptr;

void (*pcall)(const char*, uint32_t,
              int64_t,
              void**, uint64_t,
              void**, uint64_t,
              char**, uint64_t*
) = nullptr;

int64_t (*pload_function)(const char*, uint32_t,
                          const char*, uint32_t,
                          int64_t,
                          char**, uint32_t*) = nullptr;

#ifdef _WIN32 //// --- START WINDOWS ---
#include <Windows.h>
void get_last_error_string(DWORD err, char** out_err_str)
{
    DWORD bufLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                 FORMAT_MESSAGE_FROM_SYSTEM |
							     FORMAT_MESSAGE_IGNORE_INSERTS,
							     NULL,
							     err,
						         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						         (LPTSTR) out_err_str,
						         0,
						         NULL );

    // TODO: out_err_str should get cleaned up!
}

void* load_library(const char* name, char** out_err)
{
	void* handle = LoadLibraryA(name);
	if(!handle)
	{
		get_last_error_string(GetLastError(), out_err);
	}

	return handle;
}

const char* free_library(void* lib) // return error string. null if no error.
{
	if(!lib)
	{
		return NULL;
	}

	if(!FreeLibrary(lib))
	{
		char* out_err;
		get_last_error_string(GetLastError(), &out_err);
		return out_err;
	}

	return NULL;
}

void* load_symbol(void* handle, const char* name, char** out_err)
{
	void* res = GetProcAddress(handle, name);
	if(!res)
	{
		get_last_error_string(GetLastError(), out_err);
		return NULL;
	}

	return res;
}

#else // ------ START POSIX ----
#include <dlfcn.h>
#include <cstring>

void* load_library(const char* name, char** out_err)
{
	void* handle = dlopen(name, RTLD_NOW);
	if(!handle)
	{
		*out_err = dlerror();
	}
	
	return handle;
}

const char* free_library(void* lib)
{
	if(dlclose(lib))
	{
		return dlerror();
	}
	
	return nullptr;
}

void* load_symbol(void* handle, const char* name, char** out_err)
{
	void* res = dlsym(handle, name);
	if(!res)
	{
		*out_err = dlerror();
		return nullptr;
	}
	
	return res;
}

#endif // ------- END POSIX -----

int64_t load_function(const char* runtime_plugin, uint32_t runtime_plugin_len,
                      const char* function_path, uint32_t function_path_len,
                      int64_t function_id_opt,
                      char** out_err, uint32_t* out_err_len)
{
	return pload_function(runtime_plugin, runtime_plugin_len,
	                      function_path, function_path_len,
	                      function_id_opt,
	                      out_err, out_err_len);
}

void call(
		const char* runtime_plugin_name, uint32_t runtime_plugin_name_len,
		int64_t function_id,
		void** parameters, uint64_t parameters_length,
		void** return_values, uint64_t return_values_length,
		char** out_err, uint64_t* out_err_len
)
{
	pcall(runtime_plugin_name, runtime_plugin_name_len,
	      function_id,
	      parameters, parameters_length,
	      return_values, return_values_length,
	      out_err, out_err_len);
}

const char* load_xllr_api()
{
	char* out_err = NULL;
	pcall = (void (*)(const char*, uint32_t,
			int64_t,
			void**, uint64_t,
			void**, uint64_t,
			char**, uint64_t*))load_symbol(xllr_handle, "call", &out_err);
	if(!pcall)
	{
		return out_err;
	}
	
	pload_function = (int64_t (*)(const char*, uint32_t,
								const char*, uint32_t,
								int64_t,
								char**, uint32_t*))load_symbol(xllr_handle, "load_function", &out_err);
	if(!pload_function)
	{
		return out_err;
	}
	
	return nullptr;
}

bool load_xllr()
{
	const char* openffi_home = getenv("OPENFFI_HOME");
	
	char xllr_name[250] = {0};

#ifdef _WIN32
	const char* extension = "dll";
#elifdef __APPLE__
	const char* extension = "dylib";
#else
	const char* extension = "so";
#endif
	
	sprintf(xllr_name, "%s/xllr.%s", openffi_home, extension);
	
	char err[1000] = {0};
	
	xllr_handle = load_library(xllr_name, reinterpret_cast<char **>(&err));
	
	if(!xllr_handle)
	{
		printf("Failed to load XLLR: %s", err);
		return false;
	}
	
	const char* errmsg = load_xllr_api();
	if(errmsg)
	{
		printf("Failed to load XLLR functions: %s\n", errmsg);
		return false;
	}
	
	
	return true;
}

// 0 - success
// otherwise - failed
int test_guest(const char* lang_plugin, const char* function_path)
{
	printf("test_guest - loading XLLR\n");
	if(!load_xllr())
	{
		return 1;
	}
	
	char* err = nullptr;
	uint64_t err_len = 0;
	
	/*
	std::string language_plugin(lang_plugin);
	load_runtime_plugin(language_plugin.c_str(), language_plugin.length(), &err, reinterpret_cast<uint32_t *>(&err_len));
	if(err != nullptr){
		printf("Failed to load plugin: %s. Error: %s\n", lang_plugin, err);
		return 1;
	}
	*/
	
	printf("test_guest - loading function\n");
	int64_t function_id = load_function(lang_plugin, strlen(lang_plugin), function_path, strlen(function_path), -1, &err, reinterpret_cast<uint32_t *>(&err_len));
	
	if(err != nullptr){
		printf("Failed to load runtime \"%s\" or function \"%s\". Error: %s\n", lang_plugin, function_path, err);
		return 1;
	}
	
	/* This function calls guest with the parameters (in that order):
	    double = 3.141592
	    float = 2.71f
	    
	    int8 = -10
	    int16 = -20
	    int32 = -30
	    int64 = -40
	    
	    uint8 = 50
	    uint16 = 60
	    uint32 = 70
	    uint64 = 80
	    
	    bool = 1
	    
	    string = "This is an input"
	    string[] = {"element one", "element two"}
	    
	    bytes = {2, 4, 6, 8, 10}
	*/
	
	printf("preparing parameters\n");
	void** parameters = alloc_args_buffer(18);
	void** return_values = alloc_args_buffer(3);
	
	openffi_float64 p1 = 3.141592;
	set_arg(parameters, 0, &p1);
	openffi_float32 p2 = 2.71f;
	set_arg(parameters, 1, &p2);
	
	openffi_int8 p3 = -10;
	set_arg_openffi_int8(parameters, 2, &p3);
	openffi_int16 p4 = -20;
	set_arg_openffi_int16(parameters, 3, &p4);
	openffi_int32 p5 = -30;
	set_arg_openffi_int32(parameters, 4, &p5);
	openffi_int64 p6 = -40;
	set_arg_openffi_int64(parameters, 5, &p6);
	
	openffi_uint8 p7 = 50;
	set_arg_openffi_uint8(parameters, 6, &p7);
	openffi_uint16 p8 = 60;
	set_arg_openffi_uint16(parameters, 7, &p8);
	openffi_uint32 p9 = 70;
	set_arg_openffi_uint32(parameters, 8, &p9);
	openffi_uint64 p10 = 80;
	set_arg_openffi_uint64(parameters, 9, &p10);
	
	openffi_bool p11 = 1;
	set_arg_openffi_bool(parameters, 10, &p11);
	
	const char* p12 = "This is an input";
	openffi_size p12_len = strlen("This is an input");
	set_arg_openffi_string(parameters, 11, (char*)p12, &p12_len);
	
	const char* p13[] = {"element one", "element two"};
	openffi_size p13_sizes[] = {strlen("element one"), strlen("element two")};
	openffi_size p13_len = 2;
	set_arg_openffi_string_array(parameters, 13, const_cast<openffi_string *>(p13), p13_sizes, &p13_len);
	
	openffi_uint8 p14[] = {2, 4, 6, 8, 10};
	openffi_size p14_len = 5;
	set_arg_openffi_uint8_array(parameters, 16, reinterpret_cast<openffi_uint8 *>(&p14), &p14_len);
	
	printf("calling guest function\n");
	call(lang_plugin, strlen(lang_plugin),
	     function_id,
	     parameters, 18,
	     return_values, 3,
	     &err, reinterpret_cast<uint64_t *>(&err_len)
	);
	
	printf("checking for errors\n");
	if(err)
	{
		printf("Error returned from guest: %s\n", std::string(err, err_len).c_str());
		return 2;
	}
	
	printf("checking return values\n");
	
	/* Expects return of:
	    String[] = {"return one", "return two"}
	*/
	openffi_size* r1_sizes;
	openffi_size r1_len;
	openffi_string* r1 = get_arg_openffi_string_array(return_values, 0, &r1_sizes, &r1_len);
	
	if(r1_len != 2)
	{
		printf("returned array is not of size 2. size:%ld\n", r1_len);
		return 3;
	}
	
	if(std::string(r1[0], r1_sizes[0]) != "return one")
	{
		printf("r1[0] is not \"return one\"\n");
		return 4;
	}
	
	if(std::string(r1[1], r1_sizes[1]) != "return two")
	{
		printf("r1[0] is not \"return two\"\n");
		return 5;
	}
	
	return 0;
}

