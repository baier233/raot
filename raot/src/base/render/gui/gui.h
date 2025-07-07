#pragma once
#include <imgui/imgui.h>
namespace gui
{
	void do_render();
	inline bool show = false;

	inline int height;
	inline int width;
	inline ImFont* font;
};

