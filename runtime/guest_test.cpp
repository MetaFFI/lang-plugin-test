#include "guest_test.h"
#include <runtime/common_data_type_helper_loader.h>
#include <runtime/common_data_type_parser.h>
#include <string>
#include <cstdio>
#include <cstring>

using namespace openffi::utils;

const char* guest_idl = R"(idl_filename": "test","idl_extension": ".proto","idl_filename_with_extension": "test.proto","idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": {"openffi_function_path": "package=main","openffi_target_language": "python3"},"functions": [{"name": "f1","comment": "f1 comment\nparam1 comment\n","tags": {"openffi_function_path": "function=f1"},"path_to_foreign_function": {"module": "$PWD/temp","package": "GoFuncs","function": "f1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{ "name": "p1", "type": "float64", "comment": "= 3.141592", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p2", "type": "float32", "comment": "= 2.71", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p3", "type": "int8", "comment": "= -10", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p4", "type": "int16", "comment": "= -20", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p5", "type": "int32", "comment": "= -30", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p6", "type": "int64", "comment": "= -40", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p7", "type": "uint8", "comment": "= 50", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p8", "type": "uint16", "comment": "= 60", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p9", "type": "uint32", "comment": "= 70", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p10", "type": "uint64", "comment": "= 80", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p11", "type": "bool", "comment": "= true", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p12", "type": "string", "comment": "= This is an input", "tags": null, "is_array": false, "pass_method": "" },{ "name": "p13", "type": "string", "comment": "= {element one, element two}", "tags": null, "is_array": true, "pass_method": "" },{ "name": "p14", "type": "uint8", "comment": "= {2, 4, 6, 8, 10}", "tags": null, "is_array": true, "pass_method": "" }],"return_values": [{"name": "r1","type": "string","comment": "= {return one, return two}","tags": null,"is_array": true,"pass_method": ""}]}]}]});)";

/*
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

*/

