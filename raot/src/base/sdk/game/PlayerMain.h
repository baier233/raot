#pragma once
#include <sdk.h>
class PlayerMain : public II::MonoBehaviour
{
public:


	static inline I::MethodPointer<PlayerMain*> get_instance = nullptr;
	static inline I::MethodPointer<void*, PlayerMain*> get_combat_manager = nullptr;

	void set_parry(bool value);
	static auto init() -> void;
};

