#include "apg_aimbot.h"
#include <sdk.h>
#include <game/MirrorClientObject.h>
#include <game/ClientPlayerInstance.h>
#include <game/CharacterObject.h>
#include <game/MirrorClientModule.h>
#include <game/AoTNetworkModule.h>
#include <game/PlayerMain.h>
#include <chrono>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <numbers>
#include <optional>
#include <iostream>
#include <Windows.h>
#undef max
#undef min

#define AIM_LOG(x) do { std::cout << "[apg_aimbot] " << x << std::endl; } while(0)

namespace {
using cclk = std::chrono::steady_clock;
constexpr float kPi = std::numbers::pi_v<float>;
constexpr float kRad2Deg = 180.f / kPi;

struct sample_t {
	cclk::time_point t;
	II::Vector3 pos;
};

std::mutex g_history_mutex;
std::unordered_map<MirrorClientObject*, std::deque<sample_t>> g_history;

I::MethodPointer<MirrorClientObject*> fn_get_my_client = nullptr;

// Persistent smoothing state across on_update ticks.
struct smooth_state_t {
	float pitch;
	float yaw;
	cclk::time_point last_tick;
	MirrorClientObject* target;
	bool locked;
};
std::optional<smooth_state_t> g_smooth_state;

// Offsets resolved at ctor (fallback defaults from IDA of RAoT 2.086).
// Camera:
//   PlayerMain.<Camera>k__BackingField           -> CameraController*
//   CameraControlBase.targetEuler  (Vector3)     -> desired pitch/yaw (degrees)
//   CameraControlBase.currentEuler (Vector3)     -> interpolated pitch/yaw (degrees)
int32_t off_player_camera     = 0x88;
int32_t off_target_euler      = 0x48;   // 0x10 (_o header) + 0x38 (field)
int32_t off_current_euler     = 0x54;   // 0x10 (_o header) + 0x44 (field)
// Charge ("ready attack") state chain:
//   CombatManager._ChargeHandler_k__BackingField       -> PlayerCombatChargeHandler*
//   PlayerCombatChargeHandler._LeftHandler / _RightHandler -> PlayerCombatChargeSubHandler*
//   PlayerCombatChargeSubHandler.attackChargeTime (float) == -1.0  -> not charging
int32_t off_cm_charge_handler = 0x38;   // CombatManager, fields 0x28 + header 0x10
int32_t off_ch_left_handler   = 0x18;   // ChargeHandler,  fields 0x08 + header 0x10
int32_t off_ch_right_handler  = 0x20;   // ChargeHandler,  fields 0x10 + header 0x10
int32_t off_sub_charge_time   = 0x28;   // SubHandler,     fields 0x18 + header 0x10

inline II::Vector3 v3_sub(const II::Vector3& a, const II::Vector3& b) { return {a.x-b.x, a.y-b.y, a.z-b.z}; }
inline II::Vector3 v3_add(const II::Vector3& a, const II::Vector3& b) { return {a.x+b.x, a.y+b.y, a.z+b.z}; }
inline II::Vector3 v3_mul(const II::Vector3& a, float s)              { return {a.x*s, a.y*s, a.z*s}; }
inline float       v3_dot(const II::Vector3& a, const II::Vector3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline float       v3_len(const II::Vector3& a)                       { return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z); }
inline II::Vector3 v3_norm(const II::Vector3& a) {
	float l = v3_len(a);
	if (l < 1e-6f) return {0, 0, 1};
	return {a.x / l, a.y / l, a.z / l};
}

II::Vector3 estimate_velocity(const std::deque<sample_t>& samples, int window_ms) {
	if (samples.size() < 2) return {0, 0, 0};
	const auto& newest = samples.back();
	auto cutoff = newest.t - std::chrono::milliseconds(window_ms);
	sample_t oldest = samples.front();
	for (const auto& s : samples) {
		if (s.t >= cutoff) { oldest = s; break; }
	}
	float dt = std::chrono::duration<float>(newest.t - oldest.t).count();
	if (dt < 1e-4f) return {0, 0, 0};
	return v3_mul(v3_sub(newest.pos, oldest.pos), 1.f / dt);
}

II::Vector3 compute_aim_point(const II::Vector3& shooter,
                              const II::Vector3& target_pos,
                              const II::Vector3& target_vel,
                              float proj_speed,
                              float max_lead) {
	II::Vector3 predicted = target_pos;
	for (int i = 0; i < 3; ++i) {
		float distance = v3_len(v3_sub(predicted, shooter));
		float ttime    = distance / std::max(proj_speed, 1.f);
		II::Vector3 lead = v3_mul(target_vel, ttime);
		float lead_len = v3_len(lead);
		if (max_lead > 0.f && lead_len > max_lead) {
			lead = v3_mul(lead, max_lead / lead_len);
		}
		predicted = v3_add(target_pos, lead);
	}
	return predicted;
}

void direction_to_euler_deg(const II::Vector3& dir, float& pitch_deg, float& yaw_deg) {
	float y = std::clamp(dir.y, -1.f, 1.f);
	pitch_deg = -std::asin(y) * kRad2Deg;
	yaw_deg   =  std::atan2(dir.x, dir.z) * kRad2Deg;
}

float shortest_angle_delta(float target, float current) {
	float d = std::fmod(target - current + 540.f, 360.f) - 180.f;
	return d;
}

// Read a pointer field
inline void* read_ptr(void* base, int32_t off) {
	return *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(base) + off);
}

