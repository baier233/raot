#pragma once
#include <sdk.h>
#include "ClientPlayerInstance.h"
class MirrorClientObject : public II::MonoBehaviour
{
public:
	inline static I::MethodPointer < ClientPlayerInstance*, MirrorClientObject* > get_player = nullptr;

	static auto init() -> void;
};

