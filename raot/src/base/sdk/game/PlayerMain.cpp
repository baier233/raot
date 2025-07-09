#include "PlayerMain.h"

void PlayerMain::set_parry(bool value)
{
	auto combat_manager = this->get_combat_manager(this);
	if (combat_manager) {
		static I::MethodPointer<void*, void*> get_parry_handler_method = nullptr;
		if (!get_parry_handler_method) {
			get_parry_handler_method = I::Get("Assembly-CSharp.dll")->Get("CombatManager")->Get<IM>("get_ParryHandler")->Cast<void*, void*>();
		}
		if (get_parry_handler_method) {
			auto handler = get_parry_handler_method(combat_manager);
			if (handler)
			{
				static I::MethodPointer<void, void*, bool> set_parry_method = nullptr;
				if (!set_parry_method) {
					set_parry_method = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatParryHandler")->Get<IM>("set_IsParrying")->Cast<void, void*, bool>();
				}
				if (set_parry_method) {
					set_parry_method(handler, value);
				}
			}
			else {
				std::cerr << "CombatManager ParryHandler is null!" << std::endl;
			}
		}
	}

}

auto PlayerMain::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerMain")) {
		get_instance = pClass->Get<IM>("get_Instance")->Cast<PlayerMain*>();
		get_combat_manager = pClass->Get<IM>("get_CombatManager")->Cast<void*, PlayerMain*>();
	}
}
