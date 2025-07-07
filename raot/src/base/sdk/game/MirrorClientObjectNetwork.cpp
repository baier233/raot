#include "MirrorClientObjectNetwork.h"

auto MirrorClientObjectNetwork::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorClientObjectNetwork")) {
		get_fps = pClass->Get<IM>("get_FPS")->Cast<II::Int32, MirrorClientObjectNetwork*>();
		get_network_sync_name = pClass->Get<IM>("get_NetworkSyncUsername")->Cast<II::String*, MirrorClientObjectNetwork*>();
	}
}
