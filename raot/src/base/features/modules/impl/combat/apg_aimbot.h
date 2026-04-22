#pragma once
#include <base/features/modules/abstract_module.h>

class apg_aimbot : public instance_module<apg_aimbot> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	apg_aimbot();

	// Trigger (all gated by game's "ready attack" charge state; buttons select which side activates it)
	DEFINE_BOOL_VALUE (always_on,           "Always on",             "Aim continuously (skips ready-attack gate)", false);
	DEFINE_BOOL_VALUE (hold_rmb,            "Trigger on RMB",        "Aim while charging attack with RMB",         true);
	DEFINE_BOOL_VALUE (hold_lmb,            "Trigger on LMB",        "Aim while charging attack with LMB",         true);

	// Target filter
	DEFINE_FLOAT_VALUE(fov_deg,             "FOV cone (deg)",        "Only target enemies within this half-cone",  30.f,  1.f,  180.f);
	DEFINE_FLOAT_VALUE(max_range_m,         "Max range (m)",         "Ignore far enemies",                         50.f,  5.f,  500.f);
	DEFINE_BOOL_VALUE (aim_head,            "Aim head",              "Target head bone (else player origin)",      true);

	// Prediction
	DEFINE_BOOL_VALUE (prediction,          "Prediction",            "Lead target by measured velocity",           true);
	DEFINE_FLOAT_VALUE(projectile_speed,    "Projectile speed",      "APG bullet speed (m/s)",                     160.f, 20.f, 200.f);
	DEFINE_FLOAT_VALUE(history_window_ms,   "Vel sample (ms)",       "Velocity estimation window",                 150.f, 50.f, 500.f);
	DEFINE_FLOAT_VALUE(max_lead_m,          "Max lead (m)",          "Clamp prediction to avoid wild leads",       40.f,  0.f,  200.f);

	// Aim application
	DEFINE_BOOL_VALUE (smooth,              "Smooth aim",            "Smooth initial lock-on, then track 1:1",     false);
	DEFINE_FLOAT_VALUE(smooth_factor,       "Smooth speed",          "Initial converge speed (higher = faster)",   0.6f,  0.05f, 1.f);
	DEFINE_FLOAT_VALUE(lock_threshold_deg,  "Lock threshold (deg)",  "Switch to 1:1 tracking once within this",    2.0f,  0.1f,  30.f);
};
