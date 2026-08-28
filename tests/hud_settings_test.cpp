#include "config.hpp"
#include "service_imports.hpp"
#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

ModContext* mod_ctx = nullptr;
const ConfigService* svc_config = nullptr;
const UiService* svc_ui = nullptr;
namespace twilight_hd_hud { ModResult register_ui(ModError*); }
using namespace twilight_hd_hud;

static std::map<std::string, int64_t> saved;
static std::vector<std::string> names;
static std::map<std::string, ConfigVarType> types;
static std::vector<UiControlDesc> controls;
static UiMenuTabDesc menuTab{};

static ModResult register_var(ModContext*, const ConfigVarDesc* desc, ConfigVarHandle* handle) {
    assert(desc->type == CONFIG_VAR_INT || desc->type == CONFIG_VAR_BOOL);
    types[desc->name] = desc->type;
    names.emplace_back(desc->name);
    *handle = names.size();
    saved.try_emplace(desc->name, desc->type == CONFIG_VAR_BOOL ?
        desc->default_bool : desc->default_int);
    return MOD_OK;
}
static ModResult get_int(ModContext*, ConfigVarHandle handle, int64_t* value) {
    assert(types.at(names.at(handle - 1)) == CONFIG_VAR_INT);
    *value = saved.at(names.at(handle - 1));
    return MOD_OK;
}
static ModResult set_int(ModContext*, ConfigVarHandle handle, int64_t value) {
    assert(types.at(names.at(handle - 1)) == CONFIG_VAR_INT);
    saved[names.at(handle - 1)] = value;
    return MOD_OK;
}
static ModResult get_bool(ModContext*, ConfigVarHandle handle, bool* value) {
    assert(types.at(names.at(handle - 1)) == CONFIG_VAR_BOOL);
    *value = saved.at(names.at(handle - 1)) != 0;
    return MOD_OK;
}
static ModResult set_bool(ModContext*, ConfigVarHandle handle, bool value) {
    assert(types.at(names.at(handle - 1)) == CONFIG_VAR_BOOL);
    saved[names.at(handle - 1)] = value;
    return MOD_OK;
}
static ModResult section(ModContext*, UiElementHandle pane, const char*) {
    assert(pane == 2); return MOD_OK;
}
static ModResult text(ModContext*, UiElementHandle pane, const char*, UiElementHandle*) {
    assert(pane == 2); return MOD_OK;
}
static ModResult control(ModContext*, UiElementHandle pane, const UiControlDesc* desc, UiElementHandle*) {
    assert(pane == 2); controls.push_back(*desc); return MOD_OK;
}
static ModResult window(ModContext*, const UiWindowDesc* desc, UiWindowHandle* handle) {
    assert(desc->tab_count == 2);
    assert(std::string(desc->tabs[1].title) == "HUD Sizing");
    assert(desc->tabs[0].build(nullptr, 1, 2, 3, nullptr, nullptr) == MOD_OK);
    assert(controls[0].kind == UI_CONTROL_SELECT && controls[0].option_count == 5);
    assert(std::string(controls[0].options[0]) == "ABXY");
    assert(std::string(controls[0].options[1]) == "BAYX");
    assert(std::string(controls[0].options[2]) == "BAYX Flipped");
    assert(std::string(controls[0].options[3]) == "Universal");
    assert(std::string(controls[0].options[4]) == "PlayStation");
    assert(std::string(controls[0].help_rml).find("BAYX") == std::string::npos);
    // Reordering the menu must not reinterpret existing saved choices.
    constexpr int64_t persisted[] = {0, 1, 4, 2, 3};
    const auto& layout = controls[0];
    assert(layout.binding == UI_BINDING_CALLBACKS);
    for (int64_t index = 0; index < 5; ++index) {
        saved["button-layout"] = persisted[index];
        UiControlValue value = UI_CONTROL_VALUE_INIT;
        layout.get(nullptr, nullptr, &value);
        assert(value.int_value == index);
        saved["button-layout"] = 0;
        layout.set(nullptr, nullptr, &value);
        assert(saved.at("button-layout") == persisted[index]);
    }
    for (int64_t invalid : {-1, 5}) {
        UiControlValue value = UI_CONTROL_VALUE_INIT;
        value.int_value = invalid;
        layout.set(nullptr, nullptr, &value);
        assert(saved.at("button-layout") == 3);
    }
    saved["button-layout"] = 0;
    assert(std::string(controls[3].help_rml) ==
        "Choose the in-game text font. Restart Dusklight to apply changes.");
    assert(std::string(controls[4].label) == "Items Screen");
    assert(controls[4].option_count == 2);
    assert(std::string(controls[4].options[0]) == "TPHD Bank");
    assert(std::string(controls[4].options[1]) == "Original Wheel");
    assert(controls.size() == 6);
    const auto& swap = controls[5];
    assert(swap.kind == UI_CONTROL_TOGGLE);
    assert(std::string(swap.label) == "TPD  Items / Collection Buttons");
    assert(swap.binding == UI_BINDING_CONFIG_VAR);
    assert(swap.config_var == swap_menu_buttons_config_var());
    assert(std::string(swap.help_rml) ==
        "On: D-Pad Down opens Collection/Save; Start / + opens Items.<br/>"
        "Off: D-Pad Down opens Items; Start / + opens Collection/Save.");
    assert(!swap_menu_buttons()); // saved Off survives reopening Settings
    for (int64_t screen : {0, 1}) {
        saved["items-screen"] = screen;
        for (bool enabled : {true, false}) {
            assert(svc_config->set_bool(nullptr, swap.config_var, enabled) == MOD_OK);
            assert(swap_menu_buttons() == enabled);
            assert(item_bank_enabled() == (screen == 0));
        }
    }
    saved["items-screen"] = 0;
    controls.clear();
    *handle = 1;
    return desc->tabs[1].build(nullptr, 1, 2, 3, nullptr, nullptr);
}
static int64_t displayed(const UiControlDesc& control) {
    UiControlValue value = UI_CONTROL_VALUE_INIT;
    control.get(nullptr, control.user_data, &value);
    return value.int_value;
}
static void enter(const UiControlDesc& control, int64_t percent) {
    UiControlValue value = UI_CONTROL_VALUE_INIT;
    value.int_value = percent;
    control.set(nullptr, control.user_data, &value);
}

