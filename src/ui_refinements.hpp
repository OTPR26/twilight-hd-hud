#pragma once

namespace twilight_hd_hud {

enum class ItemHelpButton { None, Action, Back, ItemX, ItemY, Shoulder, Target };

// Native out-font glyph IDs. Only item-help text uses this mapping; ordinary
// dialogue retains its own controls and illustrations.
constexpr ItemHelpButton item_help_button(int type, bool bowCombination) {
    switch (type) {
    case 0: return ItemHelpButton::Action;
    case 1: return ItemHelpButton::Back;
    case 3: return ItemHelpButton::Target;
    case 4: return bowCombination ? ItemHelpButton::Target : ItemHelpButton::Shoulder;
    case 5: return ItemHelpButton::ItemX;
    case 6: return ItemHelpButton::ItemY;
    case 7: return ItemHelpButton::Shoulder;
    default: return ItemHelpButton::None;
    }
}

constexpr bool show_ring_assignment_prompts(bool wolf, int explanationStatus) {
    // Includes the description's opening and closing animations.
    return !wolf && explanationStatus == 0;
}

struct MeterOffset { float x; float y; };

constexpr MeterOffset top_meter_offset(float safeLeft, float safeRight, float safeTop,
    float frameLeft, float frameTop, float frameRight, float frameBottom) {
    // Anchor the full frame, not the shrinking fill, so consumption cannot
    // move the meter. Coordinates use the game's 608-by-448 HUD canvas.
    return {(safeLeft + safeRight - frameLeft - frameRight) * 0.5f,
        safeTop + 54.0f - (frameTop + frameBottom) * 0.5f};
}

}  // namespace twilight_hd_hud
