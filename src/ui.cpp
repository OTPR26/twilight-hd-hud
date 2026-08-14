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

ModResult build_hud_tab(
    ModContext* ctx, UiWindowHandle, UiElementHandle left, UiElementHandle, void*, ModError*) {
    if (add_section(ctx, left, "Twilight Princess HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_text(ctx, left,
            "Uses the Twilight Princess HD HUD arrangement and smaller minimap presentation. "
            "Disable the mod from Dusklight's Mods screen to restore the standard HUD.")
        != MOD_OK) return MOD_ERROR;
    static constexpr const char* kButtonLayouts[] = {
        "ABXY",
        "BAYX",
        "Universal",
    };
    if (add_select(ctx, left, "Button Layout", button_layout_config_var(),
            kButtonLayouts, std::size(kButtonLayouts),
            "Changes the buttons to match ABXY or BAYX controllers. Universal leaves buttons "
            "blank.")
        != MOD_OK)
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
            "Follow Dusklight Bindings respects the active controller profile and its Call Midna "
            "action. TPHD Fixed Bindings instead reserves D-Pad Up for Midna. In both modes, the "
            "control mapped to logical R operates the third item and related menu actions.")
        != MOD_OK)
    {
        return MOD_ERROR;
    }
    static constexpr const char* kHudSizes[] = {
        "75%",
        "100%",
        "125%",
    };
    if (add_select(ctx, left, "HUD Size", hud_scale_config_var(), kHudSizes,
            std::size(kHudSizes), "Scales the complete gameplay HUD. The current size is 100%.")
        != MOD_OK)
    {
        return MOD_ERROR;
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

    std::array<UiTabDesc, 1> tabs{};
    tabs[0] = UI_TAB_DESC_INIT;
    tabs[0].title = "HUD";
    tabs[0].build = build_hud_tab;

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
