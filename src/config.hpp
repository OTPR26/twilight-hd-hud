#pragma once

#include "mods/api.h"
#include "mods/svc/config.h"

namespace twilight_hd_hud {

enum class ButtonLayout : int {
    Nintendo = 0,
    Xbox = 1,
    Universal = 2,
};

enum class ButtonStyle : int {
    Silver = 0,
    BlackPro = 1,
};

enum class ControllerCompatibility : int {
    FollowDusklight = 0,
    FixedTphd = 1,
};

ModResult register_config(ModError* error);
ButtonLayout button_layout();
ButtonStyle button_style();
ControllerCompatibility controller_compatibility();
float hud_scale();

ConfigVarHandle button_layout_config_var();
ConfigVarHandle button_style_config_var();
ConfigVarHandle controller_compatibility_config_var();
ConfigVarHandle hud_scale_config_var();

}  // namespace twilight_hd_hud
