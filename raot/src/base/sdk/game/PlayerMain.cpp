#include "PlayerMain.h"
#include <iostream>

void* PlayerMain::get_parry_handler()
{
	auto cm = this->get_combat_manager(this);
	if (!cm) { std::cout << "[PlayerMain] get_parry_handler: CombatManager is null" << std::endl; return nullptr; }
	static I::MethodPointer<void*, void*> get_ph = nullptr;
	if (!get_ph) {
		if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("CombatManager"))
			get_ph = pClass->Get<IM>("get_ParryHandler")->Cast<void*, void*>();
		std::cout << "[PlayerMain] cached get_ParryHandler = " << (void*)get_ph << std::endl;
	}
	if (!get_ph) return nullptr;
	void* ph = get_ph(cm);
	if (!ph) std::cout << "[PlayerMain] get_parry_handler: ParryHandler is null" << std::endl;
	return ph;
}

bool PlayerMain::is_parrying()
{
	auto ph = get_parry_handler();
	if (!ph) return false;
	static I::MethodPointer<bool, void*> get_is = nullptr;
	if (!get_is) {
		if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatParryHandler"))
			get_is = pClass->Get<IM>("get_IsParrying")->Cast<bool, void*>();
	}
	return get_is ? get_is(ph) : false;
}

void PlayerMain::start_parry(int attack_type, bool is_special)
{
	auto ph = get_parry_handler();
	if (!ph) { std::cout << "[PlayerMain] start_parry: parry handler null, abort" << std::endl; return; }
	static I::MethodPointer<void, void*, int, bool> fn = nullptr;
	if (!fn) {
		if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatParryHandler"))
			fn = pClass->Get<IM>("StartParry")->Cast<void, void*, int, bool>();
		std::cout << "[PlayerMain] cached StartParry = " << (void*)fn << std::endl;
	}
	if (!fn) { std::cout << "[PlayerMain] start_parry: StartParry method unresolved" << std::endl; return; }
	std::cout << "[PlayerMain] StartParry(handler=" << ph << ", type=" << attack_type << ", isSpecial=" << is_special << ")" << std::endl;
	fn(ph, attack_type, is_special);
}

auto PlayerMain::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerMain")) {
		get_instance          = pClass->Get<IM>("get_Instance")    ->Cast<PlayerMain*>();
		get_combat_manager    = pClass->Get<IM>("get_CombatManager")->Cast<void*, PlayerMain*>();
		get_camera_transform  = pClass->Get<IM>("get_PlayerCamera") ->Cast<II::Transform*, PlayerMain*>();
	}
}
