#pragma once
#include <sdk.h>
#include "ClientPlayerInstance.h"
#include "MirrorClientObjectNetwork.h"
class MirrorClientObject : public MirrorClientObjectNetwork
{
public:
	inline static I::MethodPointer < ClientPlayerInstance*, MirrorClientObject* > get_player = nullptr;

	static auto init() -> void;
};

