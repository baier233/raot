#include "gui.h"
#include <d3d11.h>
#include <dxgi.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <string>
#include <format>
#include <base/render/d3dhook.h>
#include <base/sdk/sdk.h>

#include <base/features/events/events.h>
#include <base/render/fonts/fonts.h>
extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

static void render_gui_internal() {
	ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_FirstUseEver);
	ImGui::Begin("hello");
	ImGui::Text("Hello, World!");
	ImGui::Text(std::format("{}", ImGui::GetIO().Framerate).c_str());
	ImGui::End();
}

void gui::do_render()
{
	static std::once_flag flag;
	std::call_once(flag, []() {
		fonts::init();
		tagRECT rect{};
		GetClientRect(dx_hook::Hk11::GetHwnd(), &rect);
		gui::width = rect.right - rect.left;
		gui::height = rect.bottom - rect.top;
		sdk::attach();
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(dx_hook::Hk11::GetHwnd());
		ImGui_ImplDX11_Init(dx_hook::Hk11::GetDevice(), dx_hook::Hk11::GetContext());

		auto& io = ImGui::GetIO();
		ImFontConfig config{};
		config.FontDataOwnedByAtlas = false;
		gui::font = io.Fonts->AddFontFromMemoryTTF(fonts::harmony_data, fonts::harmony_size, 17.f, &config, io.Fonts->GetGlyphRangesChineseFull());

		dx_hook::Hk11::SetWndProc([](const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) -> char {
			POINT mPos;
			GetCursorPos(&mPos);
			ScreenToClient(dx_hook::Hk11::GetHwnd(), &mPos);
			ImGui::GetIO().MousePos.x = static_cast<float>(mPos.x);
			ImGui::GetIO().MousePos.y = static_cast<float>(mPos.y);
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

			switch (msg) {
			case WM_KEYDOWN:
				if (wParam == VK_DELETE) {
					gui::show = !gui::show;
				}
				break;
				/*	case WM_SIZE:
					{
						int width = LOWORD(lParam);
						int height = HIWORD(lParam);
						gui::height = height;
						gui::width = width;
						break;
					}*/
			case WM_KEYUP: break;
			case WM_CLOSE:
				exit(0);
				break;

			}


			return !gui::show;
			});
		});

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGuiContext* context = ImGui::GetCurrentContext();
	if (context->WithinFrameScope)
	{
		ImGui::Begin("Overlay", nullptr,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoBackground);
		raot::get().event_bus.fire_event(render_2d_event{});
		ImGui::End();


		if (gui::show)
		{
			ImGui::GetIO().MouseDrawCursor = true;

			render_gui_internal();
		}
		else {
			ImGui::GetIO().MouseDrawCursor = false;

		}

	}
	ImGui::EndFrame();
	ImGui::Render();
	dx_hook::Hk11::GetContext()->OMSetRenderTargets(1, dx_hook::Hk11::GetTargetView(), nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
