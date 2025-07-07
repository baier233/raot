#pragma once
#include <sdk.h>
class ClientPlayerInstance : public II::MonoBehaviour
{
public:
	static inline I::MethodPointer<II::Transform*> get_transform = nullptr;
	static auto init() -> void;
};

