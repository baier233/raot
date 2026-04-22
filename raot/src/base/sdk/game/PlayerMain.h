#pragma once
#include <sdk.h>
class PlayerMain : public II::MonoBehaviour
{
public:


	static inline I::MethodPointer<PlayerMain*> get_instance = nullptr;
	static inline I::MethodPointer<void*, PlayerMain*> get_combat_manager = nullptr;
	static inline I::MethodPointer<II::Transform*, PlayerMain*> get_camera_transform = nullptr;

	void* get_parry_handler();
	bool  is_parrying();
	void  start_parry(int attack_type, bool is_special);

	static auto init() -> void;
};
