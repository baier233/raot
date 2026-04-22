#include "ClientPlayerInstance.h"

void ClientPlayerInstance::init()
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("ClientPlayerInstance")) {
		get_transform = pClass->Get<IM>("get_Transform")->Cast<II::Transform*, ClientPlayerInstance*>();
		get_character = pClass->Get<IM>("get_CharacterObject")->Cast<CharacterObject*, ClientPlayerInstance*>();
		get_is_active = pClass->Get<IM>("get_IsActive")->Cast<bool, ClientPlayerInstance*>();
	}
}
