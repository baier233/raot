#include "MirrorNetworkedPlayerNetwork.h"

auto MirrorNetworkedPlayerNetwork::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkedPlayerNetwork")) {
		auto addr = pClass->Get<IM>("InvokeUserCode_CmdRequestWeaponDurability")->address;
		field_sync_lookat = pClass->Get<IF>("_syncLookAt");
	}
}
