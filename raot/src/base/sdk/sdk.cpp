#include "sdk.h"
#include <xcall_once.h>
#include <game/ClientPlayerInstance.h>
#include <game/MirrorClientObject.h>
#include <game/MirrorClientModule.h>
#include <game/AoTNetworkModule.h>
bool sdk::init(bool dump_sdk)
{
	UnityResolve::Init(GetModuleHandle(L"GameAssembly.dll"), UnityResolve::Mode::Il2Cpp);
	if (!sdk::attach())
	{
		return false;
	}
	if (dump_sdk)
	{
		UnityResolve::DumpToFile("sdk\\raot_");
	}
	ClientPlayerInstance::init();
	MirrorClientObject::init();
	MirrorClientModule::init();
	AoTNetworkModule::init();
	return true;
}

bool sdk::attach()
{
	static thread_local std::once_flag flag;
	std::call_once(flag, []() {
		UnityResolve::ThreadAttach();
		});
	return true;
}