// Game's "ready attack" gate. Walk PlayerMain -> CombatManager -> ChargeHandler -> sub-handlers.
// Returns:
//   left_ready, right_ready  = true if that side's attackChargeTime != -1.0
bool is_charge_active(PlayerMain* pm, bool& left_ready, bool& right_ready) {
	left_ready  = false;
	right_ready = false;
	if (!pm || !PlayerMain::get_combat_manager) return false;
	void* cm = PlayerMain::get_combat_manager(pm);
	if (!cm) return false;
	void* ch = read_ptr(cm, off_cm_charge_handler);
	if (!ch) return false;
	if (void* left = read_ptr(ch, off_ch_left_handler)) {
		float t = *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(left) + off_sub_charge_time);
		if (t != -1.0f) left_ready = true;
	}
	if (void* right = read_ptr(ch, off_ch_right_handler)) {
		float t = *reinterpret_cast<float*>(reinterpret_cast<uint8_t*>(right) + off_sub_charge_time);
		if (t != -1.0f) right_ready = true;
	}
	return left_ready || right_ready;
}

bool trigger_active(apg_aimbot& self, PlayerMain* pm) {
	if (self.always_on->get_value()) return true;

	// Must be in "ready attack" (charging) state first
	bool left_ready = false, right_ready = false;
	if (!is_charge_active(pm, left_ready, right_ready)) return false;

	// Selected button(s) must also be held — this is how we distinguish which side the user
	// wants aimbot on (e.g. only RMB-charged attacks, not LMB-charged).
	bool rmb_held = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	bool lmb_held = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	if (self.hold_rmb->get_value() && rmb_held) return true;
	if (self.hold_lmb->get_value() && lmb_held) return true;
	return false;
}
} // namespace

void apg_aimbot::on_enable() {
	AIM_LOG("on_enable");
}

void apg_aimbot::on_disable() {
	g_smooth_state.reset();
	std::lock_guard<std::mutex> g(g_history_mutex);
	g_history.clear();
	AIM_LOG("on_disable: state cleared");
}

