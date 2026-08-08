#pragma once

#include "mods/api.h"
#include "mods/svc/config.h"

namespace twilight_hd_hud {

enum class ButtonLayout : int {
    Nintendo = 0,
    Xbox = 1,
    Universal = 2,
};

ModResult register_config(ModError* error);
ButtonLayout button_layout();
float hud_scale();

ConfigVarHandle button_layout_config_var();
ConfigVarHandle hud_scale_config_var();

}  // namespace twilight_hd_hud
