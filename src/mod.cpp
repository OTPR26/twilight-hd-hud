#include "config.hpp"
#include "font_override.hpp"
#include "service_imports.hpp"

#include "mods/service.hpp"
#include "mods/svc/config.h"
#include "mods/svc/hook.h"
#include "mods/svc/host.h"
#include "mods/svc/log.h"
#include "mods/svc/resource.h"
#include "mods/svc/ui.h"

DEFINE_MOD();
IMPORT_SERVICE(ConfigService, svc_config);
IMPORT_SERVICE(HookService, svc_hook);
IMPORT_SERVICE(HostService, svc_host);
IMPORT_SERVICE(LogService, svc_log);
IMPORT_SERVICE(ResourceService, svc_resource);
IMPORT_SERVICE(UiService, svc_ui);

namespace twilight_hd_hud {
ModResult install_item_slot_hooks(ModError* error);
ModResult register_ui(ModError* error);
void shutdown_update_checker();
void initialize_wolf_action_icons();
void shutdown_wolf_action_icons();
void initialize_face_button_textures();
void shutdown_face_button_textures();
void shutdown_item_slot_resources();
}

extern "C" {

MOD_EXPORT ModResult mod_initialize(ModError* error) {
    if (const ModResult result = twilight_hd_hud::register_config(error); result != MOD_OK) {
        return result;
    }
    if (const ModResult result = twilight_hd_hud::register_ui(error); result != MOD_OK) {
        return result;
    }
    if (const ModResult result = twilight_hd_hud::install_item_slot_hooks(error);
        result != MOD_OK)
    {
        return result;
    }

    twilight_hd_hud::initialize_wolf_action_icons();
    twilight_hd_hud::initialize_face_button_textures();
    twilight_hd_hud::initialize_font_override();

    svc_log->info(mod_ctx, "Twilight HD initialized");
    return MOD_OK;
}

MOD_EXPORT ModResult mod_update(ModError*) {
    return MOD_OK;
}

MOD_EXPORT ModResult mod_shutdown(ModError*) {
    twilight_hd_hud::shutdown_update_checker();
    twilight_hd_hud::shutdown_font_override();
    twilight_hd_hud::shutdown_item_slot_resources();
    twilight_hd_hud::shutdown_face_button_textures();
    twilight_hd_hud::shutdown_wolf_action_icons();
    svc_log->info(mod_ctx, "Twilight HD stopped");
    return MOD_OK;
}

}  // extern "C"
