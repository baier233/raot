#include <Windows.h>

#include "Main.hpp"




BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
	(void)lpReserved;
#ifndef DEBUG

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		return main::process_attach(hModule);
	case DLL_PROCESS_DETACH:
		return main::process_detach(hModule);
	case DLL_THREAD_ATTACH:
		return thread::thread_attach(hModule);
	case DLL_THREAD_DETACH:
		return thread::thread_detach(hModule);
	}
#endif // !DEBUG


	return TRUE;
}