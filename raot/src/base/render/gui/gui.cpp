#include "gui.h"
#include <d3d11.h>
#include <dxgi.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui.h>
#include <string>
#include <format>
#include <base/render/d3dhook.h>
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
		ImGui::CreateContext();
		ImGui_ImplWin32_Init(dx_hook::Hk11::GetHwnd());
		ImGui_ImplDX11_Init(dx_hook::Hk11::GetDevice(), dx_hook::Hk11::GetContext());

		auto& io = ImGui::GetIO();
		io.Fonts->AddFontDefault();

		dx_hook::Hk11::SetWndProc([](const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam) -> char {
			POINT mPos;
			GetCursorPos(&mPos);
			ScreenToClient(dx_hook::Hk11::GetHwnd(), &mPos);
			ImGui::GetIO().MousePos.x = static_cast<float>(mPos.x);
			ImGui::GetIO().MousePos.y = static_cast<float>(mPos.y);
			ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);


			switch (msg) {
			case WM_KEYDOWN:
				if ((wParam == VK_DELETE) || (wParam == VK_END)) {
					gui::show = !gui::show;
				}
				break;
			case WM_KEYUP: break;
			case WM_CLOSE: const auto result = MessageBox(nullptr, L"你确定要退出游戏吗?", L"Confirmation", MB_YESNO | MB_ICONQUESTION);

				if (result == IDYES) exit(0);
				if (result == IDNO) return 2;
				break;
			}


			return !gui::show;
			});
		});

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	render_gui_internal();

	ImGui::EndFrame();
	ImGui::Render();
	dx_hook::Hk11::GetContext()->OMSetRenderTargets(1, dx_hook::Hk11::GetTargetView(), nullptr);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
