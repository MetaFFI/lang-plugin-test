#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <cstdlib>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>

TEST_CASE( "Test XLLR using Test Plugin", "[xllr.test]" )
{
	const char* metaffi_home = getenv("METAFFI_HOME");
	
	char xllr_dir[100] = {0};
	sprintf(xllr_dir, "%s/xllr.dll", metaffi_home);
	
	void* xllr_handle = LoadLibraryA(xllr_dir);
	if(!xllr_handle)
	{
		printf("Failed loading library %s - 0x%lx\n", xllr_dir, GetLastError());
		CAPTURE(GetLastError());
		REQUIRE(xllr_handle != nullptr);
	}
	
	char lib_dir[100] = {0};
	sprintf(lib_dir, "%s/xllr.test.dll", metaffi_home);
	
	HMODULE lib_handle = LoadLibraryA(lib_dir);
	if(!lib_handle)
	{
		printf("Failed loading library - 0x%lx\n", GetLastError());
		CAPTURE(GetLastError());
		REQUIRE(lib_handle != nullptr);
	}
	
	void* res = GetProcAddress(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol \"test_guest\" - 0x%lx\n", GetLastError());
		CAPTURE(GetLastError());
		REQUIRE(res != nullptr);
	}
	
	int r = ((int (*) (const char*, const char*))res)("xllr.test", "package=GuestCode,function=f1,metaffi_guest_lib=test_MetaFFIGuest,entrypoint_function=EntryPoint_f1");
	
	REQUIRE(r == 0);
}
#else
#include <dlfcn.h>
TEST_CASE( "Test XLLR using Test Plugin", "[xllr.test]" )
{
	const char* metaffi_home = getenv("METAFFI_HOME");
	if(!metaffi_home)
	{
		printf("METAFFI_HOME not defined!\n");
		CAPTURE("METAFFI_HOME not defined!");
		REQUIRE(metaffi_home != nullptr);
	}
	
	char xllr_dir[100] = {0};
	sprintf(xllr_dir, "%s/xllr.so", metaffi_home);
	printf("Loading %s", xllr_dir);
	void* xllr_handle = dlopen(xllr_dir, RTLD_NOW | RTLD_GLOBAL);
	if(!xllr_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		CAPTURE(dlerror());
		REQUIRE(xllr_handle != nullptr);
	}
	char lib_dir[100] = {0};
	sprintf(lib_dir, "%s/xllr.test.so", metaffi_home);
	printf("Loading %s", lib_dir);
	void* lib_handle = dlopen(lib_dir, RTLD_NOW | RTLD_GLOBAL);
	if(!lib_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		CAPTURE(dlerror());
		REQUIRE(lib_handle != nullptr);
	}
	
	printf("Loading exported function \"%s\"", "test_guest");
	void* res = dlsym(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol - %s\n", dlerror());
		CAPTURE(dlerror());
		REQUIRE(res != nullptr);
	}

	printf("Calling test_guest(\"package=GuestCode,function=f1,metaffi_guest_lib=test_MetaFFIGuest,entrypoint_function=EntryPoint_f1\")\n");
	int r = ((int (*) (const char*, const char*))res)("xllr.test", "package=GuestCode,function=f1,metaffi_guest_lib=test_MetaFFIGuest,entrypoint_function=EntryPoint_f1");
	
	REQUIRE(r == 0);
}
#endif