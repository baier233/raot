#pragma once
#include <base/features/modules/abstract_module.h>
#include <base/features/events/events.h>
#include <imgui/imgui.h>
class freecam : public instance_module<freecam> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	freecam();
protected:

};
