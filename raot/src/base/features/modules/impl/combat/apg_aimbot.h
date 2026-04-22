#pragma once
#include <base/features/modules/abstract_module.h>

class apg_aimbot : public instance_module<apg_aimbot> {
public:
	void on_disable();
	void on_enable();
	void on_update();
	apg_aimbot();

	// Trigger
	DEFINE_BOOL_VALUE (always_on,         "Always on",           "Aim continuously (overrides hold key)",     false);
	DEFINE_BOOL_VALUE (hold_rmb,          "Hold RMB to aim",     "Only aim while right mouse button is held", true);

	// Target filter
	DEFINE_FLOAT_VALUE(fov_deg,           "FOV cone (deg)",      "Only target enemies within this half-cone", 30.f,  1.f,  180.f);
	DEFINE_FLOAT_VALUE(max_range_m,       "Max range (m)",       "Ignore far enemies",                        150.f, 5.f,  500.f);
	DEFINE_BOOL_VALUE (aim_head,          "Aim head",            "Target head bone (else player origin)",     true);

	// Prediction
	DEFINE_BOOL_VALUE (prediction,        "Prediction",          "Lead target by measured velocity",          true);
	DEFINE_FLOAT_VALUE(projectile_speed,  "Projectile speed",    "APG bullet speed (m/s)",                    200.f, 50.f, 2000.f);
	DEFINE_FLOAT_VALUE(history_window_ms, "Vel sample (ms)",     "Velocity estimation window",                150.f, 50.f, 500.f);
	DEFINE_FLOAT_VALUE(max_lead_m,        "Max lead (m)",        "Clamp prediction to avoid wild leads",      40.f,  0.f,  200.f);

	// Aim application
	DEFINE_BOOL_VALUE (smooth,            "Smooth aim",          "Slerp toward target instead of snapping",   true);
	DEFINE_FLOAT_VALUE(smooth_factor,     "Smooth factor",       "Higher = faster snap (1.0 = instant)",      0.35f, 0.05f, 1.f);
};
