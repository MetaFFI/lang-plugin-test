// contains main to test plugin-test
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

int main()
{
	const char* openffi_home = getenv("OPENFFI_HOME");
	
	char xllr_dir[100] = {0};
	sprintf(xllr_dir, "%s/xllr.so", openffi_home);
	
	void* xllr_handle = dlopen(xllr_dir, RTLD_NOW | RTLD_GLOBAL);
	if(!xllr_handle)
	{
		printf("Failed loading library - %s\n", dlerror());
		return -1;
	}
	
	char lib_dir[100] = {0};
	sprintf(lib_dir, "%s/xllr.test.so", openffi_home);
	
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
	
	return ((int (*) (const char*, const char*))res)("xllr.test", "package=GuestCode,function=f1,openffi_guest_lib=test_OpenFFIGuest,entrypoint_function=EntryPoint_f1");
	
}
