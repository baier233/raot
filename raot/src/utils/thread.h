#pragma once
#include <Windows.h>
#include <TlHelp32.h>
namespace utils {
	namespace thread {
		inline void resume_all_threads() {

			HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (hThreadSnap != INVALID_HANDLE_VALUE) {
				THREADENTRY32 te32;
				te32.dwSize = sizeof(THREADENTRY32);
				if (Thread32First(hThreadSnap, &te32)) {
					do {
						if (te32.th32OwnerProcessID == GetCurrentProcessId()) {
							if (te32.th32ThreadID == GetCurrentThreadId()) continue;
							HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
							if (hThread != NULL) {
								ResumeThread(hThread);
								CloseHandle(hThread);
							}
						}
					} while (Thread32Next(hThreadSnap, &te32));
				}
				CloseHandle(hThreadSnap);
			}
		}
		inline void pause_all_threads() {
			HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (hThreadSnap != INVALID_HANDLE_VALUE) {
				THREADENTRY32 te32;
				te32.dwSize = sizeof(THREADENTRY32);
				if (Thread32First(hThreadSnap, &te32)) {
					do {
						if (te32.th32OwnerProcessID == GetCurrentProcessId()) {
							if (te32.th32ThreadID == GetCurrentThreadId()) continue;
							HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
							if (hThread != NULL) {
								SuspendThread(hThread);
								CloseHandle(hThread);
							}
						}
					} while (Thread32Next(hThreadSnap, &te32));
				}
				CloseHandle(hThreadSnap);
			}
		}
		inline bool hide_thread(void* hThread) {
			typedef NTSTATUS(NTAPI* pNtSetInformationThread)
				(HANDLE, UINT, PVOID, ULONG);

			NTSTATUS Status;

			// Get NtSetInformationThread
			static pNtSetInformationThread NtSIT = (pNtSetInformationThread)
				GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")),
					"NtSetInformationThread");
			// Shouldn't fail
			if (NtSIT == NULL)
				return false;

			// Set the thread info
			if (hThread == NULL)
				Status = NtSIT(GetCurrentThread(),
					0x11, //ThreadHideFromDebugger
					0, 0);
			else
				Status = NtSIT(hThread, 0x11, 0, 0);

			if (Status != 0x00000000)
				return false;
			else
				return true;
			return false;
		}
	}
}