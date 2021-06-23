#include <runtime/runtime_plugin_api.h>
#include <utils/scope_guard.hpp>
#include <boost/filesystem.hpp>
#include <runtime/common_data_type_helper_loader.h>
#include <runtime/common_data_type_parser.h>
#include <sstream>

using namespace openffi::utils;

const char* host_idl = R"(idl_filename": "test","idl_extension": ".proto","idl_filename_with_extension": "test.proto","idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": {"openffi_function_path": "package=main","openffi_target_language": "python3"},"functions": [{"name": "f1","comment": "f1 comment\nparam1 comment\n","tags": {"openffi_function_path": "function=f1"},"path_to_foreign_function": {"module": "$PWD/temp","package": "GoFuncs","function": "f1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{ "name": "p1", "type": "float64", "comment": "= 3.141592", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p2", "type": "float32", "comment": "= 2.71", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p3", "type": "int8", "comment": "= -10", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p4", "type": "int16", "comment": "= -20", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p5", "type": "int32", "comment": "= -30", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p6", "type": "int64", "comment": "= -40", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p7", "type": "uint8", "comment": "= 50", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p8", "type": "uint16", "comment": "= 60", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p9", "type": "uint32", "comment": "= 70", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p10", "type": "uint64", "comment": "= 80", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p11", "type": "bool", "comment": "= true", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p12", "type": "string", "comment": "= This is an input", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p13", "type": "string", "comment": "= {element one, element two}", "tags": null, "dimensions": 1, "pass_method": "" },{ "name": "p14", "type": "uint8", "comment": "= {2, 4, 6, 8, 10}", "tags": null, "dimensions": 1, "pass_method": "" }],"return_values": [{"name": "r1","type": "string","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]});)";

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
	void** parameters, uint64_t parameters_length,
	void** return_values, uint64_t return_values_length,
	char** out_err, uint64_t *out_err_len
)
{
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
		    string[] = ["element one", "element two"]
		    
		    bytes = [2, 4, 6, 8, 10]
		    
		    matrix = [ [1,2,3], [4,5,6] ]
		*/
		
		if(func_id != 0){
			throw std::runtime_error("func_id is not 0");
		}

		openffi_float64 p1;
		openffi_float32 p2;
		openffi_int8 p3;
		openffi_int16 p4;
		openffi_int32 p5;
		openffi_int64 p6;
		openffi_uint8 p7;
		openffi_uint16 p8;
		openffi_uint32 p9;
		openffi_uint64 p10;
		openffi_bool p11;
		openffi_string p12;
		openffi_size p12_len;
		string_n_array_wrapper<openffi_string> p13;
		numeric_n_array_wrapper<openffi_uint8> p14;
		numeric_n_array_wrapper<openffi_uint32> p15;
		
		
#define read_num_param(type, var_name) [&](const type& p){ (var_name) = p; }, [&](const numeric_array_wrapper<type>& p){ (var_name) = p; }
#define read_str_param(type, var_name) [&](const type& p, openffi_size s){ (var_name) = p; var_name##_len = s; }, [&](const string_array_wrapper<type>& p){ (var_name) = p; }
		
		common_data_type_parse_callbacks cb
		(
			[&](const openffi_float32& p){ p2 = p; }, [&](const numeric_n_array_wrapper<openffi_float32>& p){},
			[&](const openffi_float64& p){ p1 = p; }, [&](const numeric_n_array_wrapper<openffi_float64>& p){},
			[&](const openffi_int8& p){ p3 = p; }, [&](const numeric_n_array_wrapper<openffi_int8>& p){},
			[&](const openffi_int16& p){ p4 = p; }, [&](const numeric_n_array_wrapper<openffi_int16>& p){},
			[&](const openffi_int32& p){ p5 = p; }, [&](const numeric_n_array_wrapper<openffi_int32>& p){},
			[&](const openffi_int64& p){ p6 = p; }, [&](const numeric_n_array_wrapper<openffi_int64>& p){},
			[&](const openffi_uint8& p){ p7 = p; }, [&](const numeric_n_array_wrapper<openffi_uint8>& p){ p14 = p; },
			[&](const openffi_uint16& p){ p8 = p; }, [&](const numeric_n_array_wrapper<openffi_uint16>& p){},
			[&](const openffi_uint32& p){ p9 = p; }, [&](const numeric_n_array_wrapper<openffi_uint32>& p){ p15 = p; },
			[&](const openffi_uint64& p){ p10 = p; }, [&](const numeric_n_array_wrapper<openffi_uint64>& p){},
			[&](const openffi_bool& p){ p11 = p; }, [&](const numeric_n_array_wrapper<openffi_bool>& p){},
			[&](const openffi_string& p, openffi_size s){ p12 = p; p12_len = s; }, [&](const string_n_array_wrapper<openffi_string>& p){ p13 = p; },
			[&](const openffi_string8& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string8>& p){},
			[&](const openffi_string16& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string16>& p){},
			[&](const openffi_string32& p, openffi_size s){}, [&](const string_n_array_wrapper<openffi_string32>& p){}
		);
		
		common_data_type_parser parser(parameters, parameters_length, cb);
		parser.parse();

		int index = 0;

#define check_num_var(var_name, expected)\
		if((var_name) != (expected)){ throw std::runtime_error(#var_name" of type is not "#expected); }
		
		// check parameters
		check_num_var(p1, 3.141592);
		check_num_var(p2, 2.71f);
		
		check_num_var(p3, -10);
		check_num_var(p4, -20);
		check_num_var(p5, -30);
		check_num_var(p6, -40);
		
		check_num_var(p7, 50);
		check_num_var(p8, 60);
		check_num_var(p9, 70);
		check_num_var(p10, 80);
		
		check_num_var(p11, 1);
		
		std::string p12_str(p12, p12_len);
		if(p12_str != "This is an input"){
			std::stringstream ss;
			ss << R"(p12 of type string is not "This is an input", but ")" << p12_str.c_str() << "\"";
			throw std::runtime_error(ss.str().c_str());
		}
		
		// string[]
		if(p13.get_dimensions_count() != 1){
			throw std::runtime_error("p13 array is not of 1 dimension");
		}
		
		if(p13.get_dimension_length(0) != 2){
			throw std::runtime_error("p13 array length of type openffi_size is not 2");
		}
		
		openffi_string p13_elem1_pchar;
		openffi_size p13_elem1_size;
		openffi_size arr_index[] = {0};
		p13.get_elem_at(arr_index, 1, &p13_elem1_pchar, &p13_elem1_size);
		std::string p13_elem1(p13_elem1_pchar, p13_elem1_size);
		if(p13_elem1 != "element one")
		{
			std::stringstream ss;
			ss << R"(p13_elem1 of type string is not "element one": ")" << p13_elem1 << "\"";
			throw std::runtime_error(ss.str().c_str());
		}
		
		openffi_string p13_elem2_pchar;
		openffi_size p13_elem2_size;
		arr_index[0] = 1;
		p13.get_elem_at(arr_index, 1, &p13_elem2_pchar, &p13_elem2_size);
		std::string p13_elem2(p13_elem2_pchar, p13_elem2_size);
		if(p13_elem2 != "element two"){
			throw std::runtime_error("p13_elem2 of type string is not \"element two\"");
		}
		
		// bytes
		if(!p14.is_simple_array() || p14.get_simple_array_length() != 5){
			throw std::runtime_error("p14_len of type int64_t is not 5");
		}
		
		arr_index[0] = 0; if(p14.get_elem_at(arr_index, 1) != 2){ throw std::runtime_error("p14[0] of type unsigned char is not 2"); }
		arr_index[0] = 1; if(p14.get_elem_at(arr_index, 1) != 4){ throw std::runtime_error("p14[1] of type unsigned char is not 4"); }
		arr_index[0] = 2; if(p14.get_elem_at(arr_index, 1) != 6){ throw std::runtime_error("p14[2] of type unsigned char is not 6"); }
		arr_index[0] = 3; if(p14.get_elem_at(arr_index, 1) != 8){ throw std::runtime_error("p14[3] of type unsigned char is not 8"); }
		arr_index[0] = 4; if(p14.get_elem_at(arr_index, 1) != 10){ throw std::runtime_error("p14[4] of type unsigned char is not 10"); }

		/*
		// matrix[2][3]
		if(p15.dimensions_lengths != 2){
			throw std::runtime_error("p15 matrix is not of 2 dimensions");
		}
		
		if(p15.dimensions[0] != 2){
			throw std::runtime_error("p15 first dimension is not 2");
		}
		
		if(p15.dimensions[1] != 3){
			throw std::runtime_error("p15 second dimension is not 3");
		}

#define set_index(arr, i, j) arr[0] = i; (arr)[1] = j;
		
		openffi_size mat_index[] = { 0, 0 };
		set_index(mat_index, 0, 0); if(p15.get_elem_at(mat_index, 2) != 10){ throw std::runtime_error("p15[0][0] of type unsigned char is not 1"); }
		set_index(mat_index, 0, 1); if(p15.get_elem_at(mat_index, 2) != 20){ throw std::runtime_error("p15[0][1] of type unsigned char is not 2"); }
		set_index(mat_index, 0, 2); if(p15.get_elem_at(mat_index, 2) != 30){ throw std::runtime_error("p15[0][2] of type unsigned char is not 3"); }
		set_index(mat_index, 1, 0); if(p15.get_elem_at(mat_index, 2) != 40){ throw std::runtime_error("p15[1][0] of type unsigned char is not 4"); }
		set_index(mat_index, 1, 1); if(p15.get_elem_at(mat_index, 2) != 50){ throw std::runtime_error("p15[1][1] of type unsigned char is not 5"); }
		set_index(mat_index, 1, 2); if(p15.get_elem_at(mat_index, 2) != 60){ throw std::runtime_error("p15[1][2] of type unsigned char is not 6"); }
		*/
		
		/* This function returns:
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
		index = 0;
		index = set_arg_openffi_float64(return_values, index, alloc_openffi_float64_on_heap(0.57721));
		index = set_arg_openffi_float32(return_values, index, alloc_openffi_float32_on_heap(3.359f));
		
		index = set_arg_openffi_int8(return_values, index, alloc_openffi_int8_on_heap(-11));
		index = set_arg_openffi_int16(return_values, index, alloc_openffi_int16_on_heap(-21));
		index = set_arg_openffi_int32(return_values, index, alloc_openffi_int32_on_heap(-31));
		index = set_arg_openffi_int64(return_values, index, alloc_openffi_int64_on_heap(-41));
		
		index = set_arg_openffi_uint8(return_values, index, alloc_openffi_uint8_on_heap(51));
		index = set_arg_openffi_uint16(return_values, index, alloc_openffi_uint16_on_heap(61));
		index = set_arg_openffi_uint32(return_values, index, alloc_openffi_uint32_on_heap(71));
		index = set_arg_openffi_uint64(return_values, index, alloc_openffi_uint64_on_heap(81));
		
		index = set_arg_openffi_bool(return_values, index, alloc_openffi_bool_on_heap(1));
		
		index = set_arg_openffi_string(return_values, index,
				   alloc_openffi_string_on_heap(const_cast<char*>("This is an output"), strlen("This is an output")),
				   alloc_openffi_size_on_heap(strlen("This is an output")));
		
		// string[]
		const char* r1_elem1 = "return one";
		const char* r1_elem2 = "return two";
		openffi_size* r1_len = alloc_openffi_size_on_heap(2);
		openffi_size* arr_sizes = new openffi_size[2]{strlen(r1_elem1), strlen(r1_elem2)};
		openffi_size* str_arr_dimensions_lengths = new openffi_size[1]{ 2 };
		char** arr = new char*[2]{alloc_openffi_string_on_heap(const_cast<char*>(r1_elem1), arr_sizes[0]),
									alloc_openffi_string_on_heap(const_cast<char*>(r1_elem2), arr_sizes[1])};
		index = set_arg_openffi_string_array(return_values, index, arr, arr_sizes, str_arr_dimensions_lengths, alloc_openffi_size_on_heap(1));
		
		// byte[]
		uint8_t* barray = new uint8_t[5]{20, 40, 60, 80, 100};
		openffi_size* barray_dimensions_lengths = new openffi_size[1]{ 5 };
		index = set_arg_openffi_uint8_array(return_values, index, barray, barray_dimensions_lengths, alloc_openffi_size_on_heap(1));

		/*
		// matrix
		uint32_t** resmatrix = new uint32_t*[2]{
			new uint32_t[3]{ 11, 21, 31 },
			new uint32_t[3]{ 41, 51, 61 },
		};
		openffi_size* matrix_dimensions = new openffi_size[2]{ 2, 3 };
		set_arg_openffi_uint32_array(return_values, index, (uint32_t*)resmatrix, matrix_dimensions, alloc_openffi_size_on_heap(2));
		*/
	}
	catch_err((char**)out_err, out_err_len, exc.what());
}
//--------------------------------------------------------------------
