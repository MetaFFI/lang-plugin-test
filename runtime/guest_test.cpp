#include "guest_test.h"
#include <runtime/cdt_capi_loader.h>
#include <runtime/cdts_wrapper.h>
#include <utils/scope_guard.hpp>
#include <string>
#include <cstdio>
#include <cstring>
#include <runtime/cdts_alloc.h>

// NOLINT(bugprone-macro-parentheses)

using namespace metaffi::utils;

const char* guest_idl = R"({"idl_filename": "test","idl_extension": ".proto","idl_filename_with_extension": "test.proto","idl_full_path": "","modules": [{"name": "Service1","target_language": "test","comment": "Comments for Service1\n","tags": {"metaffi_function_path": "package=main","metaffi_target_language": "test"},"functions": [{"name": "f1","comment": "F1 comment\nparam1 comment\n","tags": {"metaffi_function_path": "function=F1,metaffi_guest_lib=$PWD/temp/test_MetaFFIGuest.so"},"path_to_foreign_function": {"module": "$PWD/temp","package": "GoFuncs","function": "F1"},"parameter_type": "Params1","return_values_type": "Return1","parameters": [{"name": "p1","type": "float64","comment": "= 3.141592","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p2","type": "float32","comment": "= 2.71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p3","type": "int8","comment": "= -10","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p4","type": "int16","comment": "= -20","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p5","type": "int32","comment": "= -30","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p6","type": "int64","comment": "= -40","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p7","type": "uint8","comment": "= 50","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p8","type": "uint16","comment": "= 60","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p9","type": "uint32","comment": "= 70","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p10","type": "uint64","comment": "= 80","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p12","type": "string","comment": "= This is an input","tags": null,"dimensions": 0,"pass_method": ""},{"name": "p13","type": "string","comment": "= {element one, element two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "p14","type": "uint8","comment": "= {2, 4, 6, 8, 10}","tags": null,"dimensions": 1,"pass_method": ""}],"return_values": [{"name": "r1","type": "float64","comment": "= 0.57721","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r2","type": "float32","comment": "= 3.359","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r3","type": "int8","comment": "= -11","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r4","type": "int16","comment": "= -21","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r5","type": "int32","comment": "= -31","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r6","type": "int64","comment": "= -41","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r7","type": "uint8","comment": "= 51","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r8","type": "uint16","comment": "= 61","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r9","type": "uint32","comment": "= 71","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r10","type": "uint64","comment": "= 81","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r11","type": "bool","comment": "= true","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r12","type": "string","comment": "= This is an output","tags": null,"dimensions": 0,"pass_method": ""},{"name": "r13","type": "string","comment": "= {return one, return two}","tags": null,"dimensions": 1,"pass_method": ""},{"name": "r14","type": "uint8","comment": "= {20, 40, 60, 80, 100}","tags": null,"dimensions": 1,"pass_method": ""}]}]}]})";

