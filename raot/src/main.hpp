#pragma once
#include <Windows.h>

#ifdef CURRENT_IN_PUBLISH_CONFIGURTION
#define PUBLISH
#endif
#include <fstream>
#include <cstdio>
namespace console {
	//创建调试窗口
	inline void create_console(void)
	{

#ifndef PUBLISH

		FreeConsole();
		if (!AllocConsole())
		{
			char buffer[1024] = { 0 };
			sprintf_s(buffer, "Failed to AllocConsole( ), GetLastError( ) = %d", GetLastError());
			MessageBoxA(HWND_DESKTOP, buffer, "Error", MB_OK);

			return;
		}

		FILE* fp = nullptr;
		freopen_s(&fp, "CONOUT$", "w", stdout);

		*(__acrt_iob_func(1)) = *fp;
		setvbuf(stdout, NULL, _IONBF, 0);
#else


#endif // !PUBLIC
	}

	inline void close_console(void)
	{
#ifndef PUBLISH
		FILE* fp = (__acrt_iob_func(1));
		if (fp != nullptr) {
			fclose(fp);
		}

		if (!FreeConsole())
		{
			char buffer[1024] = { 0 };
			sprintf_s(buffer, "Failed to FreeConsole(), GetLastError() = %d", GetLastError());
			MessageBoxA(HWND_DESKTOP, buffer, "Error", MB_OK);
			return;
		}
#endif // !PUBLISH


	}


};

namespace main {
	inline HMODULE current_module{};
	BOOL process_attach(HMODULE hDll);
	BOOL process_detach(HMODULE hDll);
}
namespace thread {

	inline LONG s_nTlsIndent = -1;
	inline LONG s_nTlsThread = -1;
	inline LONG s_nThreadCnt = 0;


	BOOL thread_attach(HMODULE hDll);
	BOOL thread_detach(HMODULE hDll);
}
