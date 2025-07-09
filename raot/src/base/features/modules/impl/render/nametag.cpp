#include "nametag.h"
#include <imgui/imgui.h>
#include <game/AoTNetworkModule.h>
#include <game/MirrorClientModule.h>
class Entity {
public:
	std::string name;
	UnityResolve::UnityType::Vector3 headPostion;
	UnityResolve::UnityType::Vector3 headPoint;
};
void nametag::on_disable()
{
}

void nametag::on_enable()
{
}
#include <mutex>
static std::vector<Entity> rs;
static std::mutex rs_mutex;

void nametag::on_update()
{
	if (!get_toggle()) return;
	auto mirror_client_module = AoTNetworkModule::get_instance<MirrorClientModule>(MirrorClientModule::cached);
	if (mirror_client_module)
	{
		rs.clear();
		auto clients = mirror_client_module->get_clients();
		for (auto& client : clients)
		{
			if (!client)
			{
				continue;
			}
			/*auto fps = client->get_fps(client);
			if (fps != 0)
			{
				continue;
			}*/
			auto instance_player = client->get_player(client);
			if (!instance_player)
			{
				continue;
			}
			auto transform = instance_player->get_transform(instance_player);
			if (!transform)
			{
				continue;
			}
			auto character = instance_player->get_character(instance_player);
			if (!character)
			{
				continue;
			}
			auto head_position = character->get_headbone(character);
			if (!head_position)
			{
				continue;
			}

			auto name = client->get_network_sync_name(client);
			if (!name)
			{
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(rs_mutex);
				Entity entity;
				entity.name = name->ToString();
				auto head_pos = head_position->GetPosition();

				entity.headPostion = head_pos;

				head_pos.y += 0.5f;
				try
				{
					auto camera = UnityResolve::UnityType::Camera::GetMain();
					if (camera)
					{
						entity.headPoint = UnityResolve::UnityType::Camera::GetMain()->WorldToScreenPoint(head_pos);
						rs.push_back(entity);
					}
				}
				catch (...)
				{
					continue;
				}
			}

		}
	}
}
#include <base/render/gui/gui.h>

static float TextColor[4]{ 1.0f, 1.0f, 1.0f, 1.0f * 255 };
static float TextOutlineColor[4]{ 0, 0, 0, 1.0f * 255 };
#include <utils/wnd.h>
#include <base/render/d3dhook.h>
void nametag::on_render_2d(const render_2d_event& e)
{

	if (!this->get_toggle()) return;
	auto entites = std::vector<Entity>();
	static auto dpi = utils::wnd::get_pixel_ratio(dx_hook::Hk11::GetHwnd());
	{
		std::lock_guard<std::mutex> lock(rs_mutex);
		entites = rs;
		//rs.clear();
	}
	static auto draw_out_lined_text = [](ImFont* font, float textSize, ImVec2 pos, ImColor color, ImColor outlineColor, const char* begin, const char* end = 0) -> void {
		ImVec2 text_size = font->CalcTextSizeA(textSize, FLT_MAX, 0.0f, begin, end);
		pos.x = pos.x - text_size.x * 0.5f;
		pos.y = pos.y - text_size.y * 0.5f;
		ImGui::GetBackgroundDrawList()->AddText(font, textSize, ImVec2(pos.x + 1, pos.y), outlineColor, begin, end);
		ImGui::GetBackgroundDrawList()->AddText(font, textSize, ImVec2(pos.x - 1, pos.y), outlineColor, begin, end);
		ImGui::GetBackgroundDrawList()->AddText(font, textSize, ImVec2(pos.x, pos.y + 1), outlineColor, begin, end);
		ImGui::GetBackgroundDrawList()->AddText(font, textSize, ImVec2(pos.x, pos.y - 1), outlineColor, begin, end);

		ImGui::GetBackgroundDrawList()->AddText(font, textSize, pos, color, begin, end);
		};

	for (auto& entity : entites)
	{
		if (resize->get_value())
		{
			entity.headPoint.y /= 0.75;
			entity.headPoint.x /= 0.75;

		}
		entity.headPoint.y = gui::height - entity.headPoint.y;



		if ((entity.headPoint.x > 0 && entity.headPoint.y > 0) && (entity.headPoint.x < gui::width && entity.headPoint.y < gui::height) && entity.headPoint.z > 0) {
			draw_out_lined_text(gui::font, text_size->get_value(), ImVec2(entity.headPoint.x, entity.headPoint.y), ImColor(255, 255, 255), ImColor(TextOutlineColor[0], TextOutlineColor[1], TextOutlineColor[2], TextOutlineColor[3]), (const char*)entity.name.c_str(), (const char*)(entity.name.c_str() + entity.name.size()));
			/*ImGui::GetBackgroundDrawList()->AddText(ImVec2(entity.headPoint.x, entity.headPoint.y), ImColor(148, 105, 204), entity.name.c_str());*/
		}
	}


}

nametag::nametag() : instance_module("nametag", category::VISUAL)
{
	REGISTER_EVENT(render_2d_event, nametag::on_render_2d);

	add_value(values::BOOL, text);
	add_value(values::BOOL, tex_outlined);
	add_value(values::FLOAT, text_size);
	add_value(values::BOOL, resize);
}
