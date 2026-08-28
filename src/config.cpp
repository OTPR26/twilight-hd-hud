#include "config.hpp"
#include "service_imports.hpp"

#include "mods/service.hpp"
#include "mods/svc/config.h"

#include <array>

namespace twilight_hd_hud {
namespace {

ConfigVarHandle s_buttonLayout = 0;
ConfigVarHandle s_buttonStyle = 0;
ConfigVarHandle s_controllerCompatibility = 0;
ConfigVarHandle s_hudScale = 0;
ConfigVarHandle s_controllerDiamondSize = 0;
ConfigVarHandle s_dpadSize = 0;
ConfigVarHandle s_heartsSize = 0;
ConfigVarHandle s_textFont = 0;
ConfigVarHandle s_itemsScreen = 0;
ConfigVarHandle s_swapMenuButtons = 0;

ModResult register_menu_swap() {
    ConfigVarDesc desc = CONFIG_VAR_DESC_INIT;
    desc.name = "swap-menu-buttons";
    desc.type = CONFIG_VAR_BOOL;
    desc.default_bool = true;
    return svc_config->register_var(mod_ctx, &desc, &s_swapMenuButtons);
}

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

ConfigVarHandle size_handle(HudSizeSetting setting) {
    switch (setting) {
    case HudSizeSetting::Overall: return s_hudScale;
    case HudSizeSetting::ControllerDiamond: return s_controllerDiamondSize;
    case HudSizeSetting::Dpad: return s_dpadSize;
    case HudSizeSetting::Hearts: return s_heartsSize;
    }
    return 0;
}

ModResult register_hud_sizes() {
    // New keys keep old dropdown indices recoverable for rollback. Migrate
    // once; later launches must never overwrite saved percentage choices.
    constexpr const char* legacyNames[] = {
        "hud-size", "controller-diamond-size", "dpad-size", "hearts-size",
    };
    constexpr const char* percentNames[] = {
        "hud-percent", "controller-diamond-percent", "dpad-percent", "hearts-percent",
    };
    ConfigVarHandle* handles[] = {
        &s_hudScale, &s_controllerDiamondSize, &s_dpadSize, &s_heartsSize,
    };
    std::array<ConfigVarHandle, 4> legacy{};
    ConfigVarHandle migrated = 0;
    if (register_int("hud-percent-migrated", 0, migrated) != MOD_OK) return MOD_ERROR;
    for (std::size_t i = 0; i < legacy.size(); ++i) {
        if (register_int(legacyNames[i], 1, legacy[i]) != MOD_OK ||
            register_int(percentNames[i], 100, *handles[i]) != MOD_OK) return MOD_ERROR;
    }
    if (get_int(migrated, 0) == 0) {
        for (std::size_t i = 0; i < legacy.size(); ++i) {
            if (svc_config->set_int(mod_ctx, *handles[i],
                    legacy_hud_percent(get_int(legacy[i], 1))) != MOD_OK) return MOD_ERROR;
        }
        return svc_config->set_int(mod_ctx, migrated, 1);
    }
    return MOD_OK;
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
        register_hud_sizes() != MOD_OK ||
        register_int("text-font", 0, s_textFont) != MOD_OK ||
        register_int("items-screen", 0, s_itemsScreen) != MOD_OK ||
        register_menu_swap() != MOD_OK)
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
        value > static_cast<int64_t>(ButtonLayout::BayxFlipped))
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
    return hud_size_multiplier(hud_size_percent(HudSizeSetting::Overall));
}

HudScales hud_scales() {
    return compose_hud_scales(hud_size_percent(HudSizeSetting::Overall),
        hud_size_percent(HudSizeSetting::ControllerDiamond),
        hud_size_percent(HudSizeSetting::Dpad), hud_size_percent(HudSizeSetting::Hearts));
}

int64_t hud_size_percent(HudSizeSetting setting) {
    return clamp_hud_percent(get_int(size_handle(setting), 100));
}

bool hud_size_locked(HudSizeSetting setting) {
    return setting != HudSizeSetting::Overall &&
        hud_size_percent(HudSizeSetting::Overall) != 100;
}

int64_t displayed_hud_size_percent(HudSizeSetting setting) {
    return hud_size_locked(setting) ? hud_size_percent(HudSizeSetting::Overall) :
        hud_size_percent(setting);
}

void set_hud_size_percent(HudSizeSetting setting, int64_t percent) {
    if (const auto handle = size_handle(setting); handle != 0 && !hud_size_locked(setting)) {
        svc_config->set_int(mod_ctx, handle, clamp_hud_percent(percent));
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

TextFont text_font() {
    const auto value = get_int(s_textFont, 0);
    if (value < 0 || value > static_cast<int64_t>(TextFont::FiraSans)) {
        return TextFont::Original;
    }
    return static_cast<TextFont>(value);
}

ConfigVarHandle text_font_config_var() {
    return s_textFont;
}

bool item_bank_enabled() {
    return get_int(s_itemsScreen, 0) == 0;
}

ConfigVarHandle items_screen_config_var() {
    return s_itemsScreen;
}

bool swap_menu_buttons() {
    bool value = true;
    if (s_swapMenuButtons != 0) {
        svc_config->get_bool(mod_ctx, s_swapMenuButtons, &value);
    }
    return value;
}

ConfigVarHandle swap_menu_buttons_config_var() {
    return s_swapMenuButtons;
}

}  // namespace twilight_hd_hud
