#include "wolf_touch_icons.hpp"
#include "config.hpp"
#include "service_imports.hpp"

#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_msg_object.h"

#include <array>
#include <cstdint>
#include <string>

namespace twilight_hd_hud {
namespace {
UiStyleHandle s_style = 0;
std::string s_stylesheet;
bool s_failed = false;

bool prepare_icons() {
    constexpr std::array names{"sense", "dig-left", "attack"};
    constexpr std::array buttons{"x", "y", "b"};
    for (std::size_t i = 0; i < names.size(); ++i) {
        const std::string resource = std::string("ui/wolf/") + names[i] + ".png";
        ResourceBuffer buffer = RESOURCE_BUFFER_INIT;
        if (svc_resource->load(mod_ctx, resource.c_str(), &buffer) != MOD_OK) return false;
        std::uint64_t hash = 14695981039346656037ull;
        const auto* bytes = static_cast<const unsigned char*>(buffer.data);
        for (std::size_t byte = 0; byte < buffer.size; ++byte)
            hash = (hash ^ bytes[byte]) * 1099511628211ull;
        svc_resource->free(mod_ctx, &buffer);
        // The bundle texture provider also supports signed mobile builds.
        const std::string source = "mod://" + std::string(svc_host->mod_id(mod_ctx)) +
            "/res/" + resource + "?v=" + std::to_string(hash);
        const std::string selector = "#button-" + std::string(buttons[i]);
        s_stylesheet += selector + " { decorator: image(\"" + source + "\" contain center center); }\n";
        s_stylesheet += selector + " span { position: absolute; right: 6dp; bottom: 6dp; "
            "font-size: var(--font-size-xs); line-height: 1; }\n";
        s_stylesheet += selector + " .item-icon { display: none; }\n";
    }
    return true;
}
}

void update_wolf_touch_icons() {
    const bool active = wolf_touch_icons_enabled() && dComIfGp_getLinkPlayer() != nullptr &&
        daPy_py_c::checkNowWolf() && !dComIfGp_event_runCheck() &&
        !dComIfGp_isPauseFlag() && dComIfGp_getMesgStatus() == 0 &&
        !dMsgObject_isTalkNowCheck();
    if (!active) {
        if (s_style != 0 && svc_ui->unregister_styles(mod_ctx, s_style) == MOD_OK) s_style = 0;
        return;
    }
    if (s_style != 0 || s_failed) return;
    if ((s_stylesheet.empty() && !prepare_icons()) ||
        svc_ui->register_styles(mod_ctx, UI_SCOPE_TOUCH_CONTROLS,
            s_stylesheet.c_str(), &s_style) != MOD_OK) {
        s_failed = true;
        svc_log->warn(mod_ctx, "Wolf touch icons unavailable; keeping HUD icons");
    }
}

bool wolf_touch_icons_active() { return s_style != 0; }

void shutdown_wolf_touch_icons() {
    if (s_style != 0) svc_ui->unregister_styles(mod_ctx, s_style);
    s_style = 0;
    s_stylesheet.clear();
    s_failed = false;
}
}
