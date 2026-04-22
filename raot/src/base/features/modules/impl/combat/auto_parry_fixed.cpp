#include "auto_parry.h"
#include <sdk.h>
#include <titanhook.h>
#include <game/MirrorNetworkedPlayer.h>
#include <game/MirrorClientObject.h>
#include <game/PlayerMain.h>
#include <chrono>
#include <deque>
#include <mutex>
#include <cmath>
#include <cstdint>
#include <iostream>

#define API_LOG(x) do { std::cout << "[auto_parry] " << x << std::endl; } while(0)

namespace {
enum ParryableType : uint8_t {
    PT_None = 0, PT_SwordLunge = 1, PT_SwordThrow = 2,
    PT_ApgGunSingle = 3, PT_ApgGunDouble = 4,
    PT_SpearImpaleSingle = 5, PT_SpearImpaleDouble = 6,
    PT_SwordSpin = 7,
};

// Cached method pointers / field offsets. Resolved once in auto_parry() ctor
I::MethodPointer<uint8_t, int, bool>       fn_get_parry_type      = nullptr;
I::MethodPointer<MirrorClientObject*>     fn_get_my_client       = nullptr;
I::MethodPointer<uint32_t>                fn_get_my_client_id    = nullptr;
I::MethodPointer<int, void*>              fn_read_packed_i32     = nullptr;
I::MethodPointer<uint8_t, void*>          fn_read_byte           = nullptr;

// Offsets: default verified via IDA against GameAssembly.dll ROT 2.80.6
int32_t off_data_manager = 0x150; // MirrorNetworkedPlayer.<DataManager>k__BackingField
int32_t off_rpc          = 0x38;  // RemotePlayerDataManager.<LocalRelativeProperties>k__BackingField
int32_t off_rpc_dir      = 0x1C;  // LocalPlayerRelativeProperties.<DirectionToLocalPlayer>k__BackingField
int32_t off_rpc_distance = 0xC2; //                              .<DistanceToLocalPlayer>k__BackingField
int32_t off_rpc_visible  = 0x30; //                              .<IsVisibleToLocalPlayer>k__BackingField
int32_t off_rpc_valid    = 0x38; //                              .<IsValid>k__BackingField

// Mirror NetworkReader: Position at obj+0x20
constexpr int READER_POSITION_OFFSET = 0x20;

// RequestData value-struct layout (struct passed by pointer to CheckForDefensiveParry)
constexpr int REQ_REQUESTER        = 0x0;    // uint32
constexpr int REQ_TARGET          = 0x5;    // uint32
constexpr int REQ_TYPE            = 0x8;    // int32 AttackType
constexpr int REQ_ATTACK_VECTOR   = 0xC;    // Vector3
constexpr int REQ_SPECIAL         = 0x18;   // bool

struct pending_parry {
    std::chrono::steady_clock::time_point when;
    int type;
    bool is_special;
    II::Vector3 desired_forward; // direction from local to attacker
    bool has_forward;
};

struct active_snap {
    std::chrono::steady_clock::time_point until;
    II::Vector3 forward;
};

std::mutex                              pending_mutex;
std::deque<pending_parry>               pending;
std::mutex                              snap_mutex;
std::optional<active_snap>              current_snap;

using invoke_fn     = void(__fastcall*)(void*, void*, void*);
using check_def_fn  = bool(__fastcall*)(void*, void*);                    // (RequestData*, method)
using view_angle_fn = bool(__fastcall*)(float dotLimit, void* target, void* attackVector, void* method);

TitanHook<invoke_fn>      fire_weapon_hook;
TitanHook<check_def_fn>   check_def_parry_hook;
TitanHook<view_angle_fn>  view_angle_hook;

inline void* read_ptr(void* base, int offset) {
    return *reinterpret_cast<void**>(reinterpret_cast<uint8_t*>(base) + offset);
}

//-------------------- host-side hook: force CheckForDefensiveParry = true when target is me --------------------
bool detour_check_def_parry(void* request, void* method) {
    if (!request) return check_def_parry_hook.GetOriginalFunc()(request, method);
    if (!auto_parry::instance().get_toggle() || !auto_parry::instance().host_force_parry->get_value()) {
        return check_def_parry_hook.GetOriginalFunc()(request, method);
    }

    uint32_t target_id = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(request) + REQ_TARGET);
    uint32_t my_id     = fn_get_my_client_id ? fn_get_my_client_id() : 0xFFFFFFu;
    if (target_id == my_id && my_id != 0xFFFFFFu) {
        API_LOG("host hook: CheckForDefensiveParry forced=true (target is me)");
        return true;
    }

