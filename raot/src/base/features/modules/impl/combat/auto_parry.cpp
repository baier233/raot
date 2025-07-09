#include "auto_parry.h"
#include <sdk.h>
#include <titan_hook.h>
#include <game/MirrorNetworkedPlayer.h>
class NetworkReader {
public:
	auto ToString() -> II::String* {

		static IM* method;
		if (!method) method = I::Get("Mirror.dll")->Get("NetworkReader", "Mirror")->Get<IM>("ToString");
		if (method) return method->Invoke<II::String*>(this);
		return {};
	}
};
#include <utils/timer_util.hpp>
static utils::TimerUtil timer;
static std::atomic<bool> parrying = false;
#include <game/PlayerMain.h>
static void detour_InvokeUserCode_RpcTriggerFireWeapon(MirrorNetworkedPlayer* player, NetworkReader* object, void* sender_connection);
#include <base/render/d3dhook.h>
TitanHook<decltype(&detour_InvokeUserCode_RpcTriggerFireWeapon)> fire_weapon_hook;
static void detour_InvokeUserCode_RpcTriggerFireWeapon(MirrorNetworkedPlayer* player, NetworkReader* object, void* sender_connection) {
	//if (!auto_parry::instance().get_toggle()) return;
	auto str_obj = object->ToString();
	if (str_obj)
	{
		auto client = player->get_client(player);
		auto str = str_obj->ToString();
		auto name_obj = client->get_network_sync_name(client);
		if (name_obj)
		{
			auto name = name_obj->ToString();
			auto look_at = *player->get_sync_lookat();
			if (name != "Basil123")
			{
				if (str.find("buffer=08") != std::string::npos)
				{
					PlayerMain::get_instance()->set_parry(true);
					parrying = true;
					timer.reset();
					////press mouse5
					//SendMessage(dx_hook::Hk11::GetHwnd(), WM_XBUTTONDOWN, MAKEWPARAM(0, MK_XBUTTON2), 0);
					//SendMessage(dx_hook::Hk11::GetHwnd(), WM_XBUTTONUP, MAKEWPARAM(0, MK_XBUTTON2), 0);
					return;
				}
			}
			std::cout << "Player: " << name << " triggered weapon fire with buf: " << str << std::endl;
			std::cout << "lookat " << look_at.x << " " << look_at.y << " " << look_at.z << std::endl;
		}

	}
	return fire_weapon_hook.GetOrignalFunc()(player, object, sender_connection);
}
void auto_parry::on_disable()
{


}

void auto_parry::on_enable()
{

}

void auto_parry::on_update()
{
	if (!get_toggle()) return;
	if (parrying && timer.has_time_elapsed(500, false))
	{
		PlayerMain::get_instance()->set_parry(false);
		parrying = false;
	}

}
#include <base/render/gui/gui.h>


auto_parry::auto_parry() : instance_module("auto_parry", category::COMBAT)
{
	if (const auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkedPlayerNetwork")) {
		auto addr = pClass->Get<IM>("InvokeUserCode_RpcTriggerFireWeapon")->function;
		fire_weapon_hook.InitHook(addr, &detour_InvokeUserCode_RpcTriggerFireWeapon);
		fire_weapon_hook.SetHook();
	}
}
