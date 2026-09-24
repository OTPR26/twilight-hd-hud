#pragma once

#include "hud_scaling.hpp"

#include "mods/api.h"
#include "mods/svc/config.h"

namespace twilight_hd_hud {

enum class ButtonLayout : int {
    Nintendo = 0,
    Xbox = 1,
    Universal = 2,
    PlayStation = 3,
    BayxFlipped = 4,
    NintendoBotw = 5,
    XboxBotw = 6,
    UniversalBotw = 7,
    BayxFlippedBotw = 8,
    PlayStationSwapped = 9,
    PlayStationFlipped = 10,
};

enum class ButtonStyle : int {
    Silver = 0,
    BlackPro = 1,
    Transparent = 2,
    PlayStationColors = 3,
};

constexpr bool uses_dark_buttons(ButtonStyle style) {
    return style == ButtonStyle::BlackPro || style == ButtonStyle::PlayStationColors;
}

constexpr bool is_playstation_layout(ButtonLayout layout) {
    return layout == ButtonLayout::PlayStation || layout == ButtonLayout::PlayStationSwapped ||
        layout == ButtonLayout::PlayStationFlipped;
}

constexpr bool is_botw_layout(ButtonLayout layout) {
    return layout == ButtonLayout::NintendoBotw || layout == ButtonLayout::XboxBotw ||
        layout == ButtonLayout::UniversalBotw || layout == ButtonLayout::BayxFlippedBotw;
}

constexpr bool is_universal_layout(ButtonLayout layout) {
    return layout == ButtonLayout::Universal || layout == ButtonLayout::UniversalBotw;
}

enum class TextFont : int {
    Original = 0,
    ZenKakuGothicNew = 1,
    MPlus2 = 2,
    FiraSans = 3,
};

enum class ControllerCompatibility : int {
    FollowDusklight = 0,
    FixedTphd = 1,
};

enum class HudSizeSetting { Overall, ControllerDiamond, Dpad, Hearts, ActionText, DialogueText, Rupees, Minimap };

enum class Feature { ThirdItemSlot, CollectionScreen, DpadShortcuts };
bool feature_enabled(Feature feature);
ConfigVarHandle feature_config_var(Feature feature);

int64_t hud_size_percent(HudSizeSetting setting);
int64_t displayed_hud_size_percent(HudSizeSetting setting);
bool hud_size_locked(HudSizeSetting setting);
void set_hud_size_percent(HudSizeSetting setting, int64_t percent);

ModResult register_config(ModError* error);
ButtonLayout button_layout();
ButtonStyle button_style();
ControllerCompatibility controller_compatibility();
float hud_scale();
HudScales hud_scales();
TextFont text_font();
bool item_bank_enabled();
bool swap_menu_buttons();

ConfigVarHandle button_layout_config_var();
ConfigVarHandle button_style_config_var();
ConfigVarHandle controller_compatibility_config_var();
ConfigVarHandle text_font_config_var();
ConfigVarHandle items_screen_config_var();
ConfigVarHandle swap_menu_buttons_config_var();

}  // namespace twilight_hd_hud
