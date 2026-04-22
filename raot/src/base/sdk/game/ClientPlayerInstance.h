#pragma once
#include <sdk.h>
#include "CharacterObject.h"
class ClientPlayerInstance : public II::MonoBehaviour
{
public:
	static inline I::MethodPointer<II::Transform*, ClientPlayerInstance*> get_transform = nullptr;
	static inline I::MethodPointer<CharacterObject*, ClientPlayerInstance*> get_character = nullptr;
	static inline I::MethodPointer<bool, ClientPlayerInstance*> get_is_active = nullptr;
	static auto init() -> void;
};