// 0 - success
// otherwise - failed
int test_guest(const char* lang_plugin, const char* function_path)
{
    load_xllr();
    load_args_helpers();
	
	char* err = nullptr;
	uint64_t err_len = 0;
	
	printf("test_guest - loading function\n");
	int64_t function_id = xllr_load_function(lang_plugin, strlen(lang_plugin), function_path, strlen(function_path), -1, &err, reinterpret_cast<uint32_t *>(&err_len));
	
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

	    matrix = { {10, 20, 30}, {40, 50, 60} }
	*/
	
	printf("preparing parameters\n");
	void** parameters = alloc_args_buffer(38);
	void** return_values = alloc_args_buffer(38);
	
	int index = 0;
	
	openffi_float64 p1 = 3.141592;
	index = set_arg_openffi_float64(parameters, index, &p1);
	openffi_float32 p2 = 2.71f;
	index = set_arg_openffi_float32(parameters, index, &p2);
	
	openffi_int8 p3 = -10;
	index = set_arg_openffi_int8(parameters, index, &p3);
	openffi_int16 p4 = -20;
	index = set_arg_openffi_int16(parameters, index, &p4);
	openffi_int32 p5 = -30;
	index = set_arg_openffi_int32(parameters, index, &p5);
	openffi_int64 p6 = -40;
	index = set_arg_openffi_int64(parameters, index, &p6);
	
	openffi_uint8 p7 = 50;
	index = set_arg_openffi_uint8(parameters, index, &p7);
	openffi_uint16 p8 = 60;
	index = set_arg_openffi_uint16(parameters, index, &p8);
	openffi_uint32 p9 = 70;
	index = set_arg_openffi_uint32(parameters, index, &p9);
	openffi_uint64 p10 = 80;
	index = set_arg_openffi_uint64(parameters, index, &p10);
	
	openffi_bool p11 = 1;
	index = set_arg_openffi_bool(parameters, index, &p11);
	
	const char* p12 = "This is an input";
	openffi_size p12_len = strlen("This is an input");
	index = set_arg_openffi_string(parameters, index, (char*)p12, &p12_len);
	
	const char* p13[] = {"element one", "element two"};
	openffi_size p13_sizes[] = {strlen("element one"), strlen("element two")};
	openffi_size p13_dimentions[] = { 2 };
	openffi_size p13_dimensions_length = 1;
	index = set_arg_openffi_string_array(parameters, index, const_cast<openffi_string*>(p13), p13_sizes, p13_dimentions, &p13_dimensions_length);
	
	openffi_uint8 p14[] = {2, 4, 6, 8, 10};
	openffi_size p14_dimensions[] = {5};
	openffi_size p14_dimensions_length = 1;
	index = set_arg_openffi_uint8_array(parameters, index, reinterpret_cast<openffi_uint8 *>(p14), p14_dimensions, &p14_dimensions_length);
	
	// matrix
	uint32_t** resmatrix = new uint32_t*[2]{
			new uint32_t[3]{ 10, 20, 30 },
			new uint32_t[3]{ 40, 50, 60 },
	};
	openffi_size* matrix_dimensions = new openffi_size[2]{ 2, 3 };
	openffi_size p15_dimensions_length = 2;
	index = set_arg_openffi_uint32_array(parameters, index, (uint32_t*)resmatrix, matrix_dimensions, &p15_dimensions_length);
	
	printf("calling guest function\n");
	xllr_call(lang_plugin, strlen(lang_plugin),
	     function_id,
	     parameters, 38,
	     return_values, 38,
	     &err, reinterpret_cast<uint64_t *>(&err_len)
	);
	
	printf("checking for errors\n");
	if(err)
	{
		printf("Error returned from guest: %s\n", std::string(err, err_len).c_str());
		return 2;
	}
	
	printf("checking return values TODO!!!!\n");
	
	/* Expects return of:
	    double = 0.57721
	    float = 3.359f

	    int8 = -11
	    int16 = -21
	    int32 = -31
	    int64 = -41

	    uint8 = 51
	    uint16 = 61
	    uint32 = 71
	    uint64 = 81

	    bool = 1

	    string = "This is an output"
	    string[] = ["return one", "return two"]

	    bytes = [20, 40, 60, 80, 100]

	    matrix = [ [11,21,31], [41,51,61] ]
	*/

	openffi_float64 r1;
	openffi_float32 r2;
	openffi_int8 r3;
	openffi_int16 r4;
	openffi_int32 r5;
	openffi_int64 r6;
	openffi_uint8 r7;
	openffi_uint16 r8;
	openffi_uint32 r9;
	openffi_uint64 r10;
	openffi_bool r11;
	openffi_string r12;
	openffi_size r12_len;
	string_n_array_wrapper<openffi_string> r13;
	numeric_n_array_wrapper<openffi_uint8> r14;
	numeric_n_array_wrapper<openffi_uint32> r15;

	common_data_type_parse_callbacks cb
	(
			[&](const openffi_float32& p){ r2 = p; }, [&](const numeric_n_array_wrapper<openffi_float32>& p){},
			[&](const openffi_float64& p){ r1 = p; }, [&](const numeric_n_array_wrapper<openffi_float64>& p){},
			[&](const openffi_int8& p){ r3 = p; }, [&](const numeric_n_array_wrapper<openffi_int8>& p){},
			[&](const openffi_int16& p){ r4 = p; }, [&](const numeric_n_array_wrapper<openffi_int16>& p){},
			[&](const openffi_int32& p){ r5 = p; }, [&](const numeric_n_array_wrapper<openffi_int32>& p){},
			[&](const openffi_int64& p){ r6 = p; }, [&](const numeric_n_array_wrapper<openffi_int64>& p){},
			[&](const openffi_uint8& p){ r7 = p; }, [&](const numeric_n_array_wrapper<openffi_uint8>& p){ r14 = p; },
			[&](const openffi_uint16& p){ r8 = p; }, [&](const numeric_n_array_wrapper<openffi_uint16>& p){},
			[&](const openffi_uint32& p){ r9 = p; }, [&](const numeric_n_array_wrapper<openffi_uint32>& p){ r15 = p; },
			[&](const openffi_uint64& p){ r10 = p; }, [&](const numeric_n_array_wrapper<openffi_uint64>& p){},
			[&](const openffi_bool& p){ r11 = p; }, [&](const numeric_n_array_wrapper<openffi_bool>& p){},
			[&](const openffi_string& p, openffi_size s){ r12 = p; r12_len = s; }, [&](const string_n_array_wrapper<openffi_string>& p){ r13 = p; },
			[&](const openffi_string8& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string8>& p){},
			[&](const openffi_string16& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string16>& p){},
			[&](const openffi_string32& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string32>& p){}
	);

	common_data_type_parser parser(return_values, 38, cb);
	parser.parse();

#define check_num_var(var_name, expected)\
	if((var_name) != (expected)){ throw std::runtime_error(#var_name" of type is not "#expected); }

	// check parameters
	check_num_var(r1, 0.57721);
	check_num_var(r2, 3.359f);

	check_num_var(r3, -11);
	check_num_var(r4, -21);
	check_num_var(r5, -31);
	check_num_var(r6, -41);

	check_num_var(r7, 51);
	check_num_var(r8, 61);
	check_num_var(r9, 71);
	check_num_var(r10, 81);

	check_num_var(r11, 1);

	std::string r12_str(r12, r12_len);
	if(r12_str != "This is an output"){
		std::stringstream ss;
		ss << R"(r12 of type string is not "This is an input", but ")" << r12_str.c_str() << "\"";
		throw std::runtime_error(ss.str().c_str());
	}

	// string[]
	if(r13.get_dimensions_count() != 1){
		throw std::runtime_error("p13 array is not of 1 dimension");
	}

	if(r13.get_dimension_length(0) != 2){
		throw std::runtime_error("p13 array length of type openffi_size is not 2");
	}

	openffi_string r13_elem1_pchar;
	openffi_size r13_elem1_size;
	openffi_size arr_index[] = {0};
	r13.get_elem_at(arr_index, 1, &r13_elem1_pchar, &r13_elem1_size);
	std::string r13_elem1(r13_elem1_pchar, r13_elem1_size);
	if(r13_elem1 != "return one")
	{
		std::stringstream ss;
		ss << R"(r13_elem1 of type string is not "return one": ")" << r13_elem1 << "\"";
		throw std::runtime_error(ss.str().c_str());
	}

	openffi_string r13_elem2_pchar;
	openffi_size r13_elem2_size;
	arr_index[0] = 1;
	r13.get_elem_at(arr_index, 1, &r13_elem2_pchar, &r13_elem2_size);
	std::string r13_elem2(r13_elem2_pchar, r13_elem2_size);
	if(r13_elem2 != "return two"){
		throw std::runtime_error("r13_elem2 of type string is not \"return two\"");
	}

	// bytes
	if(!r14.is_simple_array() || r14.get_simple_array_length() != 5){
		throw std::runtime_error("p14_len of type int64_t is not 5");
	}

	arr_index[0] = 0; if(r14.get_elem_at(arr_index, 1) != 20){ throw std::runtime_error("r14[0] of type unsigned char is not 20"); }
	arr_index[0] = 1; if(r14.get_elem_at(arr_index, 1) != 40){ throw std::runtime_error("r14[1] of type unsigned char is not 40"); }
	arr_index[0] = 2; if(r14.get_elem_at(arr_index, 1) != 60){ throw std::runtime_error("r14[2] of type unsigned char is not 60"); }
	arr_index[0] = 3; if(r14.get_elem_at(arr_index, 1) != 80){ throw std::runtime_error("r14[3] of type unsigned char is not 80"); }
	arr_index[0] = 4; if(r14.get_elem_at(arr_index, 1) != 100){ throw std::runtime_error("r14[4] of type unsigned char is not 100"); }

	// matrix[2][3]
	if(r15.dimensions_length != 2){
		throw std::runtime_error("p15 matrix is not of 2 dimensions");
	}

	if(r15.dimensions[0] != 2){
		throw std::runtime_error("p15 first dimension is not 2");
	}

	if(r15.dimensions[1] != 3){
		throw std::runtime_error("p15 second dimension is not 3");
	}

#define set_index(arr, i, j) arr[0] = i; (arr)[1] = j;

	openffi_size mat_index[] = { 0, 0 };
	set_index(mat_index, 0, 0); if(r15.get_elem_at(mat_index, 2) != 11){ throw std::runtime_error("r15[0][0] of type unsigned char is not 11"); }
	set_index(mat_index, 0, 1); if(r15.get_elem_at(mat_index, 2) != 21){ throw std::runtime_error("r15[0][1] of type unsigned char is not 21"); }
	set_index(mat_index, 0, 2); if(r15.get_elem_at(mat_index, 2) != 31){ throw std::runtime_error("r15[0][2] of type unsigned char is not 31"); }
	set_index(mat_index, 1, 0); if(r15.get_elem_at(mat_index, 2) != 41){ throw std::runtime_error("r15[1][0] of type unsigned char is not 41"); }
	set_index(mat_index, 1, 1); if(r15.get_elem_at(mat_index, 2) != 51){ throw std::runtime_error("r15[1][1] of type unsigned char is not 51"); }
	set_index(mat_index, 1, 2); if(r15.get_elem_at(mat_index, 2) != 61){ throw std::runtime_error("r15[1][2] of type unsigned char is not 61"); }

	err_len = 0;
	xllr_free_runtime_plugin("xllr.test", strlen("xllr.test"), &err, reinterpret_cast<uint32_t*>(&err_len));
	free_xllr();

	return 0;
}

