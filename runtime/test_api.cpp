#include <runtime/runtime_plugin_api.h>
#include <utils/scope_guard.hpp>
#include <boost/filesystem.hpp>
#include <utils/foreign_function.h>
#include <cstdint>
#include <runtime/openffi_primitives.h>
#include <sstream>

using namespace openffi::utils;

#define handle_err(err, err_len, desc) \
	*err_len = strlen( desc ); \
	*err = (char*)malloc(*err_len + 1); \
	strcpy(*err, desc ); \
	memset((*err+*err_len), 0, 1);

#define catch_err(err, err_len, desc) \
catch(std::exception& exc) \
{\
	handle_err(err, err_len, desc);\
}

#define handle_err_str(err, err_len, descstr) \
	*err_len = descstr.length(); \
	*err = (char*)malloc(*err_len + 1); \
	descstr.copy(*err, *err_len, 0); \
	memset((*err+*err_len), 0, 1);


#define TRUE 1
#define FALSE 0

std::map<int64_t, std::string> loaded_functions;
int64_t function_id = 0;

//--------------------------------------------------------------------
void load_runtime(char** /*err*/, uint32_t* /*err_len*/){ /* No runtime to load */ }
//--------------------------------------------------------------------
void free_runtime(char** /*err*/, uint32_t* /*err_len*/){ /* No runtime free */ }
//--------------------------------------------------------------------
int64_t load_function(const char* function_path, uint32_t function_path_len, char** out_err, uint32_t* out_err_len)
{
	try
	{
		if(function_path_len == 0)
		{
			throw std::runtime_error("No function path");
		}
		
		int64_t curfunc_id = function_id;
		loaded_functions[curfunc_id] = std::string(function_path, function_path_len);
		function_id++;
		
		printf("Loaded function path %s. Function ID: %ld\n", loaded_functions[curfunc_id].c_str(), curfunc_id);
		
		return curfunc_id;
	}
	catch_err((char**)out_err, out_err_len, exc.what());
	
	return -1;
}
//--------------------------------------------------------------------
void free_function(int64_t func_id, char** /*err*/, uint32_t* /*err_len*/)
{
	auto it = loaded_functions.find(func_id);
	if(it == loaded_functions.end())
	{
		printf("Function ID %ld failed to free as it was not found\n", func_id);
	}
	
	loaded_functions.erase(it);
	printf("Function ID %ld freed\n", func_id);
}
//--------------------------------------------------------------------
void call(
	int64_t func_id,
	char** out_err, uint64_t *out_err_len,
	uint64_t args_len,
	va_list params
)
{ // TODO: check args_len
	try
	{
		/* This function expects the parameters (in that order):
		    func_id = 0
		    
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
		
		if(func_id != 0){
			throw std::runtime_error("func_id is not 0");
		}

#define check_num_var(var_name, type, expected) \
		type var_name = va_arg(params, type);    \
		if((var_name) != (expected)){ \
			throw std::runtime_error(#var_name" of type "#type" is not "#expected);\
		}
		
		
		// read parameters
		check_num_var(p1, openffi_float64, 3.141592);
		check_num_var(p2, openffi_float64/*openffi_float32*/, 2.71f); // ellipsis promotes to double
		
		check_num_var(p3, openffi_int32/*openffi_int8*/, -10); // ellipsis promotes to int32_t
		check_num_var(p4, openffi_int32/*openffi_int16*/, -20); // ellipsis promotes to int32_t
		check_num_var(p5, openffi_int32, -30);
		check_num_var(p6, openffi_int64, -40);
		
		check_num_var(p7, openffi_uint32/*openffi_uint8*/, 50); // ellipsis promotes to uint32_t
		check_num_var(p8, openffi_uint32/*openffi_uint16*/, 60); // ellipsis promotes to uint32_t
		check_num_var(p9, openffi_uint32, 70);
		check_num_var(p10, openffi_uint64, 80);
		
		check_num_var(p11, openffi_int32/*openffi_bool*/, 1); // ellipsis promotes to int32_t
		
		openffi_string p12 = va_arg(params, openffi_string);
		openffi_size p12_len = va_arg(params, openffi_size);
		std::string p12_str(p12, p12_len);
		if(p12_str != "This is an input"){
			throw std::runtime_error("p12 of type string is not \"This is an input\"");
		}
		
		// string[]
		openffi_string* p13 = va_arg(params, openffi_string*);
		openffi_size* p13_sizes = va_arg(params, openffi_size*);
		openffi_size p13_len = va_arg(params, openffi_size);
		if(p13_len != 2){
			throw std::runtime_error("p13_len of type int64_t is not 2");
		}
		
		std::string p13_elem1(p13[0], p13_sizes[0]);
		if(p13_elem1 != "element one"){
			std::stringstream ss;
			ss << "p13_elem1 of type string is not \"element one\": \"" << p13_elem1 << "\"";
			throw std::runtime_error(ss.str().c_str());
		}
		
		std::string p13_elem2(p13[1], p13_sizes[1]);
		if(p13_elem2 != "element two"){
			throw std::runtime_error("p13_elem2 of type string is not \"element two\"");
		}
		
		// bytes
		const openffi_uint8* p14 = va_arg(params, const openffi_uint8*);
		openffi_size p14_len = va_arg(params, openffi_size);
		if(p14_len != 5){
			throw std::runtime_error("p14_len of type int64_t is not 5");
		}
		
		if(p14[0] != 2){ throw std::runtime_error("p14[0] of type unsigned char is not 2"); }
		if(p14[1] != 4){ throw std::runtime_error("p14[1] of type unsigned char is not 4"); }
		if(p14[2] != 6){ throw std::runtime_error("p14[2] of type unsigned char is not 6"); }
		if(p14[3] != 8){ throw std::runtime_error("p14[3] of type unsigned char is not 8"); }
		if(p14[4] != 10){ throw std::runtime_error("p14[4] of type unsigned char is not 10"); }
		
		
		/* This function returns:
		    String[] = {"return one", "return two"}
		 */
		openffi_string* r1 = va_arg(params, openffi_string*);
		openffi_size** r1_sizes = va_arg(params, openffi_size**);
		openffi_size* r1_len = va_arg(params, openffi_size*);
		
		*r1_len = 2;
		
		const char* r1_elem1 = "return one";
		int64_t r1_elem1_len = strlen(r1_elem1);
		
		const char* r1_elem2 = "return two";
		int64_t r1_elem2_len = strlen(r1_elem2);
		
		const char* arr[2] = {r1_elem1, r1_elem2};
		int64_t arr_sizes[2] = {r1_elem1_len, r1_elem2_len};
		*r1 = (openffi_string)&arr;
		*r1_sizes = (openffi_size*)&arr_sizes;
	}
	catch_err((char**)out_err, out_err_len, exc.what());
}
//--------------------------------------------------------------------