    return check_def_parry_hook.GetOriginalFunc()(request, method);
}

//-------------------- host-side hook: IsViewAngleParrying short-circuit when target is me --------------------
bool detour_view_angle(float dotLimit, void* target, void* attackVector, void* method) {
    if (!auto_parry::instance().get_toggle() || !auto_parry::instance().host_force_parry->get_value()) {
        return view_angle_hook.GetOriginalFunc()(dotLimit, target, attackVector, method);
    }

    auto* mine = fn_get_my_client ? fn_get_my_client() : nullptr;
    if (mine && target == mine) {
        API_LOG("host hook: IsViewAngleParrying forced=true (target is me)");
        return true;
    }

    return view_angle_hook.GetOriginalFunc()(dotLimit, target, attackVector, method);
}

//-------------------- inbound fireRPC hook --------------------
void detour_invoke(void* obj_nb, void* reader, void* senderConn) {
    auto call_original = [&] { fire_weapon_hook.GetOriginalFunc()(obj_nb, reader, senderConn); };

    auto self = auto_parry::instance();
    if (!self.get_toggle() || !obj_nb || !reader) {
        call_original();
        return;
    }

    // Peek RPC args without disturbing reader position.
    int32_t saved_pos = *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(reader) + READER_POSITION_OFFSET);
    int   type        = fn_read_packed_i32 ? fn_read_packed_i32(reader) : 0;
    bool  left        = fn_read_byte       ? fn_read_byte      (reader) != 0 : false;
    bool  is_double   = fn_read_byte       ? fn_read_byte      (reader) != 0 : false;
    *reinterpret_cast<int32_t*>(reinterpret_cast<uint8_t*>(reader) + READER_POSITION_OFFSET) = saved_pos;
    (void)left;

    call_original();

    // Type-class enable
    bool allow_type = false;
    switch(type) {
        case 1: case 2: allow_type = self.parry_sword->get_value(); break;
        case 4:         allow_type = self.parry_apg  ->get_value(); break;
        case 10:        allow_type = self.parry_spear->get_value(); break;
        default:
            API_LOG("Unknown attack type: " << type);
            return;
    }
    if (!allow_type) {
        API_LOG("Attack type " << type << " disabled in config");
        return;
    }

    bool is_special = is_double;
    if (!fn_get_parry_type) {
        API_LOG("fn_get_parry_type not available");
        return;
    }
    if (fn_get_parry_type(type, is_special) == PT_None) {
        API_LOG("GetParryTypeFromAttack returned PT_None for type=" << type << " special=" << is_special);
        return;
    }

    // FIXED LOGIC: Check if we have our client object
    auto* mine = fn_get_my_client ? fn_get_my_client() : nullptr;
    if (!mine) {
        API_LOG("Could not get my client object, skip attack processing");
        return;
    }

    auto attacker = reinterpret_cast<MirrorNetworkedPlayer*>(obj_nb);
    auto* owner   = attacker->get_client ? attacker->get_client(attacker) : nullptr;
    if (owner == mine) {
        API_LOG("Ignoring attack from myself");
        return; // Don't parry my own attacks
    }

    void* dm = read_ptr(attacker, off_data_manager);
    if (!dm) {
        API_LOG("Could not read DataManager from attacker");
        return;
    }
    void* rpc = read_ptr(dm, off_rpc);
    if (!rpc) {
        API_LOG("Could not read RPC from DataManager");
        return;
    }

    auto  rpc_u8 = reinterpret_cast<uint8_t*>(rpc);
    bool  valid     = *reinterpret_cast<bool*>    (rpc_u8 + off_rpc_valid);
    float distance  = *reinterpret_cast<float*>   (rpc_u8 + off_rpc_distance);
    bool  visible   = *reinterpret_cast<bool*>    (rpc_u8 + off_rpc_visible);
    II::Vector3 dir_to_local = *reinterpret_cast<II::Vector3*>(rpc_u8 + off_rpc_dir);

    if (!valid) {
        API_LOG("RPC data not valid, skip");
        return;
    }
    if (distance > self.max_range_m->get_value()) {
        API_LOG("Attack too far: " << distance << "m > " << self.max_range_m->get_value() << "m");
        return;
    }
    if (self.require_visible->get_value() && !visible) {
        API_LOG("Attacker not visible, skip");
        return;
    }

    // Compute desired camera forward = - DirectionToLocalPlayer (look back toward attacker)
    II::Vector3 desired_forward{-dir_to_local.x, -dir_to_local.y, -dir_to_local.z};
    float lenF = std::sqrt(desired_forward.x*desired_forward.x + desired_forward.y*desired_forward.y + desired_forward.z*desired_forward.z);
    bool has_forward = lenF > 1e-6f;
    if (has_forward) {
        desired_forward.x /= lenF;
        desired_forward.y /= lenF;
        desired_forward.z /= lenF;
    }

    // Don't re-queue if already parrying
    if (PlayerMain::get_instance()) {
        if (auto* pm = PlayerMain::get_instance(); pm && pm->is_parrying()) {
            API_LOG("Already parrying, skip");
            return;
        }
    }

    auto delay_ms = static_cast<int>(self.reaction_ms->get_value());
    auto when = std::chrono::steady_clock::now() + std::chrono::milliseconds(delay_ms);
    {
        std::lock_guard<std::mutex> g(pending_mutex);
        pending.push_back({when, type, is_special, desired_forward, has_forward});
    }

    API_LOG("QUEUED ATTACK: type=" << type << " isSpecial=" << is_special
            << " dist=" << distance << "m delay=" << delay_ms << "ms valid=" << valid << " visible=" << visible);
}

} // namespace

