#pragma once
#include <base/features/modules/abstract_module.h>
#include <base/features/events/events.h>

class auto_parry : public instance_module<auto_parry> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	auto_parry();

	// Filter: which attack types to auto-parry
	DEFINE_BOOL_VALUE (parry_sword,     "Parry sword",           "Lunge / throw / spin",                    true);
	DEFINE_BOOL_VALUE (parry_apg,       "Parry APG",             "Single or double APG shot",               true);
	DEFINE_BOOL_VALUE (parry_spear,     "Parry spear impale",    "Single or double impale",                 true);

	// Filter: whether to act
	DEFINE_FLOAT_VALUE(max_range_m,     "Max range (m)",         "Ignore far attackers",                     60.f, 5.f, 200.f);
	DEFINE_BOOL_VALUE (require_visible, "Require line of sight", "Skip if attacker can't see you",          true);
	DEFINE_FLOAT_VALUE(reaction_ms,     "Reaction delay (ms)",   "Fake human reaction time",                80.f, 0.f, 300.f);

	// Client-side: snap local camera to face attacker so server's IsViewAngleParrying passes
	DEFINE_BOOL_VALUE (camera_snap,     "Camera snap (client)",  "Snap camera to attacker direction",       true);
	DEFINE_FLOAT_VALUE(snap_hold_ms,    "Snap hold (ms)",        "Duration to hold forced camera forward",  120.f, 0.f, 500.f);

	// Host-side: force server parry check to return true (only works when you're the host)
	DEFINE_BOOL_VALUE (host_force_parry,"Host force parry",      "If you're host, force parry for yourself",true);

	// Parry state management (default 600ms matches game's native parry duration)
	DEFINE_FLOAT_VALUE(parry_duration_ms,"Parry duration (ms)",  "How long to hold parry state",           600.f, 100.f, 1500.f);
};