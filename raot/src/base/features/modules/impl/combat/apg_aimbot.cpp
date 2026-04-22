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

// Offsets resolved at ctor (fallback defaults from IDA of RAoT 2.086):
//   PlayerMain.<Camera>k__BackingField           → CameraController*
//   CameraControlBase.targetEuler  (Vector3)     → desired pitch/yaw in degrees
//   CameraControlBase.currentEuler (Vector3)     → interpolated pitch/yaw in degrees
int32_t off_player_camera  = 0x88;
int32_t off_target_euler   = 0x48;   // 0x10 (_o header) + 0x38 (field)
int32_t off_current_euler  = 0x54;   // 0x10 (_o header) + 0x44 (field)

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

// Unity ZXY Euler order:
//   forward.x = sin(yaw)  * cos(pitch)
//   forward.y = -sin(pitch)
//   forward.z = cos(yaw)  * cos(pitch)
// Inverse:
//   pitch = -asin(forward.y)
//   yaw   =  atan2(forward.x, forward.z)
void direction_to_euler_deg(const II::Vector3& dir, float& pitch_deg, float& yaw_deg) {
	float y = std::clamp(dir.y, -1.f, 1.f);
	pitch_deg = -std::asin(y) * kRad2Deg;
	yaw_deg   =  std::atan2(dir.x, dir.z) * kRad2Deg;
}

// Normalize angle delta to shortest path in [-180, 180]
float shortest_angle_delta(float target, float current) {
	float d = std::fmod(target - current + 540.f, 360.f) - 180.f;
	return d;
}

bool trigger_active(apg_aimbot& self) {
	if (self.always_on->get_value()) return true;
	if (self.hold_rmb->get_value()) {
		return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	}
	return false;
}
} // namespace

void apg_aimbot::on_enable() {
	AIM_LOG("on_enable");
}

void apg_aimbot::on_disable() {
	std::lock_guard<std::mutex> g(g_history_mutex);
	g_history.clear();
	AIM_LOG("on_disable: history cleared");
}

void apg_aimbot::on_update() {
	if (!get_toggle() || !trigger_active(*this)) return;

	auto* pm = PlayerMain::get_instance ? PlayerMain::get_instance() : nullptr;
	if (!pm) return;

	// PlayerMain.<Camera>k__BackingField → CameraController (inherits CameraControlBase)
	void* cam_ctrl = *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(pm) + off_player_camera);
	if (!cam_ctrl) return;

	auto* target_euler  = reinterpret_cast<II::Vector3*>(reinterpret_cast<uint8_t*>(cam_ctrl) + off_target_euler);
	auto* current_euler = reinterpret_cast<II::Vector3*>(reinterpret_cast<uint8_t*>(cam_ctrl) + off_current_euler);

	// Shooter position & current forward for target selection (read from the Unity Camera transform)
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

	if (!best_client) return;

	// Convert aim direction → pitch/yaw in degrees
	float aim_pitch, aim_yaw;
	direction_to_euler_deg(best_dir, aim_pitch, aim_yaw);

	if (smooth->get_value()) {
		// Blend from currentEuler toward aim, shortest path in yaw
		float t = std::clamp(smooth_factor->get_value(), 0.f, 1.f);
		float dp = aim_pitch - current_euler->x;
		float dy = shortest_angle_delta(aim_yaw, current_euler->y);
		target_euler->x = current_euler->x + dp * t;
		target_euler->y = current_euler->y + dy * t;
	} else {
		target_euler->x = aim_pitch;
		target_euler->y = aim_yaw;
	}
	// Leave target_euler->z alone; game zeros it each frame anyway
}

apg_aimbot::apg_aimbot() : instance_module("apg_aimbot", category::COMBAT) {
	add_value(values::BOOL,  always_on);
	add_value(values::BOOL,  hold_rmb);
	add_value(values::FLOAT, fov_deg);
	add_value(values::FLOAT, max_range_m);
	add_value(values::BOOL,  aim_head);
	add_value(values::BOOL,  prediction);
	add_value(values::FLOAT, projectile_speed);
	add_value(values::FLOAT, history_window_ms);
	add_value(values::FLOAT, max_lead_m);
	add_value(values::BOOL,  smooth);
	add_value(values::FLOAT, smooth_factor);

	// Resolve field offsets so we don't depend on version-specific constants.
	if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerMain")) {
		if (auto f = pClass->Get<IF>("<Camera>k__BackingField")) {
			off_player_camera = f->offset;
			AIM_LOG("PlayerMain.<Camera>k__BackingField offset = 0x" << std::hex << off_player_camera << std::dec);
		} else {
			AIM_LOG("WARN: <Camera>k__BackingField not found, using default 0x88");
		}
	}

	auto pCamBase = I::Get("Assembly-CSharp.dll")->Get("CameraControlBase");
	if (!pCamBase) pCamBase = I::Get("Assembly-CSharp.dll")->Get("CameraControlBase", "Player.Cameras");
	if (pCamBase) {
		if (auto f = pCamBase->Get<IF>("targetEuler")) {
			off_target_euler = f->offset;
			AIM_LOG("CameraControlBase.targetEuler offset = 0x" << std::hex << off_target_euler << std::dec);
		} else {
			AIM_LOG("WARN: targetEuler not found, using default 0x48");
		}
		if (auto f = pCamBase->Get<IF>("currentEuler")) {
			off_current_euler = f->offset;
			AIM_LOG("CameraControlBase.currentEuler offset = 0x" << std::hex << off_current_euler << std::dec);
		} else {
			AIM_LOG("WARN: currentEuler not found, using default 0x54");
		}
	} else {
		AIM_LOG("WARN: CameraControlBase class not found");
	}
}