void apg_aimbot::on_update() {
	if (!get_toggle()) {
		g_smooth_state.reset();
		return;
	}

	auto* pm = PlayerMain::get_instance ? PlayerMain::get_instance() : nullptr;
	if (!pm) {
		g_smooth_state.reset();
		return;
	}

	if (!trigger_active(*this, pm)) {
		g_smooth_state.reset();
		return;
	}

	// PlayerMain.<Camera>k__BackingField -> CameraController (inherits CameraControlBase)
	void* cam_ctrl = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pm) + off_player_camera);
	if (!cam_ctrl) return;

	auto* target_euler  = reinterpret_cast<II::Vector3*>(reinterpret_cast<uint8_t*>(cam_ctrl) + off_target_euler);
	auto* current_euler = reinterpret_cast<II::Vector3*>(reinterpret_cast<uint8_t*>(cam_ctrl) + off_current_euler);

	auto* unity_cam_raw = PlayerMain::get_camera_transform ? PlayerMain::get_camera_transform(pm) : nullptr;
	if (!unity_cam_raw) return;
	auto* unity_cam = reinterpret_cast<II::Camera*>(unity_cam_raw);
	auto* unity_cam_tf = unity_cam->GetTransform();
	if (!unity_cam_tf) return;

	II::Vector3 shooter_pos = unity_cam_tf->GetPosition();
	II::Vector3 current_fwd = v3_norm(unity_cam_tf->GetForward());

	if (!MirrorClientModule::cached) return;
	auto* mcm = AoTNetworkModule::get_instance<MirrorClientModule>(MirrorClientModule::cached);
	if (!mcm) return;
	auto clients = mcm->get_clients();
	if (clients.empty()) return;

	if (!fn_get_my_client) {
		if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkHub"))
			fn_get_my_client = pClass->Get<IM>("get_MyClientObject")->Cast<MirrorClientObject*>();
	}
	MirrorClientObject* mine = fn_get_my_client ? fn_get_my_client() : nullptr;

	auto now = cclk::now();
	const float fov_half_rad = fov_deg->get_value() * 0.5f * (kPi / 180.f);
	const int   win_ms       = static_cast<int>(history_window_ms->get_value());
	const float proj_speed   = projectile_speed->get_value();
	const float max_lead     = max_lead_m->get_value();
	const float max_range    = max_range_m->get_value();

	std::lock_guard<std::mutex> g(g_history_mutex);

	MirrorClientObject* best_client = nullptr;
	float               best_ang    = fov_half_rad;
	II::Vector3         best_dir{};
	float               best_dist   = 0.f;

	for (auto* client : clients) {
		if (!client || client == mine) continue;

		auto* pi = MirrorClientObject::get_player ? MirrorClientObject::get_player(client) : nullptr;
		if (!pi) continue;

		if (ClientPlayerInstance::get_is_active && !ClientPlayerInstance::get_is_active(pi)) continue;

		II::Vector3 target_pos;
		if (aim_head->get_value()) {
			auto* ch = ClientPlayerInstance::get_character ? ClientPlayerInstance::get_character(pi) : nullptr;
			if (!ch) continue;
			auto* head = CharacterObject::get_headbone ? CharacterObject::get_headbone(ch) : nullptr;
			if (!head) continue;
			target_pos = head->GetPosition();
		} else {
			auto* tr = ClientPlayerInstance::get_transform ? ClientPlayerInstance::get_transform(pi) : nullptr;
			if (!tr) continue;
			target_pos = tr->GetPosition();
		}

		if (v3_len(target_pos) < 1e-3f) continue;

		auto& dq = g_history[client];
		dq.push_back({now, target_pos});
		auto hist_cutoff = now - std::chrono::milliseconds(win_ms * 2 + 200);
		while (!dq.empty() && dq.front().t < hist_cutoff) dq.pop_front();

		II::Vector3 aim_point;
		if (prediction->get_value()) {
			II::Vector3 vel = estimate_velocity(dq, win_ms);
			aim_point = compute_aim_point(shooter_pos, target_pos, vel, proj_speed, max_lead);
		} else {
			aim_point = target_pos;
		}

		II::Vector3 to_target = v3_sub(aim_point, shooter_pos);
		float dist = v3_len(to_target);
		if (dist < 0.1f || dist > max_range) continue;
		II::Vector3 dir = v3_mul(to_target, 1.f / dist);

		float d   = std::clamp(v3_dot(current_fwd, dir), -1.f, 1.f);
		float ang = std::acos(d);
		if (ang < best_ang) {
			best_ang    = ang;
			best_client = client;
			best_dir    = dir;
			best_dist   = dist;
		}
	}

	if (!best_client) {
		g_smooth_state.reset();
		return;
	}

	float aim_pitch, aim_yaw;
	direction_to_euler_deg(best_dir, aim_pitch, aim_yaw);

	auto now_time = cclk::now();

	if (smooth->get_value()) {
		if (!g_smooth_state || g_smooth_state->target != best_client) {
			g_smooth_state = smooth_state_t{
				current_euler->x, current_euler->y, now_time, best_client, false
			};
		}

		if (g_smooth_state->locked) {
			g_smooth_state->pitch = aim_pitch;
			g_smooth_state->yaw   = aim_yaw;
		} else {
			float dt = std::chrono::duration<float>(now_time - g_smooth_state->last_tick).count();
			dt = std::clamp(dt, 0.f, 0.1f);

			float k = std::max(smooth_factor->get_value(), 0.01f) * 30.f;
			float alpha = 1.f - std::exp(-k * dt);

			float dp = aim_pitch - g_smooth_state->pitch;
			float dy = shortest_angle_delta(aim_yaw, g_smooth_state->yaw);
			g_smooth_state->pitch += dp * alpha;
			g_smooth_state->yaw   += dy * alpha;

			float rem_dp = aim_pitch - g_smooth_state->pitch;
			float rem_dy = shortest_angle_delta(aim_yaw, g_smooth_state->yaw);
			float err    = std::sqrt(rem_dp * rem_dp + rem_dy * rem_dy);
			if (err < lock_threshold_deg->get_value()) {
				g_smooth_state->locked = true;
				g_smooth_state->pitch = aim_pitch;
				g_smooth_state->yaw   = aim_yaw;
				AIM_LOG("locked on client=" << best_client << " dist=" << best_dist << "m");
			}
		}

		g_smooth_state->last_tick = now_time;

		current_euler->x = g_smooth_state->pitch;
		current_euler->y = g_smooth_state->yaw;
		target_euler->x  = g_smooth_state->pitch;
		target_euler->y  = g_smooth_state->yaw;
	} else {
		current_euler->x = aim_pitch;
		current_euler->y = aim_yaw;
		target_euler->x  = aim_pitch;
		target_euler->y  = aim_yaw;
	}
}