int main() {
    ConfigService config{};
    config.register_var = register_var;
    config.get_int = get_int;
    config.set_int = set_int;
    config.get_bool = get_bool;
    config.set_bool = set_bool;
    svc_config = &config;
    saved = {{"hud-size", 2}, {"controller-diamond-size", 0},
        {"dpad-size", 1}, {"hearts-size", 2}};
    assert(register_config(nullptr) == MOD_OK);
    assert(swap_menu_buttons()); // upgrades preserve the previous mapping by default
    assert(saved.at("action-text-percent") == 100); // independent of old size migration
    assert(saved.at("dialogue-text-percent") == 100);
    assert(saved.at("rupee-percent") == 100);
    assert(saved.at("minimap-percent") == 100);
    assert(set_bool(nullptr, swap_menu_buttons_config_var(), false) == MOD_OK);
    assert(!swap_menu_buttons());
    assert(item_bank_enabled());
    saved["items-screen"] = 1;
    assert(!item_bank_enabled());
    saved["items-screen"] = 19;
    assert(!item_bank_enabled());
    saved["items-screen"] = 0;
    saved["button-layout"] = 4;
    assert(button_layout() == ButtonLayout::BayxFlipped);
    saved["button-layout"] = 5;
    assert(button_layout() == ButtonLayout::Nintendo);
    saved["button-layout"] = 0;
    assert(hud_size_percent(HudSizeSetting::Overall) == 125);
    assert(hud_size_percent(HudSizeSetting::ControllerDiamond) == 75);
    assert(saved.at("hud-size") == 2);
    assert(hud_size_locked(HudSizeSetting::Hearts));
    assert(displayed_hud_size_percent(HudSizeSetting::ControllerDiamond) == 125);
    set_hud_size_percent(HudSizeSetting::ControllerDiamond, 90);
    assert(hud_size_percent(HudSizeSetting::ControllerDiamond) == 75);
    set_hud_size_percent(HudSizeSetting::Overall, 100);
    set_hud_size_percent(HudSizeSetting::ControllerDiamond, 83);
    set_hud_size_percent(HudSizeSetting::ActionText, 87);
    set_hud_size_percent(HudSizeSetting::DialogueText, 92);
    set_hud_size_percent(HudSizeSetting::Rupees, 81);
    set_hud_size_percent(HudSizeSetting::Minimap, 114);
    names.clear();
    assert(register_config(nullptr) == MOD_OK);
    assert(!swap_menu_buttons()); // reload must not overwrite saved Off
    assert(hud_size_percent(HudSizeSetting::Overall) == 100);
    assert(hud_size_percent(HudSizeSetting::ControllerDiamond) == 83);
    assert(hud_size_percent(HudSizeSetting::ActionText) == 87);
    assert(hud_size_percent(HudSizeSetting::DialogueText) == 92);
    assert(hud_scales().rupees == 0.81f);
    assert(hud_scales().minimap == 1.14f);

    UiService ui{};
    ui.pane_add_section = section;
    ui.pane_add_text = text;
    ui.pane_add_control = control;
    ui.window_push = window;
    ui.register_mods_panel = [](ModContext*, const UiModsPanelDesc*) { return MOD_OK; };
    ui.register_menu_tab = [](ModContext*, const UiMenuTabDesc* desc, UiMenuTabHandle* handle) {
        menuTab = *desc; *handle = 1; return MOD_OK;
    };
    svc_ui = &ui;
    assert(register_ui(nullptr) == MOD_OK);
    menuTab.on_selected(nullptr, nullptr);
    assert(controls.size() == 16);
    assert(std::string(controls[12].label) == "Rupee Scale");
    assert(std::string(controls[12].help_rml) == "Sizes the rupee icon and counter.");
    assert(displayed(controls[12]) == 81);
    assert(std::string(controls[14].label) == "Minimap Scale");
    assert(std::string(controls[14].help_rml) == "Sizes the minimap.");
    assert(displayed(controls[14]) == 114);
    assert(std::string(controls[8].label) == "Action Text Scale");
    assert(displayed(controls[8]) == 87);
    assert(std::string(controls[10].label) == "Dialogue Text Scale");
    assert(displayed(controls[10]) == 92);
    for (std::size_t i = 0; i < controls.size(); i += 2) {
        auto& number = controls[i];
        assert(number.kind == UI_CONTROL_NUMBER && number.min == 50 && number.max == 125);
        assert(number.step == 1 && std::string(number.suffix) == "%");
        assert(!number.is_disabled(nullptr, number.user_data));
        assert(controls[i + 1].kind == UI_CONTROL_BUTTON);
    }
    enter(controls[0], 125);
    for (std::size_t i = 2; i < controls.size(); i += 2) {
        assert(displayed(controls[i]) == 125);
        assert(controls[i].is_disabled(nullptr, controls[i].user_data));
        assert(controls[i + 1].is_disabled(nullptr, controls[i + 1].user_data));
    }
    // Even stale reset callbacks cannot overwrite a locked custom value.
    controls[3].on_pressed(nullptr, controls[3].user_data);
    controls[13].on_pressed(nullptr, controls[13].user_data);
    controls[15].on_pressed(nullptr, controls[15].user_data);
    controls[1].on_pressed(nullptr, controls[1].user_data);
    assert(displayed(controls[2]) == 83);
    assert(displayed(controls[12]) == 81);
    assert(displayed(controls[14]) == 114);
    for (std::size_t i : {12u, 14u}) {
        enter(controls[i], 0);
        assert(displayed(controls[i]) == 50);
        enter(controls[i], 900);
        assert(displayed(controls[i]) == 125);
        controls[i + 1].on_pressed(nullptr, controls[i + 1].user_data);
        assert(displayed(controls[i]) == 100);
        assert(!controls[i].is_modified(nullptr, controls[i].user_data));
    }
    assert(displayed(controls[8]) == 87);
    enter(controls[8], 10);
    assert(displayed(controls[10]) == 92); // independent of Action Text
    enter(controls[10], 10);
    assert(displayed(controls[10]) == 50);
    enter(controls[10], 900);
    assert(displayed(controls[10]) == 125);
    controls[11].on_pressed(nullptr, controls[11].user_data);
    assert(displayed(controls[10]) == 100);
    assert(!controls[10].is_modified(nullptr, controls[10].user_data));
    assert(displayed(controls[8]) == 50);
    enter(controls[8], 900);
    assert(displayed(controls[8]) == 125);
    controls[9].on_pressed(nullptr, controls[9].user_data);
    assert(displayed(controls[8]) == 100);
    assert(!controls[8].is_modified(nullptr, controls[8].user_data));
    assert(!controls[2].is_disabled(nullptr, controls[2].user_data));
    enter(controls[2], 0);
    assert(displayed(controls[2]) == 50);
    enter(controls[2], 900);
    assert(displayed(controls[2]) == 125);
    controls[3].on_pressed(nullptr, controls[3].user_data);
    assert(displayed(controls[2]) == 100);
    assert(!controls[2].is_modified(nullptr, controls[2].user_data));
    // A clean install starts at 100% with all individual controls available.
    saved.clear();
    names.clear();
    assert(register_config(nullptr) == MOD_OK);
    assert(swap_menu_buttons());
    for (auto setting : {HudSizeSetting::Overall, HudSizeSetting::ControllerDiamond,
             HudSizeSetting::Dpad, HudSizeSetting::Hearts, HudSizeSetting::ActionText,
             HudSizeSetting::DialogueText, HudSizeSetting::Rupees, HudSizeSetting::Minimap}) {
        assert(hud_size_percent(setting) == 100 && !hud_size_locked(setting));
    }
    std::cout << "PASS: migration/reload, numeric UI, override display/disable, saved values, clamping, reset\n";
}
