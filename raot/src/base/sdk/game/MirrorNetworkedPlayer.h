#pragma once
#include <sdk.h>
#include "MirrorClientObject.h"
#include "MirrorNetworkedPlayerNetwork.h"
class MirrorNetworkedPlayer : public MirrorNetworkedPlayerNetwork
{
public:
	static auto init() -> void;
	inline static I::MethodPointer < MirrorClientObject*, MirrorNetworkedPlayer* > get_client = nullptr;
};

