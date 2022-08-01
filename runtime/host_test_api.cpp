#include <runtime/runtime_plugin_api.h>
#include <utils/scope_guard.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <runtime/cdts_wrapper.h>
#include <map>

using namespace metaffi::runtime;

const char* host_idl = R"(idl_filename": "test","idl_extension": ".proto","idl_filename_with_extension": "test.proto","idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": {"metaffi_function_path": "package=main","metaffi_target_language": "python3"},"functions": [{"name": "f1","comment": "f1 comment\nparam1 comment\n","tags": {"metaffi_function_path": "function=f1"},"path_to_foreign_function": {"module": "$PWD/temp","package": "GoFuncs","function": "f1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{ "name": "p1", "type": "float64", "comment": "= 3.141592", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p2", "type": "float32", "comment": "= 2.71", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p3", "type": "int8", "comment": "= -10", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p4", "type": "int16", "comment": "= -20", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p5", "type": "int32", "comment": "= -30", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p6", "type": "int64", "comment": "= -40", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p7", "type": "uint8", "comment": "= 50", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p8", "type": "uint16", "comment": "= 60", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p9", "type": "uint32", "comment": "= 70", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p10", "type": "uint64", "comment": "= 80", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p11", "type": "bool", "comment": "= true", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p12", "type": "string", "comment": "= This is an input", "tags": null, "dimensions": 0, "pass_method": "" },{ "name": "p13", "type": "string", "comment": "= {element one, element two}", "tags": null, "dimensions": 1, "pass_method": "" },{ "name": "p14", "type": "uint8", "comment": "= {2, 4, 6, 8, 10}", "tags": null, "dimensions": 1, "pass_method": "" }],"return_values": [{"name": "r1","type": "string","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]});)";

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

int64_t function_id = 0;

//--------------------------------------------------------------------
void load_runtime(char** /*err*/, uint32_t* /*err_len*/){}
//--------------------------------------------------------------------
void free_runtime(char** /*err*/, uint32_t* /*err_len*/){ /* No runtime free */ }
//--------------------------------------------------------------------
int64_t load_function(const char* function_path, uint32_t function_path_len, int8_t params_count, int8_t retval_count, char** out_err, uint32_t* out_err_len)
{
	try
	{
		if(function_path_len == 0)
		{
			throw std::runtime_error("No function path");
		}
		
		int64_t curfunc_id = function_id;
		function_id++;
		
		
		return curfunc_id;
	}
	catch_err((char**)out_err, out_err_len, exc.what());
	
	return -1;
}
//--------------------------------------------------------------------
void free_function(int64_t func_id, char** /*err*/, uint32_t* /*err_len*/)
{
	printf("Function ID %ld freed\n", func_id);
}
//--------------------------------------------------------------------
[[maybe_unused]] void xcall_params_ret(
		int64_t func_id,
		cdts params_ret[2],
		char** out_err, uint64_t* out_err_len
)
{
	try
	{
		cdts_wrapper cdts_parameters(params_ret[0].pcdt, params_ret[1].len);
		
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
		*/
		
		if(func_id != 0){
			throw std::runtime_error("func_id is not 0");
		}
		
		metaffi_float64 p1;
		metaffi_float32 p2;
		metaffi_int8 p3;
		metaffi_int16 p4;
		metaffi_int32 p5;
		metaffi_int64 p6;
		metaffi_uint8 p7;
		metaffi_uint16 p8;
		metaffi_uint32 p9;
		metaffi_uint64 p10;
		metaffi_bool p11;
		metaffi_string8 p12;
		metaffi_size p12_len;
		string_n_array_wrapper<metaffi_string8> p13;
		numeric_n_array_wrapper<metaffi_uint8> p14;
		
		cdts_parse_callbacks cps
				(
						[&](void* values_to_set, int index, const metaffi_float32& val) { p2 = val; },
						[&](void* values_to_set, int index, const metaffi_float32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_float64& val) { p1 = val; },
						[&](void* values_to_set, int index, const metaffi_float64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_int8& val) { p3 = val; },
						[&](void* values_to_set, int index, const metaffi_int8* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_int16& val) { p4 = val; },
						[&](void* values_to_set, int index, const metaffi_int16* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_int32& val) { p5 = val; },
						[&](void* values_to_set, int index, const metaffi_int32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_int64& val) { p6 = val; },
						[&](void* values_to_set, int index, const metaffi_int64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_uint8& val) { p7 = val; },
						[&](void* values_to_set, int index, const metaffi_uint8* array, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions)
						{
							p14 = numeric_n_array_wrapper<metaffi_uint8>((metaffi_uint8*)array, (metaffi_size*)dimensions_lengths, (metaffi_size&)dimensions);
						},
						
						[&](void* values_to_set, int index, const metaffi_uint16& val) { p8 = val; },
						[&](void* values_to_set, int index, const metaffi_uint16* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_uint32& val) { p9 = val; },
						[&](void* values_to_set, int index, const metaffi_uint32* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_uint64& val) { p10 = val; },
						[&](void* values_to_set, int index, const metaffi_uint64* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_bool& val) { p11 = val; },
						[&](void* values_to_set, int index, const metaffi_bool* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_handle& val) {},
						[&](void* values_to_set, int index, const metaffi_handle* arr, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},
						
						[&](void* values_to_set, int index, const metaffi_string8& val, const metaffi_size& s)
						{
							p12 = val;
							p12_len = s;
						},
						[&](void* values_to_set, int index, const metaffi_string8* array, const metaffi_size* strings_lengths, const metaffi_size* dimensions_lengths, const metaffi_size& dimensions)
						{
							p13 = string_n_array_wrapper<metaffi_string8>((metaffi_string8*)array, (metaffi_size*)strings_lengths, (metaffi_size*)dimensions_lengths, (metaffi_size&)dimensions);
						},
						
						[&](void* values_to_set, int index, const metaffi_string16& val, const metaffi_size& s) {},
						[&](void* values_to_set, int index, const metaffi_string16*, const metaffi_size*, const metaffi_size*, const metaffi_size&) {},
						
						[&](void* values_to_set, int index, const metaffi_string32& val, const metaffi_size& s) {},
						[&](void* values_to_set, int index, const metaffi_string32*, const metaffi_size*, const metaffi_size*, const metaffi_size&) {}
				);
		
		cdts_parameters.parse(nullptr, cps);

#define check_num_var(var_name, expected) \
		if((var_name) != (expected)){           \
		std::stringstream err_ss;               \
		err_ss << #var_name"=" << (var_name) << " and not as expected = "#expected;\
		throw std::runtime_error(err_ss.str()); }
		
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
			throw std::runtime_error("p13 array length of type metaffi_size is not 2");
		}
		
		metaffi_string8 p13_elem1_pchar;
		metaffi_size p13_elem1_size;
		metaffi_size arr_index[] = {0};
		p13.get_elem_at(arr_index, 1, &p13_elem1_pchar, &p13_elem1_size);
		std::string p13_elem1(p13_elem1_pchar, p13_elem1_size);
		if(p13_elem1 != "element one")
		{
			std::stringstream ss;
			ss << R"(p13_elem1 of type string is not "element one": ")" << p13_elem1 << "\"";
			throw std::runtime_error(ss.str().c_str());
		}
		
		metaffi_string8 p13_elem2_pchar;
		metaffi_size p13_elem2_size;
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
		*/
		
		cdts_build_callbacks cbs
				(
						[&](void* values_to_set, int index, metaffi_float32& val, int) { val = 3.359f; },
						[&](void* values_to_set, int index, metaffi_float32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_float64& val, int) { val = 0.57721; },
						[&](void* values_to_set, int index, metaffi_float64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_int8& val, int) { val = -11; },
						[&](void* values_to_set, int index, metaffi_int8*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_int16& val, int) { val = -21; },
						[&](void* values_to_set, int index, metaffi_int16*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_int32& val, int) { val = -31; },
						[&](void* values_to_set, int index, metaffi_int32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_int64& val, int) { val = -41; },
						[&](void* values_to_set, int index, metaffi_int64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_uint8& val, int) { val = 51; },
						[&](void* values_to_set, int index, metaffi_uint8*& array, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int)
						{
							array = (metaffi_uint8*)malloc(sizeof(uint8_t)*5);
							array[0] = 20;
							array[1] = 40;
							array[2] = 60;
							array[3] = 80;
							array[4] = 100;
							
							dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
							dimensions_lengths[0] = 5;
							
							dimensions = 1;
							
							free_required = true;
						},
						
						[&](void* values_to_set, int index, metaffi_uint16& val, int) { val = 61; },
						[&](void* values_to_set, int index, metaffi_uint16*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_uint32& val, int) { val = 71; },
						[&](void* values_to_set, int index, metaffi_uint32*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_uint64& val, int) { val = 81; },
						[&](void* values_to_set, int index, metaffi_uint64*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_bool& val, int) { val = 1; },
						[&](void* values_to_set, int index, metaffi_bool*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_handle& val, int) { },
						[&](void* values_to_set, int index, metaffi_handle*& arr, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int) {},
						
						[&](void* values_to_set, int index, metaffi_string8& val, metaffi_size& s, int)
						{
							val = (metaffi_string8)calloc(sizeof(metaffi_char8), strlen("This is an output"));
							strncpy(val, "This is an output", strlen("This is an output"));
							s = strlen("This is an output");
						},
						[&](void* values_to_set, int index, metaffi_string8*& array, metaffi_size*& strings_lengths, metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required, int)
						{
							array = (metaffi_string8*)malloc(2*sizeof(metaffi_string8));
							
							array[0] = (metaffi_string8)calloc(sizeof(metaffi_char8), strlen("return one"));
							array[1] = (metaffi_string8)calloc(sizeof(metaffi_char8), strlen("return two"));
							strncpy(array[0], "return one", strlen("return one"));
							strncpy(array[1], "return two", strlen("return two"));
							
							strings_lengths = (metaffi_size*)malloc(sizeof(metaffi_size)*2);
							strings_lengths[0] = strlen("return one");
							strings_lengths[1] = strlen("return two");
							
							dimensions_lengths = (metaffi_size*)malloc(sizeof(metaffi_size));
							dimensions_lengths[0] = 2;
							
							dimensions = 1;
							
							free_required = true;
						},
						
						[&](void* values_to_set, int index, metaffi_string16& val, metaffi_size& s, int) {},
						[&](void* values_to_set, int index, metaffi_string16*&, metaffi_size*&, metaffi_size*&, metaffi_size&, metaffi_bool&, int) {},
						
						[&](void* values_to_set, int index, metaffi_string32& val, metaffi_size& s, int) {},
						[&](void* values_to_set, int index, metaffi_string32*&, metaffi_size*&, metaffi_size*&, metaffi_size&, metaffi_bool&, int) {}
				);
		
		cdts_wrapper cdts_return(params_ret[1].pcdt, params_ret[1].len);
		
		std::vector<metaffi_types> vec_types =
				{
						metaffi_float64_type,
						metaffi_float32_type,
						metaffi_int8_type,
						metaffi_int16_type,
						metaffi_int32_type,
						metaffi_int64_type,
						metaffi_uint8_type,
						metaffi_uint16_type,
						metaffi_uint32_type,
						metaffi_uint64_type,
						metaffi_bool_type,
						metaffi_string8_type,
						metaffi_string8_array_type,
						metaffi_uint8_array_type
				};
		
		cdts_return.build(&vec_types[0], vec_types.size(), nullptr, 0, cbs);
	}
	catch_err((char**)out_err, out_err_len, exc.what());
}
//--------------------------------------------------------------------
[[maybe_unused]] void xcall_no_params_ret(
		int64_t function_id,
		cdts return_values[1],
		char** out_err, uint64_t* out_err_len
)
{

}
//--------------------------------------------------------------------
[[maybe_unused]] void xcall_params_no_ret(
		int64_t function_id,
		cdts parameters[1],
		char** out_err, uint64_t* out_err_len
)
{

}
//--------------------------------------------------------------------
[[maybe_unused]] void xcall_no_params_no_ret(
		int64_t function_id,
		char** out_err, uint64_t* out_err_len
)
{

}
//--------------------------------------------------------------------