apg_aimbot::apg_aimbot() : instance_module("apg_aimbot", category::COMBAT) {
	add_value(values::BOOL,  always_on);
	add_value(values::BOOL,  hold_rmb);
	add_value(values::BOOL,  hold_lmb);
	add_value(values::FLOAT, fov_deg);
	add_value(values::FLOAT, max_range_m);
	add_value(values::BOOL,  aim_head);
	add_value(values::BOOL,  prediction);
	add_value(values::FLOAT, projectile_speed);
	add_value(values::FLOAT, history_window_ms);
	add_value(values::FLOAT, max_lead_m);
	add_value(values::BOOL,  smooth);
	add_value(values::FLOAT, smooth_factor);
	add_value(values::FLOAT, lock_threshold_deg);

	// Camera offsets
	if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerMain")) {
		if (auto f = pClass->Get<IF>("<Camera>k__BackingField")) {
			off_player_camera = f->offset;
			AIM_LOG("PlayerMain.<Camera>k__BackingField offset = 0x" << std::hex << off_player_camera << std::dec);
		}
	}
	auto pCamBase = I::Get("Assembly-CSharp.dll")->Get("CameraControlBase");
	if (!pCamBase) pCamBase = I::Get("Assembly-CSharp.dll")->Get("CameraControlBase", "Player.Cameras");
	if (pCamBase) {
		if (auto f = pCamBase->Get<IF>("targetEuler"))  off_target_euler  = f->offset;
		if (auto f = pCamBase->Get<IF>("currentEuler")) off_current_euler = f->offset;
		AIM_LOG("CameraControlBase.targetEuler/currentEuler offsets = 0x"
			<< std::hex << off_target_euler << " / 0x" << off_current_euler << std::dec);
	}

	// Charge-state offsets ("Ready Attack" chain)
	if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("CombatManager")) {
		if (auto f = pClass->Get<IF>("<ChargeHandler>k__BackingField")) {
			off_cm_charge_handler = f->offset;
		}
	}
	auto pCh = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatChargeHandler");
	if (!pCh) pCh = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatChargeHandler", "Player.Managers.Combat");
	if (!pCh) pCh = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatChargeHandler", "Player.Managers");
	if (pCh) {
		if (auto f = pCh->Get<IF>("<LeftHandler>k__BackingField"))  off_ch_left_handler  = f->offset;
		if (auto f = pCh->Get<IF>("<RightHandler>k__BackingField")) off_ch_right_handler = f->offset;
	}
	auto pSub = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatChargeSubHandler");
	if (!pSub) pSub = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatChargeSubHandler", "Player.Managers.Combat");
	if (pSub) {
		if (auto f = pSub->Get<IF>("attackChargeTime")) off_sub_charge_time = f->offset;
	}
	AIM_LOG("Charge offsets: CM.ChargeHandler=0x" << std::hex << off_cm_charge_handler
		<< " CH.Left=0x"  << off_ch_left_handler
		<< " CH.Right=0x" << off_ch_right_handler
		<< " Sub.attackChargeTime=0x" << off_sub_charge_time << std::dec);
}
