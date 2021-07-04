#include "guest_test.h"
#include <runtime/cdt_capi_loader.h>
#include <runtime/cdts_wrapper.h>
#include <utils/scope_guard.hpp>
#include <string>
#include <cstdio>
#include <cstring>

using namespace openffi::runtime;
using namespace openffi::utils;

const char* guest_idl = R"({"idl_filename": "test","idl_extension": ".proto","idl_filename_with_extension": "test.proto","idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": {"openffi_function_path": "package=main","openffi_target_language": "test"},"functions": [{"name": "f1","comment": "F1 comment\nparam1 comment\n","tags": {"openffi_function_path": "function=F1,openffi_guest_lib=$PWD/temp/test_OpenFFIGuest.so"},"path_to_foreign_function": {"module": "$PWD/temp","package": "GoFuncs","function": "F1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{"name": "p1","type": "float64","comment": "= 3.141592","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p2","type": "float32","comment": "= 2.71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p3","type": "int8","comment": "= -10","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p4","type": "int16","comment": "= -20","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p5","type": "int32","comment": "= -30","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p6","type": "int64","comment": "= -40","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p7","type": "uint8","comment": "= 50","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p8","type": "uint16","comment": "= 60","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p9","type": "uint32","comment": "= 70","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p10","type": "uint64","comment": "= 80","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p12","type": "string","comment": "= This is an input","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p13","type": "string","comment": "= {element one, element two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "p14","type": "uint8","comment": "= {2, 4, 6, 8, 10}","tags": null,"dimensions": 1,"pass_method": ""}],"return_values": [{"name": "r1","type": "float64","comment": "= 0.57721","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r2","type": "float32","comment": "= 3.359","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r3","type": "int8","comment": "= -11","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r4","type": "int16","comment": "= -21","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r5","type": "int32","comment": "= -31","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r6","type": "int64","comment": "= -41","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r7","type": "uint8","comment": "= 51","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r8","type": "uint16","comment": "= 61","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r9","type": "uint32","comment": "= 71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r10","type": "uint64","comment": "= 81","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r12","type": "string","comment": "= This is an output","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r13","type": "string","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "r14","type": "uint8","comment": "= {20, 40, 60, 80, 100}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]})";

// 0 - success
// otherwise - failed
int test_guest(const char* lang_plugin, const char* function_path)
{
    load_xllr();
    load_cdt_capi();

	char* err = nullptr;
	uint64_t err_len = 0;

	scope_guard sg([&](){
		xllr_free_runtime_plugin("xllr.test", strlen("xllr.test"), &err, reinterpret_cast<uint32_t*>(&err_len));
		free_xllr();
	});
	
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

	    //matrix = { {10, 20, 30}, {40, 50, 60} }
	*/
	
	printf("preparing parameters\n");

	cdts_wrapper cdts_params(14);

	std::vector<openffi_types> vec_types =
	{
		openffi_float64_type,
		openffi_float32_type,
		openffi_int8_type,
		openffi_int16_type,
		openffi_int32_type,
		openffi_int64_type,
		openffi_uint8_type,
		openffi_uint16_type,
		openffi_uint32_type,
		openffi_uint64_type,
		openffi_bool_type,
		openffi_string_type,
		openffi_string_array_type,
		openffi_uint8_array_type
	};

	openffi_float64 p1 = 3.141592;
	openffi_float32 p2 = 2.71f;
	openffi_int8 p3 = -10;
	openffi_int16 p4 = -20;
	openffi_int32 p5 = -30;
	openffi_int64 p6 = -40;
	openffi_uint8 p7 = 50;
	openffi_uint16 p8 = 60;
	openffi_uint32 p9 = 70;
	openffi_uint64 p10 = 80;
	openffi_bool p11 = 1;
	std::string p12("This is an input");
	size_t p12_len = p12.size();
	std::vector<openffi_string> p13 = {(char*) "element one", (char*) "element two"};
	std::vector<openffi_size> p13_sizes = {strlen("element one"), strlen("element two")};
	std::vector<openffi_size> p13_dimensions_lengths = {p13.size()};

	std::vector<openffi_uint8> p14 = {2, 4, 6, 8, 10};
	std::vector<openffi_size> p14_dimensions_lengths = {p14.size()};

	cdts_build_callbacks cbs
	(
		[&](void* values_to_set, int index, openffi_float32& val) { val = p2; },
		[&](void* values_to_set, int index, openffi_float32*& val, openffi_bool& free_required)
		{
			val = &p2;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_float32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_float64& val) { val = p1; },
		[&](void* values_to_set, int index, openffi_float64*& val, openffi_bool& free_required)
		{
			val = &p1;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_float64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_int8& val) { val = p3; },
		[&](void* values_to_set, int index, openffi_int8*& val, openffi_bool& free_required)
		{
			val = &p3;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_int8*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_int16& val) { val = p4; },
		[&](void* values_to_set, int index, openffi_int16*& val, openffi_bool& free_required)
		{
			val = &p4;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_int16*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_int32& val) { val = p5; },
		[&](void* values_to_set, int index, openffi_int32*& val, openffi_bool& free_required)
		{
			val = &p5;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_int32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_int64& val) { val = p6; },
		[&](void* values_to_set, int index, openffi_int64*& val, openffi_bool& free_required)
		{
			val = &p6;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_int64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_uint8& val) { val = p7; },
		[&](void* values_to_set, int index, openffi_uint8*& val, openffi_bool& free_required)
		{
			val = &p7;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_uint8*& array, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required)
		{
			array = &p14[0];
			dimensions_lengths = &p14_dimensions_lengths[0];
			dimensions = 1;
			free_required = false;
		},

		[&](void* values_to_set, int index, openffi_uint16& val) { val = p8; },
		[&](void* values_to_set, int index, openffi_uint16*& val, openffi_bool& free_required)
		{
			val = &p8;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_uint16*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_uint32& val) { val = p9; },
		[&](void* values_to_set, int index, openffi_uint32*& val, openffi_bool& free_required)
		{
			val = &p9;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_uint32*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_uint64& val) { val = p10; },
		[&](void* values_to_set, int index, openffi_uint64*& val, openffi_bool& free_required)
		{
			val = &p10;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_uint64*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_bool& val) { val = p11; },
		[&](void* values_to_set, int index, openffi_bool*& val, openffi_bool& free_required)
		{
			val = &p11;
			free_required = 0;
		},
		[&](void* values_to_set, int index, openffi_bool*& arr, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required) {},

		[&](void* values_to_set, int index, openffi_string& val, openffi_size& s)
		{
			val = (char*) p12.c_str();
			s = p12_len;
		},
		[&](void* values_to_set, int index, openffi_string*& val, openffi_size*& s, openffi_bool& free_required) {},
		[&](void* values_to_set, int index, openffi_string*& array, openffi_size*& strings_lengths, openffi_size*& dimensions_lengths, openffi_size& dimensions, openffi_bool& free_required)
		{
			array = &p13[0];
			strings_lengths = &p13_sizes[0];
			dimensions_lengths = &p13_dimensions_lengths[0];
			dimensions = 1;
			free_required = false;
		},

		[&](void* values_to_set, int index, openffi_string8& val, openffi_size& s) {},
		[&](void* values_to_set, int index, openffi_string8*& val, openffi_size*& s, openffi_bool& free_required) {},
		[&](void* values_to_set, int index, openffi_string8*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {},

		[&](void* values_to_set, int index, openffi_string16& val, openffi_size& s) {},
		[&](void* values_to_set, int index, openffi_string16*& val, openffi_size*& s, openffi_bool& free_required) {},
		[&](void* values_to_set, int index, openffi_string16*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {},

		[&](void* values_to_set, int index, openffi_string32& val, openffi_size& s) {},
		[&](void* values_to_set, int index, openffi_string32*& val, openffi_size*& s, openffi_bool& free_required) {},
		[&](void* values_to_set, int index, openffi_string32*&, openffi_size*&, openffi_size*&, openffi_size&, openffi_bool&) {}
	);

	cdts_params.build(&vec_types[0], vec_types.size(), nullptr, cbs);

	printf("calling guest function\n");

	cdts_wrapper cdts_return(14);

	xllr_call(lang_plugin, strlen(lang_plugin),
	          function_id,
	          cdts_params.get_cdts(), cdts_params.get_cdts_length(),
	          cdts_return.get_cdts(), cdts_return.get_cdts_length(),
	          &err, reinterpret_cast<uint64_t *>(&err_len)
	);

	if(err)
	{
		printf("Error returned from guest: %s\n", std::string(err, err_len).c_str());
		return 2;
	}

	printf("No errors returned from guest\n");

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

	cdts_parse_callbacks cps
	(
		[&](void* values_to_set, int index, const openffi_float32& val) { r2 = val; },
		[&](void* values_to_set, int index, const openffi_float32* val){},
		[&](void* values_to_set, int index, const openffi_float32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_float64& val) { r1 = val; },
		[&](void* values_to_set, int index, const openffi_float64* val){ },
		[&](void* values_to_set, int index, const openffi_float64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_int8& val) { r3 = val; },
		[&](void* values_to_set, int index, const openffi_int8* val){},
		[&](void* values_to_set, int index, const openffi_int8* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_int16& val) { r4 = val; },
		[&](void* values_to_set, int index, const openffi_int16* val){},
		[&](void* values_to_set, int index, const openffi_int16* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_int32& val) { r5 = val; },
		[&](void* values_to_set, int index, const openffi_int32* val){},
		[&](void* values_to_set, int index, const openffi_int32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_int64& val) { r6 = val; },
		[&](void* values_to_set, int index, const openffi_int64* val){ },
		[&](void* values_to_set, int index, const openffi_int64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_uint8& val) { r7 = val; },
		[&](void* values_to_set, int index, const openffi_uint8* val){ },
		[&](void* values_to_set, int index, const openffi_uint8* array, const openffi_size* dimensions_lengths, const openffi_size& dimensions)
		{
			r14 = numeric_n_array_wrapper<openffi_uint8>((openffi_uint8*)array, (openffi_size*)dimensions_lengths, (openffi_size&)dimensions);
		},

		[&](void* values_to_set, int index, const openffi_uint16& val) { r8 = val; },
		[&](void* values_to_set, int index, const openffi_uint16* val){ },
		[&](void* values_to_set, int index, const openffi_uint16* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_uint32& val) { r9 = val; },
		[&](void* values_to_set, int index, const openffi_uint32* val){ },
		[&](void* values_to_set, int index, const openffi_uint32* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_uint64& val) { r10 = val; },
		[&](void* values_to_set, int index, const openffi_uint64* val) { },
		[&](void* values_to_set, int index, const openffi_uint64* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_bool& val) { r11 = val; },
		[&](void* values_to_set, int index, const openffi_bool* val){ },
		[&](void* values_to_set, int index, const openffi_bool* arr, const openffi_size* dimensions_lengths, const openffi_size& dimensions) {},

		[&](void* values_to_set, int index, const openffi_string& val, const openffi_size& s)
		{
			r12 = val;
			r12_len = s;
		},
		[&](void* values_to_set, int index, const openffi_string* val, const openffi_size* s) {},
		[&](void* values_to_set, int index, const openffi_string* array, const openffi_size* strings_lengths, const openffi_size* dimensions_lengths, const openffi_size& dimensions)
		{
			r13 = string_n_array_wrapper<openffi_string>((openffi_string*)array, (openffi_size*)strings_lengths, (openffi_size*)dimensions_lengths, (openffi_size&)dimensions);
		},

		[&](void* values_to_set, int index, const openffi_string8& val, const openffi_size& s) {},
		[&](void* values_to_set, int index, const openffi_string8* val, const openffi_size* s) {},
		[&](void* values_to_set, int index, const openffi_string8*, const openffi_size*, const openffi_size*, const openffi_size&) {},

		[&](void* values_to_set, int index, const openffi_string16& val, const openffi_size& s) {},
		[&](void* values_to_set, int index, const openffi_string16* val, const openffi_size* s) {},
		[&](void* values_to_set, int index, const openffi_string16*, const openffi_size*, const openffi_size*, const openffi_size&) {},

		[&](void* values_to_set, int index, const openffi_string32& val, const openffi_size& s) {},
		[&](void* values_to_set, int index, const openffi_string32* val, const openffi_size* s) {},
		[&](void* values_to_set, int index, const openffi_string32*, const openffi_size*, const openffi_size*, const openffi_size&) {}
	);

	cdts_return.parse(nullptr, cps);

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


	err_len = 0;

	return 0;
}

