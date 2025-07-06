#include "main.hpp"

#include <memory>
#include <stdexcept>
#include <functional>
#include <iostream>
#include <thread>
#include <base/raot.h>


static DWORD WINAPI bootstrap_thread(LPVOID module)
{
	auto& instance = raot::get();


	if (instance.setup()) {

		instance.enter_loop();
	}
	else {
		instance.exit();
	}
	return NULL;
}

#include <utils/Thread.h>
BOOL main::process_attach(HMODULE module)
{
	thread::s_nTlsIndent = TlsAlloc();
	thread::s_nTlsThread = TlsAlloc();



#ifdef _DEBUG
	console::create_console();
#endif // PULISH
	current_module = module;

	auto handle = CreateThread(0, 0, bootstrap_thread, module, 0, 0);

	//utils::thread::hideThread(handle);
	if (handle) CloseHandle(handle);
	thread::thread_attach(module);
	return TRUE;
}

BOOL main::process_detach(HMODULE module)
{

	thread::thread_detach(module);

	if (thread::s_nTlsIndent >= 0) {
		TlsFree(thread::s_nTlsIndent);
	}
	if (thread::s_nTlsThread >= 0) {
		TlsFree(thread::s_nTlsThread);
	}
	console::close_console();
	//Console::CloseConsole_();
	return TRUE;
}


BOOL thread::thread_attach(HMODULE module)
{
	(void)module;

	if (s_nTlsIndent >= 0) {
		TlsSetValue(s_nTlsIndent, (PVOID)0);
	}
	if (s_nTlsThread >= 0) {
		LONG nThread = InterlockedIncrement(&s_nThreadCnt);
		TlsSetValue(s_nTlsThread, (PVOID)(LONG_PTR)nThread);
	}
	return TRUE;
}

BOOL thread::thread_detach(HMODULE hDll)
{
	(void)hDll;

	if (s_nTlsIndent >= 0) {
		TlsSetValue(s_nTlsIndent, (PVOID)0);
	}
	if (s_nTlsThread >= 0) {
		TlsSetValue(s_nTlsThread, (PVOID)0);
	}
	return TRUE;
}

