#include "AoTNetworkModule.h"


auto AoTNetworkModule::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("AoTNetworkModule")) {
		cached = pClass;
		field_Instances = pClass->Get<IF>("instances");
	}
}
