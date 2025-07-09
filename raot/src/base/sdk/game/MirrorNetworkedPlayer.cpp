#include "MirrorNetworkedPlayer.h"
#include "MirrorClientObject.h"
auto MirrorNetworkedPlayer::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkedPlayer")) {
		get_client = pClass->Get<IM>("get_OwnerClient")->Cast<MirrorClientObject*, MirrorNetworkedPlayer*>();
	}
}
