#include "config.hpp"
#include "service_imports.hpp"

#include "mods/service.hpp"
#include "mods/svc/config.h"

namespace twilight_hd_hud {
namespace {

ConfigVarHandle s_buttonLayout = 0;
ConfigVarHandle s_buttonStyle = 0;
ConfigVarHandle s_controllerCompatibility = 0;
ConfigVarHandle s_hudScale = 0;

ModResult register_int(const char* name, int64_t defaultValue, ConfigVarHandle& handle) {
    ConfigVarDesc desc = CONFIG_VAR_DESC_INIT;
    desc.name = name;
    desc.type = CONFIG_VAR_INT;
    desc.default_int = defaultValue;
    return svc_config->register_var(mod_ctx, &desc, &handle);
}

int64_t get_int(ConfigVarHandle handle, int64_t fallback) {
    int64_t value = fallback;
    if (handle != 0) {
        svc_config->get_int(mod_ctx, handle, &value);
    }
    return value;
}

}  // namespace

ModResult register_config(ModError* error) {
    if (register_int("button-layout", static_cast<int64_t>(ButtonLayout::Nintendo),
            s_buttonLayout) != MOD_OK ||
        register_int("button-style", static_cast<int64_t>(ButtonStyle::Silver),
            s_buttonStyle) != MOD_OK ||
        register_int("controller-compatibility",
            static_cast<int64_t>(ControllerCompatibility::FollowDusklight),
            s_controllerCompatibility) != MOD_OK ||
        register_int("hud-size", 1, s_hudScale) != MOD_OK)
    {
        return mods::set_error(
            error, MOD_ERROR, "failed to register Twilight HD HUD settings");
    }
    return MOD_OK;
}

ControllerCompatibility controller_compatibility() {
    const int64_t value = get_int(s_controllerCompatibility,
        static_cast<int64_t>(ControllerCompatibility::FollowDusklight));
    if (value < static_cast<int64_t>(ControllerCompatibility::FollowDusklight) ||
        value > static_cast<int64_t>(ControllerCompatibility::FixedTphd))
    {
        return ControllerCompatibility::FollowDusklight;
    }
    return static_cast<ControllerCompatibility>(value);
}

ButtonLayout button_layout() {
    const int64_t value = get_int(s_buttonLayout, static_cast<int64_t>(ButtonLayout::Nintendo));
    if (value < static_cast<int64_t>(ButtonLayout::Nintendo) ||
        value > static_cast<int64_t>(ButtonLayout::PlayStation))
    {
        return ButtonLayout::Nintendo;
    }
    return static_cast<ButtonLayout>(value);
}

ButtonStyle button_style() {
    const int64_t value = get_int(s_buttonStyle, static_cast<int64_t>(ButtonStyle::Silver));
    if (value < static_cast<int64_t>(ButtonStyle::Silver) ||
        value > static_cast<int64_t>(ButtonStyle::BlackPro))
    {
        return ButtonStyle::Silver;
    }
    return static_cast<ButtonStyle>(value);
}

float hud_scale() {
    switch (get_int(s_hudScale, 1)) {
    case 0:
        return 0.75f;
    case 2:
        return 1.25f;
    default:
        return 1.0f;
    }
}

ConfigVarHandle button_layout_config_var() {
    return s_buttonLayout;
}

ConfigVarHandle button_style_config_var() {
    return s_buttonStyle;
}

ConfigVarHandle controller_compatibility_config_var() {
    return s_controllerCompatibility;
}

ConfigVarHandle hud_scale_config_var() {
    return s_hudScale;
}

}  // namespace twilight_hd_hud
