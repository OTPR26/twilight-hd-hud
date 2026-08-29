#pragma once

namespace twilight_hd_hud {

enum class ItemHelpButton { None, Action, Back, ItemX, ItemY, Shoulder, Target, Trigger };

// Native out-font glyph IDs. Only item-help text uses this mapping; ordinary
// dialogue retains its own controls and illustrations.
constexpr ItemHelpButton item_help_button(int type, bool bowCombination, bool boomerang = false) {
    switch (type) {
    case 0: return ItemHelpButton::Action;
    case 1: return ItemHelpButton::Back;
    case 3: return ItemHelpButton::Target;
    case 4: return boomerang ? ItemHelpButton::Trigger :
        bowCombination ? ItemHelpButton::Target : ItemHelpButton::Shoulder;
    case 5: return ItemHelpButton::ItemX;
    case 6: return ItemHelpButton::ItemY;
    case 7: return ItemHelpButton::Shoulder;
    default: return ItemHelpButton::None;
    }
}

// Only the Gale Boomerang's inserted third-assignment glyph needs its gap
// tightened. Leave the native R targeting glyph and all following text alone.
// Out-font coordinates are local to the text box, so the shift scales with it.
constexpr float item_help_icon_x(float x, float width, int type, bool boomerang) {
    return boomerang && type == 7 && width > 0.0f ? x - width * 0.4f : x;
}

constexpr bool show_ring_assignment_prompts(bool wolf, int explanationStatus) {
    // Includes the description's opening and closing animations.
    return !wolf && explanationStatus == 0;
}

struct MeterOffset { float x; float y; };

struct OptionWarningSize { float width; float height; };

// All dimensions are in the same live screen space, even when the heading,
// warning frame and text belong to differently scaled native parent panes.
constexpr OptionWarningSize option_warning_size(float titleWidth, float titleHeight,
    float textWidth, float textHeight, float textScaleX, float textScaleY) {
    const float paddedWidth = textWidth + 28.0f * textScaleX;
    const float paddedHeight = textHeight + 16.0f * textScaleY;
    return {titleWidth > paddedWidth ? titleWidth : paddedWidth,
        titleHeight > paddedHeight ? titleHeight : paddedHeight};
}

// These are optical offsets for the authored HUD panes, not visible ink gaps.
// The cross texture and text cell both contain padding. Screenshot comparison
// with TPHD requires pulling Collection upward by 24% of the cross pane width
// from preview 6's +14% pane gap, while retaining its horizontal anchor.
constexpr MeterOffset collection_dpad_offset(float crossLeft, float crossRight,
    float crossBottom, float textLeft, float textRight, float textTop) {
    return {(crossLeft + crossRight - textLeft - textRight) * 0.5f,
        crossBottom - (crossRight - crossLeft) * 0.10f - textTop};
}

// The authored Minimap row is above the cross's optical center. Applied after
// restoring its canonical position each frame; X and all outline offsets stay.
constexpr float minimap_dpad_optical_offset(float crossWidth) {
    return crossWidth * 0.12f;
}

constexpr MeterOffset top_meter_offset(float safeLeft, float safeRight, float safeTop,
    float frameLeft, float frameTop, float frameRight, float frameBottom) {
    // Anchor the full frame, not the shrinking fill, so consumption cannot
    // move the meter. Coordinates use the game's 608-by-448 HUD canvas.
    return {(safeLeft + safeRight - frameLeft - frameRight) * 0.5f,
        safeTop + 54.0f - (frameTop + frameBottom) * 0.5f};
}

}  // namespace twilight_hd_hud
