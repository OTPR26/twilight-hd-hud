#include "config.hpp"
#include "service_imports.hpp"

#include "mods/service.hpp"
#include "mods/svc/ui.h"

#include <array>

namespace twilight_hd_hud {
namespace {

UiWindowHandle s_settingsWindow = 0;
UiMenuTabHandle s_menuTab = 0;

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

ModResult add_select(ModContext* ctx, UiElementHandle pane, const char* label,
    ConfigVarHandle var, const char* const* options, size_t optionCount,
    const char* help = nullptr) {
    UiControlDesc desc = UI_CONTROL_DESC_INIT;
    desc.kind = UI_CONTROL_SELECT;
    desc.label = label;
    desc.help_rml = help;
    desc.binding = UI_BINDING_CONFIG_VAR;
    desc.config_var = var;
    desc.options = options;
    desc.option_count = optionCount;
    return svc_ui->pane_add_control(ctx, pane, &desc, nullptr);
}

// Display order is independent of the saved enum values.
constexpr ButtonLayout kLayoutOrder[] = {ButtonLayout::Nintendo, ButtonLayout::Xbox,
    ButtonLayout::BayxFlipped, ButtonLayout::Universal, ButtonLayout::PlayStation};

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
    }
}

ModResult build_hud_tab(
    ModContext* ctx, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    if (add_section(ctx, left, "Twilight Princess HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_text(ctx, left,
            "Uses the Twilight Princess HD HUD arrangement. "
            "Disable the mod from Dusklight's Mods screen to restore the standard HUD.")
        != MOD_OK) return MOD_ERROR;
    static constexpr const char* kButtonLayouts[] = {
        "ABXY",
        "BAYX",
        "BAYX Flipped",
        "Universal",
        "PlayStation",
    };
    UiControlDesc layout = UI_CONTROL_DESC_INIT;
    layout.kind = UI_CONTROL_SELECT;
    layout.label = "Button Layout";
    layout.help_rml = "Changes the button prompts. Universal leaves face buttons blank.";
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
            "buttons with light lettering.")
        != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kControllerCompatibility[] = {
        "Follow Dusklight Bindings",
        "TPHD Fixed Bindings",
    };
    if (add_select(ctx, left, "Controller Compatibility",
            controller_compatibility_config_var(), kControllerCompatibility,
            std::size(kControllerCompatibility),
            "Follow Dusklight Bindings respects the custom bindings for Midna.<br/><br/>"
            "TPHD Fixed Bindings follows TPHD bindings.")
        != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kTextFonts[] = {
        "Original",
        "Zen Kaku Gothic New",
        "M PLUS 2",
        "Dusklight - Fira Sans",
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
    return MOD_OK;
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

ModResult build_hud_sizing_tab(
    ModContext* ctx, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    if (add_section(ctx, left, "HUD Sizing") != MOD_OK ||
        add_text(ctx, left,
            "Enter 50-125%, or adjust left/right in 1% steps. Changes apply live. "
            "Overall overrides all groups unless it is 100%; returning to 100% "
            "restores your individual values. Use Dusklight's Minimal HUD option to hide the HUD.")
            != MOD_OK) return MOD_ERROR;

    static HudSizeSetting settings[] = {HudSizeSetting::Overall,
        HudSizeSetting::ControllerDiamond, HudSizeSetting::Dpad, HudSizeSetting::Hearts};
    constexpr const char* labels[] = {
        "Overall HUD Size", "Controller Diamond Size", "D-Pad Size", "Hearts Size",
    };
    constexpr const char* resetLabels[] = {
        "Reset Overall to 100%", "Reset Diamond to 100%",
        "Reset D-Pad to 100%", "Reset Hearts to 100%",
    };
    constexpr const char* help[] = {
        "Any value other than 100% overrides all groups. Individual controls show that value "
        "and are disabled. Reset Overall to restore your saved individual sizes.",
        "Sizes the face-button/item cluster, ammo, and Wolf Link action icons. "
        "Set Overall to 100% to edit this percentage.",
        "Sizes the D-pad, labels, and map icon. Set Overall to 100% to edit this percentage.",
        "Sizes gameplay hearts, not save-menu hearts. Set Overall to 100% to edit this percentage.",
    };
    for (std::size_t i = 0; i < std::size(settings); ++i) {
        UiControlDesc number = UI_CONTROL_DESC_INIT;
        number.kind = UI_CONTROL_NUMBER;
        number.label = labels[i];
        number.help_rml = help[i];
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
    if (add_section(ctx, panel, "Twilight HD HUD") != MOD_OK) {
        return MOD_ERROR;
    }
    return add_button(ctx, panel, "Open Twilight HD HUD Settings", open_settings);
}

}  // namespace

ModResult register_ui(ModError* error) {
    UiModsPanelDesc panel = UI_MODS_PANEL_DESC_INIT;
    panel.build = build_mod_panel;
    ModResult result = svc_ui->register_mods_panel(mod_ctx, &panel);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "failed to register Twilight HD HUD panel");
    }

    UiMenuTabDesc tab = UI_MENU_TAB_DESC_INIT;
    tab.label = "Twilight HD HUD";
    tab.on_selected = open_settings;
    result = svc_ui->register_menu_tab(mod_ctx, &tab, &s_menuTab);
    if (result != MOD_OK) {
        return mods::set_error(error, result, "failed to register Twilight HD HUD menu tab");
    }
    return MOD_OK;
}

}  // namespace twilight_hd_hud
