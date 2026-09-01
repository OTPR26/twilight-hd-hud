#include "config.hpp"
#include "service_imports.hpp"

#include "mods/service.hpp"
#include "mods/svc/host.h"
#include "mods/svc/ui.h"

#include <array>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>

namespace twilight_hd_hud {
namespace {

UiWindowHandle s_settingsWindow = 0;
UiMenuTabHandle s_menuTab = 0;

constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/OTPR26/twilight-hd-hud/releases/latest";

enum class UpdateCheckState { Idle, Checking, Ready };

struct UpdateCheckResult {
    bool succeeded = false;
    bool updateAvailable = false;
    std::string currentVersion;
    std::string latestVersion;
};

std::atomic<UpdateCheckState> s_updateCheckState{UpdateCheckState::Idle};
std::mutex s_updateResultMutex;
UpdateCheckResult s_updateResult;
std::thread s_updateThread;

std::string json_string(const std::string& json, const char* name) {
    const std::string key = std::string{"\""} + name + "\"";
    auto pos = json.find(key);
    if (pos == std::string::npos) return {};
    pos = json.find(':', pos + key.size());
    if (pos == std::string::npos) return {};
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return {};
    const auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return {};
    return json.substr(pos + 1, end - pos - 1);
}

bool parse_version(const std::string& text, std::array<int, 4>& parts) {
    std::size_t pos = (!text.empty() && (text.front() == 'v' || text.front() == 'V')) ? 1 : 0;
    bool found = false;
    for (auto& part : parts) {
        if (pos >= text.size() || !std::isdigit(static_cast<unsigned char>(text[pos]))) break;
        found = true;
        part = 0;
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos]))) {
            part = part * 10 + (text[pos++] - '0');
        }
        if (pos >= text.size() || text[pos] != '.') break;
        ++pos;
    }
    return found;
}

bool version_is_newer(const std::string& latest, const std::string& current) {
    std::array<int, 4> latestParts{};
    std::array<int, 4> currentParts{};
    return parse_version(latest, latestParts) && parse_version(current, currentParts) &&
        latestParts > currentParts;
}

std::string fetch_latest_release() {
    const std::string command = std::string{
        "curl -fsSL --connect-timeout 8 --max-time 20 "
        "-H 'User-Agent: TwilightHDHUD-Updater' "
        "-H 'Accept: application/vnd.github+json' '"} + kLatestReleaseApi + "'";
#if defined(_WIN32)
    FILE* pipe = _popen(command.c_str(), "r");
#else
    FILE* pipe = popen(command.c_str(), "r");
#endif
    if (pipe == nullptr) return {};

    std::string response;
    std::array<char, 4096> buffer{};
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        response += buffer.data();
    }
#if defined(_WIN32)
    const int status = _pclose(pipe);
#else
    const int status = pclose(pipe);
#endif
    return status == 0 ? response : std::string{};
}

void start_update_check(ModContext*, void*) {
    UpdateCheckState expected = UpdateCheckState::Idle;
    if (!s_updateCheckState.compare_exchange_strong(expected, UpdateCheckState::Checking)) return;
    if (s_updateThread.joinable()) s_updateThread.join();

    const char* version = svc_host->mod_version(mod_ctx);
    const std::string current = version != nullptr ? version : "unknown";
    s_updateThread = std::thread([current] {
        UpdateCheckResult result;
        result.currentVersion = current;
        const std::string response = fetch_latest_release();
        result.latestVersion = json_string(response, "tag_name");
        result.succeeded = !result.latestVersion.empty();
        result.updateAvailable = result.succeeded &&
            version_is_newer(result.latestVersion, result.currentVersion);
        {
            std::lock_guard lock{s_updateResultMutex};
            s_updateResult = std::move(result);
        }
        s_updateCheckState.store(UpdateCheckState::Ready, std::memory_order_release);
    });
}

bool update_check_disabled(ModContext*, void*) {
    return s_updateCheckState.load(std::memory_order_acquire) != UpdateCheckState::Idle;
}

