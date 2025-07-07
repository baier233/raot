#include "CharacterObject.h"

auto CharacterObject::init() -> void
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("CharacterObject")) {
		get_headbone = pClass->Get<IM>("get_HeadBone")->Cast<II::Transform*, CharacterObject*>();
	}
}
