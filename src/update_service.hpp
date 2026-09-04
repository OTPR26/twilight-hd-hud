#pragma once

#include "mods/api.h"

namespace twilight_hd_hud {

#ifdef TWILIGHT_HD_HUD_NO_UPDATE_SERVICE
inline void initialize_update_service() {}
inline void request_update_check(ModContext*, void*) {}
inline bool update_service_busy(ModContext*, void*) { return false; }
inline void update_update_service() {}
inline void shutdown_update_service() {}
#else
void initialize_update_service();
void request_update_check(ModContext*, void*);
bool update_service_busy(ModContext*, void*);
void update_update_service();
void shutdown_update_service();
#endif

}  // namespace twilight_hd_hud
