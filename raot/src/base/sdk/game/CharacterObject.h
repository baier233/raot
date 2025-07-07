#pragma once
#include <sdk.h>
class CharacterObject : public II::MonoBehaviour
{
public:


	static inline I::MethodPointer<II::Transform*, CharacterObject*> get_headbone = nullptr;
	static auto init() -> void;
};

