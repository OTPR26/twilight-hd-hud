#pragma once

#include "JSystem/JUtility/JUTResFont.h"

namespace twilight_hd_hud {

using FontDrawOriginal = f32 (*)(JUTResFont*, f32, f32, f32, f32, int, bool,
    FontDrawContext*);

void initialize_font_override();
void shutdown_font_override();
bool draw_font_override(void* args, void* retval, FontDrawOriginal drawOriginal);

}  // namespace twilight_hd_hud
