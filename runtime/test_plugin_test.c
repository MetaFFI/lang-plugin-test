// contains main to test plugin-test
#include <stdlib.h>
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
int main()
{
	const char* metaffi_home = getenv("METAFFI_HOME");
	
	char xllr_dir[100] = {0};
	sprintf(xllr_dir, "%s/xllr.dll", metaffi_home);
	
	void* xllr_handle = LoadLibraryA(xllr_dir);
	if(!xllr_handle)
	{
		printf("Failed loading library %s - 0x%lx\n", xllr_dir, GetLastError());
		return -1;
	}
	
	char lib_dir[100] = {0};
	sprintf(lib_dir, "%s/xllr.test.dll", metaffi_home);
	
	void* lib_handle = LoadLibraryA(lib_dir);
	if(!lib_handle)
	{
		printf("Failed loading library - 0x%lx\n", GetLastError());
		return -1;
	}
	
	void* res = GetProcAddress(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol \"test_guest\" - 0x%lx\n", GetLastError());
		return -1;
	}
	
	return ((int (*) (const char*, const char*))res)("xllr.test", "package=GuestCode,function=f1,metaffi_guest_lib=test_MetaFFIGuest,entrypoint_function=EntryPoint_f1");
	
}
#else
#include <dlfcn.h>
int main()
{
	const char* metaffi_home = getenv("METAFFI_HOME");
	char xllr_dir[100] = {0};
	sprintf(xllr_dir, "%s/xllr.so", metaffi_home);
	void* xllr_handle = dlopen(xllr_dir, RTLD_NOW | RTLD_GLOBAL);
	if(!xllr_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		return -1;
	}
	char lib_dir[100] = {0};
	sprintf(lib_dir, "%s/xllr.test.so", metaffi_home);
	void* lib_handle = dlopen(lib_dir, RTLD_NOW | RTLD_GLOBAL);
	if(!lib_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		return -1;
	}

	void* res = dlsym(lib_handle, "test_guest");
	if(!res)
	{
		printf("Failed loading symbol - %s\n", dlerror());
		return -1;
	}

	return ((int (*) (const char*, const char*))res)("xllr.test", "package=GuestCode,function=f1,metaffi_guest_lib=test_MetaFFIGuest,entrypoint_function=EntryPoint_f1");
	
}
#endif