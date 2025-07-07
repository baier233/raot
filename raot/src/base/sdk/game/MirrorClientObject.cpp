#include "MirrorClientObject.h"

auto MirrorClientObject::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorClientObject")) {
		get_player = pClass->Get<IM>("get_Player")->Cast<ClientPlayerInstance*, MirrorClientObject*>();
	}
}
