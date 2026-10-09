#include "config.hpp"
#include "service_imports.hpp"
#include "update_service.hpp"

#include "mods/service.hpp"
#include "mods/svc/host.h"
#include "mods/svc/ui.h"

#include <array>
#include <string>

namespace twilight_hd_hud {
namespace {

UiWindowHandle s_settingsWindow = 0;
UiMenuTabHandle s_menuTab = 0;
UiElementHandle s_standardStyle = 0;
UiElementHandle s_universalStyle = 0;
UiElementHandle s_playStationStyle = 0;

bool standard_style_disabled(ModContext*, void*) {
    return is_universal_layout(button_layout()) || is_playstation_layout(button_layout());
}

bool playstation_style_disabled(ModContext*, void*) {
    return !is_playstation_layout(button_layout());
}

void get_playstation_style(ModContext*, void*, UiControlValue* value) {
    const auto style = button_style();
    value->int_value = style == ButtonStyle::PlayStationColors ? 2 : static_cast<int64_t>(style);
}

void set_playstation_style(ModContext* ctx, void*, const UiControlValue* value) {
    if (value->int_value < 0 || value->int_value > 2) return;
    svc_config->set_int(ctx, button_style_config_var(), value->int_value == 2 ?
        static_cast<int64_t>(ButtonStyle::PlayStationColors) : value->int_value);
}

bool universal_style_disabled(ModContext*, void*) {
    return !is_universal_layout(button_layout());
}

ModResult update_hud_tab(ModContext* ctx, void*, ModError*) {
    const bool universal = is_universal_layout(button_layout());
    if (s_standardStyle != 0)
        svc_ui->elem_set_visible(ctx, s_standardStyle, !universal && !is_playstation_layout(button_layout()));
    if (s_universalStyle != 0)
        svc_ui->elem_set_visible(ctx, s_universalStyle, universal);
    if (s_playStationStyle != 0)
        svc_ui->elem_set_visible(ctx, s_playStationStyle, is_playstation_layout(button_layout()));
    return MOD_OK;
}

ModResult add_section(ModContext* ctx, UiElementHandle pane, const char* title) {
    return svc_ui->pane_add_section(ctx, pane, title);
}

ModResult add_text(ModContext* ctx, UiElementHandle pane, const char* text) {
    return svc_ui->pane_add_text(ctx, pane, text, nullptr);
}

ModResult add_button(
    ModContext* ctx, UiElementHandle pane, const char* label, UiPressedFn onPressed) {
    UiControlDesc desc = UI_CONTROL_DESC_INIT;
    desc.kind = UI_CONTROL_BUTTON;
    desc.label = label;
    desc.on_pressed = onPressed;
    return svc_ui->pane_add_control(ctx, pane, &desc, nullptr);
}

ModResult add_toggle(ModContext* ctx, UiElementHandle pane, const char* label,
    ConfigVarHandle var, const char* help = nullptr) {
    UiControlDesc desc = UI_CONTROL_DESC_INIT;
    desc.kind = UI_CONTROL_TOGGLE;
    desc.label = label;
    desc.help_rml = help;
    desc.binding = UI_BINDING_CONFIG_VAR;
    desc.config_var = var;
    return svc_ui->pane_add_control(ctx, pane, &desc, nullptr);
}

ModResult add_select(ModContext* ctx, UiElementHandle pane, const char* label,
    ConfigVarHandle var, const char* const* options, size_t optionCount,
    const char* help = nullptr, UiElementHandle* handle = nullptr,
    UiPredicateFn disabled = nullptr) {
    UiControlDesc desc = UI_CONTROL_DESC_INIT;
    desc.kind = UI_CONTROL_SELECT;
    desc.label = label;
    desc.help_rml = help;
    desc.binding = UI_BINDING_CONFIG_VAR;
    desc.config_var = var;
    desc.options = options;
    desc.option_count = optionCount;
    desc.is_disabled = disabled;
    return svc_ui->pane_add_control(ctx, pane, &desc, handle);
}

// Display order is independent of the saved enum values.
constexpr ButtonLayout kLayoutOrder[] = {ButtonLayout::Nintendo, ButtonLayout::NintendoBotw,
    ButtonLayout::Xbox, ButtonLayout::BayxFlipped, ButtonLayout::XboxBotw,
    ButtonLayout::BayxFlippedBotw,
    ButtonLayout::Universal, ButtonLayout::UniversalBotw, ButtonLayout::PlayStation,
    ButtonLayout::PlayStationSwapped, ButtonLayout::PlayStationFlipped,
    ButtonLayout::PlayStationBotw, ButtonLayout::PlayStationFlippedBotw,
    ButtonLayout::SteamDeck, ButtonLayout::SteamDeckBotw};

void get_layout(ModContext*, void*, UiControlValue* value) {
    value->int_value = 0;
    const auto selected = button_layout();
    for (std::size_t i = 0; i < std::size(kLayoutOrder); ++i) {
        if (kLayoutOrder[i] == selected) {
            value->int_value = static_cast<int64_t>(i);
            return;
        }
    }
}

void set_layout(ModContext* ctx, void*, const UiControlValue* value) {
    if (value->int_value >= 0 &&
        value->int_value < static_cast<int64_t>(std::size(kLayoutOrder))) {
        svc_config->set_int(ctx, button_layout_config_var(),
            static_cast<int64_t>(kLayoutOrder[value->int_value]));
        if (!is_universal_layout(button_layout())) {
            int64_t style = 0;
            svc_config->get_int(ctx, button_style_config_var(), &style);
            if (style == static_cast<int64_t>(ButtonStyle::Transparent))
                svc_config->set_int(ctx, button_style_config_var(),
                    static_cast<int64_t>(ButtonStyle::Silver));
        }
        if (!is_playstation_layout(button_layout())) {
            int64_t style = 0;
            svc_config->get_int(ctx, button_style_config_var(), &style);
            if (style == static_cast<int64_t>(ButtonStyle::PlayStationColors))
                svc_config->set_int(ctx, button_style_config_var(),
                    static_cast<int64_t>(ButtonStyle::BlackPro));
        }
        update_hud_tab(ctx, nullptr, nullptr);
    }
}

ModResult build_hud_tab(
    ModContext* ctx, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    if (add_section(ctx, left, "Twilight HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_text(ctx, left, "Uses a TPHD-inspired HUD with matching fonts and visual styling.")
        != MOD_OK) return MOD_ERROR;
    static constexpr const char* kButtonLayouts[] = {
        "ABXY",
        "ABXY (BOTW Style)",
        "BAYX",
        "BAYX Flipped",
        "BAYX (BOTW Style)",
        "BAYX Flipped (BOTW Style)",
        "Universal",
        "Universal (BOTW Style)",
        "PlayStation",
        "PlayStation (Cross Action)",
        "PlayStation (Flipped)",
        "PlayStation (BOTW Style)",
        "PlayStation Flipped (BOTW Style)",
        "Steam Deck",
        "Steam Deck (BOTW Style)",
    };
    UiControlDesc layout = UI_CONTROL_DESC_INIT;
    layout.kind = UI_CONTROL_SELECT;
    layout.label = "Button Layout";
    layout.help_rml = "Visual presets only; configure controller bindings in Dusklight. "
        "BOTW Style places Attack on West, Action on South, and items on North/East. "
        "Steam Deck uses flipped BAYX face buttons with L1/R1 and L2/R2. "
        "DualSense uses PlayStation symbols. Universal leaves face buttons blank.";
    layout.options = kButtonLayouts;
    layout.option_count = std::size(kButtonLayouts);
    layout.get = get_layout;
    layout.set = set_layout;
    if (svc_ui->pane_add_control(ctx, left, &layout, nullptr) != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kButtonStyles[] = {
        "Silver",
        "Black Pro",
    };
    if (add_select(ctx, left, "Button Style", button_style_config_var(),
            kButtonStyles, std::size(kButtonStyles),
            "Silver uses the Twilight Princess HD-style prompts. Black Pro uses dark graphite "
            "buttons with light lettering.", &s_standardStyle, standard_style_disabled)
        != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kUniversalStyles[] = {"Silver", "Black Pro", "Transparent"};
    if (add_select(ctx, left, "Button Style", button_style_config_var(),
            kUniversalStyles, std::size(kUniversalStyles),
            "Silver uses blank silver buttons. Transparent preserves the original "
            "Universal backgrounds. Black Pro uses blank dark buttons.", &s_universalStyle,
            universal_style_disabled)
        != MOD_OK) return MOD_ERROR;
    static constexpr const char* kPlayStationStyles[] = {"Silver", "Black Pro", "PlayStation Colors"};
    UiControlDesc psStyle = UI_CONTROL_DESC_INIT;
    psStyle.kind = UI_CONTROL_SELECT;
    psStyle.label = "Button Style";
    psStyle.help_rml = "PlayStation Colors uses black buttons with colored face-button symbols.";
    psStyle.options = kPlayStationStyles;
    psStyle.option_count = std::size(kPlayStationStyles);
    psStyle.get = get_playstation_style;
    psStyle.set = set_playstation_style;
    psStyle.is_disabled = playstation_style_disabled;
    if (svc_ui->pane_add_control(ctx, left, &psStyle, &s_playStationStyle) != MOD_OK)
        return MOD_ERROR;
    update_hud_tab(ctx, nullptr, nullptr);
    static constexpr const char* kControllerCompatibility[] = {
        "Follow Dusklight Bindings",
        "TPHD Fixed Bindings",
    };
    if (add_select(ctx, left, "Shoulder & D-Pad Behavior",
            controller_compatibility_config_var(), kControllerCompatibility,
            std::size(kControllerCompatibility),
            "Follow uses your Midna binding. Fixed puts Midna on L.<br/>"
            "Face-button bindings are always configured in Dusklight.")
        != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kTextFonts[] = {
        "Original",
        "Zen Kaku Gothic New",
        "M PLUS 2",
        "Dusklight - Fira Sans",
        "Alegreya Sans Medium",
    };
    if (add_select(ctx, left, "Text Font (restart required)", text_font_config_var(),
            kTextFonts, std::size(kTextFonts),
            "Choose the in-game text font. Restart Dusklight to apply changes.") != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kItemsScreens[] = {"TPHD Bank", "Original Wheel"};
    if (add_select(ctx, left, "Items Screen", items_screen_config_var(),
            kItemsScreens, std::size(kItemsScreens),
            "Choose a fixed item bank or the original wheel. Close and reopen Items to apply.") != MOD_OK)
        return MOD_ERROR;
    UiControlDesc swap = UI_CONTROL_DESC_INIT;
    swap.kind = UI_CONTROL_TOGGLE;
    swap.label = "TPHD Items / Collection Buttons";
    swap.help_rml = "On: D-Pad Down opens Collection/Save; Start / + opens Items.<br/>"
        "Off: D-Pad Down opens Items; Start / + opens Collection/Save.";
    swap.binding = UI_BINDING_CONFIG_VAR;
    swap.config_var = swap_menu_buttons_config_var();
    if (svc_ui->pane_add_control(ctx, left, &swap, nullptr) != MOD_OK) return MOD_ERROR;
    if (add_toggle(ctx, left, "Wolf Icons on Touch Buttons", wolf_touch_icons_config_var(),
            "Show Sense, Dig, and Attack on touch buttons. Hide their HUD icons while touch controls are active.") != MOD_OK)
        return MOD_ERROR;
    if (dual_screen_available()) {
        static constexpr const char* kTopButtons[] = {"A + B", "Full Diamond + R"};
        if (add_section(ctx, left, "Dual Screen") != MOD_OK ||
            add_select(ctx, left, "Top Screen Buttons", dual_screen_buttons_config_var(),
                kTopButtons, std::size(kTopButtons),
                "Choose the buttons shown on the top screen. Bottom-screen controls stay available.") != MOD_OK ||
            add_toggle(ctx, left, "Show L / Midna on Top Screen", dual_screen_midna_config_var(),
                "Show Midna's L prompt on the top screen.") != MOD_OK ||
            add_toggle(ctx, left, "Show D-Pad on Top Screen", dual_screen_dpad_config_var(),
                "Show or hide the D-Pad icons. All D-Pad controls keep working.") != MOD_OK)
            return MOD_ERROR;
    }
    if (add_section(ctx, left, "Optional Features (restart required)") != MOD_OK)
        return MOD_ERROR;
    if (add_toggle(ctx, left, "Third Item Slot (Z/R)",
            feature_config_var(Feature::ThirdItemSlot),
            "Adds the third item slot and its shoulder-button controls. Turn off to leave "
            "those controls to Dusklight or another mod. Restart Dusklight to apply.") != MOD_OK)
        return MOD_ERROR;
    if (add_toggle(ctx, left, "TPHD Collection Screen",
            feature_config_var(Feature::CollectionScreen),
            "Styles and rearranges the main Collection screen. Turn off to leave its layout "
            "and navigation unchanged. Journals remain enabled. Restart Dusklight to apply.") != MOD_OK)
        return MOD_ERROR;
    if (add_toggle(ctx, left, "D-Pad Shortcuts",
        feature_config_var(Feature::DpadShortcuts),
        "Enable TPHD map and menu shortcuts. Off uses native D-Pad controls. Restart to apply.") != MOD_OK) return MOD_ERROR;
    return add_toggle(ctx, left, "Map / Minimap on Up", combined_map_config_var(),
        "Up cycles minimap, full map, then hidden. Leaves Left/Right free. Midna takes priority.");
}

HudSizeSetting size_setting(void* data) {
    return *static_cast<HudSizeSetting*>(data);
}

bool size_disabled(ModContext*, void* data) { return hud_size_locked(size_setting(data)); }
bool size_modified(ModContext*, void* data) {
    return displayed_hud_size_percent(size_setting(data)) != 100;
}
void get_size(ModContext*, void* data, UiControlValue* value) {
    value->int_value = displayed_hud_size_percent(size_setting(data));
}
void set_size(ModContext*, void* data, const UiControlValue* value) {
    set_hud_size_percent(size_setting(data), value->int_value);
}
void reset_size(ModContext*, void* data) { set_hud_size_percent(size_setting(data), 100); }

constexpr const char* kSizingGuide =
    "• Enter 50–125% or use Left/Right. Changes apply live.<br/>"
    "• Set Overall to 100% to edit individual icons. Text sizes stay independent.<br/>"
    "To hide the HUD, enable Minimal HUD in Dusklight's Gameplay settings.";

ModResult build_hud_sizing_tab(
    ModContext* ctx, UiWindowHandle, UiElementHandle left, UiElementHandle right, void*, ModError*) {
    if (add_section(ctx, left, "HUD Sizing") != MOD_OK ||
        svc_ui->pane_add_rml(ctx, right, kSizingGuide, nullptr) != MOD_OK) return MOD_ERROR;

    static HudSizeSetting settings[] = {HudSizeSetting::Overall,
        HudSizeSetting::ControllerDiamond, HudSizeSetting::Dpad, HudSizeSetting::Hearts,
        HudSizeSetting::ActionText, HudSizeSetting::DialogueText,
        HudSizeSetting::Rupees, HudSizeSetting::Minimap};
    constexpr const char* labels[] = {
        "Overall HUD Size", "Controller Diamond Size", "D-Pad Size", "Hearts Size",
        "Action Text Scale",
        "Dialogue Text Scale",
        "Rupee Scale", "Minimap Scale",
    };
    constexpr const char* resetLabels[] = {
        "Reset Overall to 100%", "Reset Diamond to 100%",
        "Reset D-Pad to 100%", "Reset Hearts to 100%",
        "Reset Action Text to 100%",
        "Reset Dialogue Text to 100%",
        "Reset Rupees to 100%", "Reset Minimap to 100%",
    };
    static const std::array<std::string, 8> help = [] {
        constexpr const char* details[] = {
            "Overall sets all icon sizes. Returning to 100% restores your saved sizes.",
            "Sizes buttons, items, ammo, and Wolf Link icons.",
            "Sizes the D-Pad and map icon.",
            "Sizes gameplay hearts; save-menu hearts keep their own size.",
            "Sizes action text. 125% restores the previous size.",
            "Sizes dialogue text.",
            "Sizes the rupee icon and counter.",
            "Sizes the minimap.",
        };
        std::array<std::string, 8> result;
        for (std::size_t i = 0; i < result.size(); ++i)
            result[i] = std::string(details[i]) + "<br/><br/>" + kSizingGuide;
        return result;
    }();
    for (std::size_t i = 0; i < std::size(settings); ++i) {
        UiControlDesc number = UI_CONTROL_DESC_INIT;
        number.kind = UI_CONTROL_NUMBER;
        number.label = labels[i];
        number.help_rml = help[i].c_str();
        number.get = get_size;
        number.set = set_size;
        number.is_disabled = size_disabled;
        number.is_modified = size_modified;
        number.user_data = &settings[i];
        number.min = 50;
        number.max = 125;
        number.step = 1;
        number.suffix = "%";
        if (svc_ui->pane_add_control(ctx, left, &number, nullptr) != MOD_OK) return MOD_ERROR;

        UiControlDesc reset = UI_CONTROL_DESC_INIT;
        reset.kind = UI_CONTROL_BUTTON;
        reset.label = resetLabels[i];
        reset.help_rml = kSizingGuide;
        reset.on_pressed = reset_size;
        reset.is_disabled = size_disabled;
        reset.user_data = &settings[i];
        if (svc_ui->pane_add_control(ctx, left, &reset, nullptr) != MOD_OK) return MOD_ERROR;
    }
    return MOD_OK;
}

void settings_closed(ModContext*, UiWindowHandle, void*) {
    s_settingsWindow = 0;
}

void open_settings(ModContext* ctx, void*) {
    if (s_settingsWindow != 0) {
        return;
    }

    std::array<UiTabDesc, 2> tabs{};
    tabs[0] = UI_TAB_DESC_INIT;
    tabs[0].title = "HUD";
    tabs[0].build = build_hud_tab;
    tabs[0].update = update_hud_tab;
    tabs[1] = UI_TAB_DESC_INIT;
    tabs[1].title = "HUD Sizing";
    tabs[1].build = build_hud_sizing_tab;

    UiWindowDesc desc = UI_WINDOW_DESC_INIT;
    desc.tabs = tabs.data();
    desc.tab_count = tabs.size();
    desc.on_closed = settings_closed;
    svc_ui->window_push(ctx, &desc, &s_settingsWindow);
}

ModResult build_mod_panel(ModContext* ctx, UiElementHandle panel, void*, ModError*) {
    if (add_section(ctx, panel, "Twilight HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_button(ctx, panel, "Open Twilight HD Settings", open_settings) != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_toggle(ctx, panel, "Auto Update Checks", check_for_updates_config_var(),
            "Check GitHub releases when the mod starts. Installation requires confirmation.") != MOD_OK)
        return MOD_ERROR;
    UiControlDesc update = UI_CONTROL_DESC_INIT;
    update.kind = UI_CONTROL_BUTTON;
    update.label = "Check Now";
    update.on_pressed = request_update_check;
    update.user_data = reinterpret_cast<void*>(1);
    update.is_disabled = update_service_busy;
    return svc_ui->pane_add_control(ctx, panel, &update, nullptr);
}

}  // namespace

ModResult register_ui(ModError* error) {
    UiModsPanelDesc panel = UI_MODS_PANEL_DESC_INIT;
    panel.build = build_mod_panel;
    ModResult result = svc_ui->register_mods_panel(mod_ctx, &panel);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "failed to register Twilight HD panel");
    }

    UiMenuTabDesc tab = UI_MENU_TAB_DESC_INIT;
    tab.label = "Twilight HD";
    tab.on_selected = open_settings;
    result = svc_ui->register_menu_tab(mod_ctx, &tab, &s_menuTab);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "failed to register Twilight HD menu tab");
    }
    return MOD_OK;
}

}  // namespace twilight_hd_hud
