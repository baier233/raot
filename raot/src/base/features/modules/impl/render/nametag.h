#pragma once
#include <base/features/modules/abstract_module.h>
#include <base/features/events/events.h>
#include <imgui/imgui.h>
class nametag : public instance_module<nametag> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	void on_render_2d(const render_2d_event& e);
	nametag();
protected:

	DEFINE_BOOL_VALUE(text, "Text", "", true);
	DEFINE_BOOL_VALUE(tex_outlined, "Text Outlined", "", true);
	DEFINE_FLOAT_VALUE(text_size, "Text Size", "", 18.f, 12.f, 24.f);
};
