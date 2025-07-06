#include "unloaded_module.h"

void cleaner::unloaded_module::clean()
{
	/*Write a 'retn' instruction into the begin of LdrpRecordUnloadEvent function,for disabling the unloaded module log*/
	auto ntdll = GetModuleHandleW(L"ntdll.dll");
	// this pattern is only for win11
	auto pLdrpRecordUnloadEvent = utils::pattern::find((uintptr_t)ntdll, "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 41 54 41 55 41 56 41 57 48 83 EC 30 8B 05 ? ? ? ?");
	if (!pLdrpRecordUnloadEvent) return;
	{

		utils::scoped_vprotect vp((PVOID)pLdrpRecordUnloadEvent, 0x1, PAGE_EXECUTE_READWRITE);
		*(BYTE*)pLdrpRecordUnloadEvent = 0xC3;//insn : RETN
	}
}