void auto_parry::on_disable() {
    {
        std::lock_guard<std::mutex> g(pending_mutex);
        pending.clear();
    }
    {
        std::lock_guard<std::mutex> g(snap_mutex);
        current_snap.reset();
    }
    API_LOG("on_disable: state cleared");
}

void auto_parry::on_enable() {
    API_LOG("on_enable");
}

void auto_parry::on_update() {
    if (!get_toggle()) return;

    auto now = std::chrono::steady_clock::now();

    // Pop expired pending entries, keep only the most recent.
    pending_parry shoot{};
    bool have = false;
    {
        std::lock_guard<std::mutex> g(pending_mutex);
        while (!pending.empty() && now >= pending.front().when) {
            shoot = pending.front();
            pending.pop_front();
            have = true;
            API_LOG("TRIGGERING PARRY: type=" << shoot.type << " special=" << shoot.is_special);
        }
    }

    auto* pm = PlayerMain::get_instance() ? PlayerMain::get_instance() : nullptr;
    if (have && pm) {
        // Try both methods: start_parry for animation and set_isParrying for state
        if (!pm->is_parrying()) {
            pm->start_parry(shoot.type, shoot.is_special);
            API_LOG("Called start_parry");

            // Also try to set parrying state directly
            auto ph = pm->get_parry_handler();
            if (ph) {
                // Try to find and call set_IsParrying
                static I::MethodPointer<void, void*, bool> fn_set_parrying = nullptr;
                if (!fn_set_parrying) {
                    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PlayerCombatParryHandler"))
                        fn_set_parrying = pClass->Get<IM>("set_IsParrying")->Cast<void, void*, bool>();
                }
                if (fn_set_parrying) {
                    fn_set_parrying(ph, true);
                    API_LOG("Called set_IsParrying(true)");
                }
            }
        }

        // 2. Arm camera snap if enabled and we have a direction
        if (camera_snap->get_value() && shoot.has_forward) {
            active_snap snap;
            snap.forward = shoot.desired_forward;
            snap.until = now + std::chrono::milliseconds(static_cast<int>(snap_hold_ms->get_value()));
            std::lock_guard<std::mutex> g(snap_mutex);
            current_snap = snap;
            API_LOG("Camera snap armed: forward=(" << snap.forward.x << "," << snap.forward.y << "," << snap.forward.z
                    << ") until=" << (int)snap_hold_ms->get_value() << "ms");
        }
    }

    // 3. Apply / maintain camera snap each tick during the hold window.
    std::optional<active_snap> snap_to_apply;
    {
        std::lock_guard<std::mutex> g(snap_mutex);
        if (current_snap) {
            if (now >= current_snap->until) {
                current_snap.reset();
            } else {
                snap_to_apply = current_snap;
            }
        }
    }

    if (snap_to_apply && pm && PlayerMain::get_camera_transform) {
        auto* cam = PlayerMain::get_camera_transform(pm);
        if (cam) cam->SetForward(snap_to_apply->forward);
    }
}

