#pragma once
#include <base/features/modules/abstract_module.h>
#include <base/features/events/events.h>
#include <imgui/imgui.h>
class auto_parry : public instance_module<auto_parry> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	auto_parry();
protected:

};