// 0 - success
// otherwise - failed
int test_guest(const char* lang_plugin, const char* function_path)
{
	try
	{
		load_xllr();
		load_cdt_capi();

		char* err = nullptr;
		uint64_t err_len = 0;

		scope_guard sg([&]() {
			xllr_free_runtime_plugin("xllr.test", strlen("xllr.test"), &err, reinterpret_cast<uint32_t*>(&err_len));
			free_xllr();
		});
		
		xllr_load_runtime_plugin(lang_plugin, strlen(lang_plugin), &err, reinterpret_cast<uint32_t*>(&err_len));
		if (err != nullptr) {
			printf("Failed to load runtime \"%s\". Error: %s\n", lang_plugin, err);
			return 1;
		}
		
		printf("test_guest - loading function\n");
		int64_t function_id = xllr_load_function(lang_plugin, strlen(lang_plugin), function_path, strlen(function_path),
		                                         -1, &err, reinterpret_cast<uint32_t*>(&err_len));

		if (err != nullptr) {
			printf("Failed to load function \"%s\". Error: %s\n", function_path, err);
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
 
		cdt* params_buf = xllr_alloc_cdts_buffer(14);
		metaffi::runtime::cdts_wrapper cdts_params(params_buf, 14, true);

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

		metaffi_float64 p1 = 3.141592;
		metaffi_float32 p2 = 2.71f;
		metaffi_int8 p3 = -10;
		metaffi_int16 p4 = -20;
		metaffi_int32 p5 = -30;
		metaffi_int64 p6 = -40;
		metaffi_uint8 p7 = 50;
		metaffi_uint16 p8 = 60;
		metaffi_uint32 p9 = 70;
		metaffi_uint64 p10 = 80;
		metaffi_bool p11 = 1;
		std::string p12("This is an input");
		size_t p12_len = p12.size();
		std::vector<metaffi_string8> p13 = {(char*) "element one", (char*) "element two"};
		std::vector<metaffi_size> p13_sizes = {strlen("element one"), strlen("element two")};
		std::vector<metaffi_size> p13_dimensions_lengths = {p13.size()};

		std::vector<metaffi_uint8> p14 = {2, 4, 6, 8, 10};
		std::vector<metaffi_size> p14_dimensions_lengths = {p14.size()};
		
		metaffi::runtime::cdts_build_callbacks cbs
		(
				[&](void* values_to_set, int index, metaffi_float32& val) { val = p2; },
				[&](void* values_to_set, int index, metaffi_float32*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_float64& val) { val = p1; },
				[&](void* values_to_set, int index, metaffi_float64*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_int8& val) { val = p3; },
				[&](void* values_to_set, int index, metaffi_int8*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_int16& val) { val = p4; },
				[&](void* values_to_set, int index, metaffi_int16*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_int32& val) { val = p5; },
				[&](void* values_to_set, int index, metaffi_int32*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_int64& val) { val = p6; },
				[&](void* values_to_set, int index, metaffi_int64*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_uint8& val) { val = p7; },
				[&](void* values_to_set, int index, metaffi_uint8*& array, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {
					array = &p14[0];
					dimensions_lengths = &p14_dimensions_lengths[0];
					dimensions = 1;
					free_required = false;
				},

				[&](void* values_to_set, int index, metaffi_uint16& val) { val = p8; },
				[&](void* values_to_set, int index, metaffi_uint16*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_uint32& val) { val = p9; },
				[&](void* values_to_set, int index, metaffi_uint32*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_uint64& val) { val = p10; },
				[&](void* values_to_set, int index, metaffi_uint64*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_bool& val) { val = p11; },
				[&](void* values_to_set, int index, metaffi_bool*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_handle& val) {},
				[&](void* values_to_set, int index, metaffi_handle*& arr, metaffi_size*& dimensions_lengths,
				    metaffi_size& dimensions, metaffi_bool& free_required) {},

				[&](void* values_to_set, int index, metaffi_string8& val, metaffi_size& s) {
					val = (char*) p12.c_str();
					s = p12_len;
				},
				[&](void* values_to_set, int index, metaffi_string8*& array, metaffi_size*& strings_lengths,
				    metaffi_size*& dimensions_lengths, metaffi_size& dimensions, metaffi_bool& free_required) {
					array = &p13[0];
					strings_lengths = &p13_sizes[0];
					dimensions_lengths = &p13_dimensions_lengths[0];
					dimensions = 1;
					free_required = false;
				},

				[&](void* values_to_set, int index, metaffi_string16& val, metaffi_size& s) {},
				[&](void* values_to_set, int index, metaffi_string16*&, metaffi_size*&, metaffi_size*&,
				    metaffi_size&, metaffi_bool&) {},

				[&](void* values_to_set, int index, metaffi_string32& val, metaffi_size& s) {},
				[&](void* values_to_set, int index, metaffi_string32*&, metaffi_size*&, metaffi_size*&,
				    metaffi_size&, metaffi_bool&) {}
		);

		cdts_params.build(&vec_types[0], vec_types.size(), nullptr, cbs);

		printf("calling guest function\n");
		
		cdt* return_buf = xllr_alloc_cdts_buffer(14);
		metaffi::runtime::cdts_wrapper cdts_return(return_buf, 14, true);

		xllr_xcall(lang_plugin, strlen(lang_plugin),
		          function_id,
		          cdts_params.get_cdts(), cdts_params.get_cdts_length(),
		          cdts_return.get_cdts(), cdts_return.get_cdts_length(),
		          &err, reinterpret_cast<uint64_t*>(&err_len)
		);

		if (err) {
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
		*/

		metaffi_float64 r1;
		metaffi_float32 r2;
		metaffi_int8 r3;
		metaffi_int16 r4;
		metaffi_int32 r5;
		metaffi_int64 r6;
		metaffi_uint8 r7;
		metaffi_uint16 r8;
		metaffi_uint32 r9;
		metaffi_uint64 r10;
		metaffi_bool r11;
		metaffi_string8 r12;
		metaffi_size r12_len;
		metaffi::runtime::string_n_array_wrapper<metaffi_string8> r13;
		metaffi::runtime::numeric_n_array_wrapper<metaffi_uint8> r14;
		
		metaffi::runtime::cdts_parse_callbacks cps
		(
				[&](void* values_to_set, int index, const metaffi_float32& val) { r2 = val; },
				[&](void* values_to_set, int index, const metaffi_float32* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_float64& val) { r1 = val; },
				[&](void* values_to_set, int index, const metaffi_float64* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_int8& val) { r3 = val; },
				[&](void* values_to_set, int index, const metaffi_int8* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_int16& val) { r4 = val; },
				[&](void* values_to_set, int index, const metaffi_int16* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_int32& val) { r5 = val; },
				[&](void* values_to_set, int index, const metaffi_int32* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_int64& val) { r6 = val; },
				[&](void* values_to_set, int index, const metaffi_int64* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_uint8& val) { r7 = val; },
				[&](void* values_to_set, int index, const metaffi_uint8* array,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {
					r14 = metaffi::runtime::numeric_n_array_wrapper<metaffi_uint8>((metaffi_uint8*) array,
					                                             (metaffi_size*) dimensions_lengths,
					                                             (metaffi_size&) dimensions);
				},

				[&](void* values_to_set, int index, const metaffi_uint16& val) { r8 = val; },
				[&](void* values_to_set, int index, const metaffi_uint16* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_uint32& val) { r9 = val; },
				[&](void* values_to_set, int index, const metaffi_uint32* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_uint64& val) { r10 = val; },
				[&](void* values_to_set, int index, const metaffi_uint64* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_bool& val) { r11 = val; },
				[&](void* values_to_set, int index, const metaffi_bool* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_handle& val) {},
				[&](void* values_to_set, int index, const metaffi_handle* arr,
				    const metaffi_size* dimensions_lengths, const metaffi_size& dimensions) {},

				[&](void* values_to_set, int index, const metaffi_string8& val, const metaffi_size& s) {
					r12 = val;
					r12_len = s;
				},
				[&](void* values_to_set, int index, const metaffi_string8* array,
				    const metaffi_size* strings_lengths, const metaffi_size* dimensions_lengths,
				    const metaffi_size& dimensions) {
					r13 = metaffi::runtime::string_n_array_wrapper<metaffi_string8>((metaffi_string8*) array,
					                                             (metaffi_size*) strings_lengths,
					                                             (metaffi_size*) dimensions_lengths,
					                                             (metaffi_size&) dimensions);
				},

				[&](void* values_to_set, int index, const metaffi_string16& val, const metaffi_size& s) {},
				[&](void* values_to_set, int index, const metaffi_string16*, const metaffi_size*,
				    const metaffi_size*, const metaffi_size&) {},

				[&](void* values_to_set, int index, const metaffi_string32& val, const metaffi_size& s) {},
				[&](void* values_to_set, int index, const metaffi_string32*, const metaffi_size*,
				    const metaffi_size*, const metaffi_size&) {}
		);

		cdts_return.parse(nullptr, cps);

#define check_num_var(var_name, expected)\
    if((var_name) != (expected)){        \
        std::stringstream ss;            \
        ss << #var_name" is not "#expected", but: " << (var_name);\
        throw std::runtime_error(ss.str()); \
    }

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
		if (r12_str != "This is an output") {
			std::stringstream ss;
			ss << R"(r12 of type string is not "This is an input", but ")" << r12_str.c_str() << "\"";
			throw std::runtime_error(ss.str().c_str());
		}

		// string[]
		if (r13.get_dimensions_count() != 1) {
			throw std::runtime_error("p13 array is not of 1 dimension");
		}

		if (r13.get_dimension_length(0) != 2) {
			throw std::runtime_error("p13 array length of type metaffi_size is not 2");
		}

		metaffi_string8 r13_elem1_pchar;
		metaffi_size r13_elem1_size;
		metaffi_size arr_index[] = {0};
		r13.get_elem_at(arr_index, 1, &r13_elem1_pchar, &r13_elem1_size);
		std::string r13_elem1(r13_elem1_pchar, r13_elem1_size);
		if (r13_elem1 != "return one") {
			std::stringstream ss;
			ss << R"(r13_elem1 of type string is not "return one": ")" << r13_elem1 << "\"";
			throw std::runtime_error(ss.str().c_str());
		}

		metaffi_string8 r13_elem2_pchar;
		metaffi_size r13_elem2_size;
		arr_index[0] = 1;
		r13.get_elem_at(arr_index, 1, &r13_elem2_pchar, &r13_elem2_size);
		std::string r13_elem2(r13_elem2_pchar, r13_elem2_size);
		if (r13_elem2 != "return two") {
			throw std::runtime_error("r13_elem2 of type string is not \"return two\"");
		}

		// bytes
		if (!r14.is_simple_array() || r14.get_simple_array_length() != 5) {
			throw std::runtime_error("p14_len of type int64_t is not 5");
		}

		arr_index[0] = 0;
		if (r14.get_elem_at(arr_index, 1) != 20) { throw std::runtime_error("r14[0] of type unsigned char is not 20"); }
		arr_index[0] = 1;
		if (r14.get_elem_at(arr_index, 1) != 40) { throw std::runtime_error("r14[1] of type unsigned char is not 40"); }
		arr_index[0] = 2;
		if (r14.get_elem_at(arr_index, 1) != 60) { throw std::runtime_error("r14[2] of type unsigned char is not 60"); }
		arr_index[0] = 3;
		if (r14.get_elem_at(arr_index, 1) != 80) { throw std::runtime_error("r14[3] of type unsigned char is not 80"); }
		arr_index[0] = 4;
		if (r14.get_elem_at(arr_index, 1) != 100) {
			throw std::runtime_error("r14[4] of type unsigned char is not 100");
		}


		err_len = 0;

		return 0;
	}
	catch(std::exception& err)
	{
		printf("Test failed with error: %s\n", err.what());
		return 1;
	}
}