ModResult update_mod_panel(ModContext* ctx, void*, ModError*) {
    if (s_updateCheckState.load(std::memory_order_acquire) != UpdateCheckState::Ready) {
        return MOD_OK;
    }
    if (s_updateThread.joinable()) s_updateThread.join();

    UpdateCheckResult result;
    {
        std::lock_guard lock{s_updateResultMutex};
        result = s_updateResult;
    }
    s_updateCheckState.store(UpdateCheckState::Idle, std::memory_order_release);

    std::string body;
    const char* title = nullptr;
    UiDialogVariant variant = UI_DIALOG_NORMAL;
    if (!result.succeeded) {
        title = "Update Check Failed";
        variant = UI_DIALOG_WARNING;
        body = "Dusklight could not reach the Twilight HD releases page.<br/><br/>"
            "Check your internet connection and try again.";
    } else if (result.updateAvailable) {
        title = "Update Available";
        body = "A newer version of <b>Twilight HD</b> is available.<br/><br/>"
            "Installed: <b>v" + result.currentVersion + "</b><br/>Latest: <b>" +
            result.latestVersion + "</b><br/><br/>Download it from the project's GitHub Releases page.";
    } else {
        title = "Twilight HD Is Up to Date";
        body = "Installed: <b>v" + result.currentVersion + "</b><br/>Latest: <b>" +
            result.latestVersion + "</b>";
    }

    const UiDialogAction action{"OK", nullptr, nullptr, false};
    UiDialogDesc desc = UI_DIALOG_DESC_INIT;
    desc.title = title;
    desc.body_rml = body.c_str();
    desc.variant = variant;
    desc.actions = &action;
    desc.action_count = 1;
    return svc_ui->dialog_push(ctx, &desc, nullptr);
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
    if (add_section(ctx, left, "Twilight HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_text(ctx, left, "Uses a TPHD-inspired HUD, fonts, and styles.")
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
    UiControlDesc swap = UI_CONTROL_DESC_INIT;
    swap.kind = UI_CONTROL_TOGGLE;
    swap.label = "TPHD Items / Collection Buttons";
    swap.help_rml = "On: D-Pad Down opens Collection/Save; Start / + opens Items.<br/>"
        "Off: D-Pad Down opens Items; Start / + opens Collection/Save.";
    swap.binding = UI_BINDING_CONFIG_VAR;
    swap.config_var = swap_menu_buttons_config_var();
    return svc_ui->pane_add_control(ctx, left, &swap, nullptr);
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
            "Overall overrides visual HUD groups unless it is 100%; text scales remain independent. "
            "Returning to 100% restores your individual icon values. Use Dusklight's Minimal HUD "
            "option to hide the HUD.")
            != MOD_OK) return MOD_ERROR;

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
    constexpr const char* help[] = {
        "Any value other than 100% overrides visual HUD groups, but never text. Icon controls show "
        "that value and are disabled. Reset Overall to restore their saved individual sizes.",
        "Sizes the face-button/item cluster, ammo, and Wolf Link action icons without changing text. "
        "Set Overall to 100% to edit this percentage.",
        "Sizes the D-pad and map icon without changing labels. Set Overall to 100% to edit this percentage.",
        "Sizes gameplay hearts, not save-menu hearts. Set Overall to 100% to edit this percentage.",
        "Sizes bottom-center action text. 100% uses the TPHD-style size; 125% restores the previous size.",
        "Sizes dialogue text. 100% uses the TPHD-style size.",
        "Sizes the rupee icon and counter.",
        "Sizes the minimap.",
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
    if (add_section(ctx, panel, "Twilight HD") != MOD_OK) {
        return MOD_ERROR;
    }
    if (add_button(ctx, panel, "Open Twilight HD Settings", open_settings) != MOD_OK) {
        return MOD_ERROR;
    }
    UiControlDesc update = UI_CONTROL_DESC_INIT;
    update.kind = UI_CONTROL_BUTTON;
    update.label = "Check for Updates";
    update.on_pressed = start_update_check;
    update.is_disabled = update_check_disabled;
    return svc_ui->pane_add_control(ctx, panel, &update, nullptr);
}

}  // namespace

ModResult register_ui(ModError* error) {
    UiModsPanelDesc panel = UI_MODS_PANEL_DESC_INIT;
    panel.build = build_mod_panel;
    panel.update = update_mod_panel;
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

void shutdown_update_checker() {
    if (s_updateThread.joinable()) s_updateThread.join();
    s_updateCheckState.store(UpdateCheckState::Idle, std::memory_order_release);
}

}  // namespace twilight_hd_hud
