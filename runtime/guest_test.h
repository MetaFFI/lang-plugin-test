#pragma once


// 0 - success
// otherwise - failed
#ifndef SKIP_XLLR_API_EXTERN
extern "C"
#endif
int test_guest(const char* lang_plugin, const char* function_path);