auto_parry::auto_parry() : instance_module("auto_parry", category::COMBAT) {
    add_value(values::BOOL, parry_sword);
    add_value(values::BOOL, parry_apg);
    add_value(values::BOOL, parry_spear);
    add_value(values::FLOAT, max_range_m);
    add_value(values::BOOL, require_visible);
    add_value(values::FLOAT, reaction_ms);
    add_value(values::BOOL, camera_snap);
    add_value(values::FLOAT, snap_hold_ms);
    add_value(values::BOOL, host_force_parry);

    // Resolve IL2CPP bindings once.
    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("GameEnumExtensions"))
        fn_get_parry_type = pClass->Get<IM>("GetParryTypeFromAttack")->Cast<uint8_t, int, bool>();
    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkHub")) {
        fn_get_my_client    = pClass->Get<IM>("get_MyClientObject")->Cast<MirrorClientObject*>();
        fn_get_my_client_id = pClass->Get<IM>("get_MyClientId") ->Cast<uint32_t>();
    }
    if (auto pClass = I::Get("Mirror.dll")->Get("NetworkReaderExtensions", "Mirror")) {
        fn_read_packed_i32 = pClass->Get<IM>("ReadPackedInt32")->Cast<int, void*>();
        if (auto pClass = I::Get("Mirror.dll")->Get("NetworkReader", "Mirror"))
            fn_read_byte = pClass->Get<IM>("ReadByte")->Cast<uint8_t, void*>();
    }

    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkedPlayerNetwork"))
        if (auto f = pClass->Get<IF>("<DataManager>k__BackingField"))
            off_data_manager = f->offset;
    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("RemotePlayerDataManager"))
        if (auto f = pClass->Get<IF>("<LocalRelativeProperties>k__BackingField"))
            off_rpc = f->offset;
    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("LocalPlayerRelativeProperties")) {
        if (auto f = pClass->Get<IF>("<DirectionToLocalPlayer>k__BackingField"))  off_rpc_dir      = f->offset;
        if (auto f = pClass->Get<IF>("<DistanceToLocalPlayer>k__BackingField"))   off_rpc_distance = f->offset;
        if (auto f = pClass->Get<IF>("<IsVisibleToLocalPlayer>k__BackingField"))  off_rpc_visible  = f->offset;
        if (auto f = pClass->Get<IF>("<IsValid>k__BackingField"))                 off_rpc_valid    = f->offset;
    }

    // Install hooks
    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("MirrorNetworkedPlayerNetwork")) {
        if (auto m = pClass->Get<IM>("InvokeUserCode_RpcTriggerFireWeapon"); m && m->function) {
            fire_weapon_hook.InitHook(m->function, reinterpret_cast<void*>(&detour_invoke));
            fire_weapon_hook.SetHook();
            API_LOG("hooked InvokeUserCode_RpcTriggerFireWeapon @ " << m->function);
        }
    }

    if (auto pClass = I::Get("Assembly-CSharp.dll")->Get("PvpUtils")) {
        if (auto m = pClass->Get<IM>("CheckForDefensiveParry"); m && m->function) {
            check_def_parry_hook.InitHook(m->function, reinterpret_cast<void*>(&detour_check_def_parry));
            check_def_parry_hook.SetHook();
            API_LOG("hooked PvpUtils.CheckForDefensiveParry @ " << m->function << " (server-side: active only when you're host)");
        }

        if (auto m = pClass->Get<IM>("IsViewAngleParrying"); m && m->function) {
            view_angle_hook.InitHook(m->function, reinterpret_cast<void*>(&detour_view_angle));
            view_angle_hook.SetHook();
            API_LOG("hooked PvpUtils.IsViewAngleParrying @ " << m->function << " (server-side: active only when you're host)");
        }
    }
}