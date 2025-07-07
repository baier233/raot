#pragma once
#include <sdk.h>
class MirrorClientObjectNetwork : public II::MonoBehaviour
{
public:
	inline static I::MethodPointer < II::Int32, MirrorClientObjectNetwork* > get_fps = nullptr;
	inline static I::MethodPointer < II::String*, MirrorClientObjectNetwork* > get_network_sync_name = nullptr;


	static auto init() -> void;
};

