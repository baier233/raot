#include "MirrorClientObject.h"

auto MirrorClientObject::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorClientObject")) {
		get_player = pClass->Get<IM>("get_Player")->Cast<ClientPlayerInstance*, MirrorClientObject*>();
		//get_fps = pClass->Get<IM>("get_FPS")->Cast<II::Int32, MirrorClientObject*>();
		//get_network_sync_name = pClass->Get<IM>("get_NetworkSyncUsername")->Cast<II::String*, MirrorClientObject*>();
	}
}


