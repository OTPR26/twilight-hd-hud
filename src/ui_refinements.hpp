#pragma once

namespace twilight_hd_hud {

enum class ItemHelpButton {
    None, Action, Back, Stick, ItemX, ItemY, Shoulder, Target, Trigger
};

constexpr float kItemHelpArtworkScale = 1.5f;
constexpr float kItemHelpTitleFontSize = 28.0f;

// Native out-font glyph IDs. Only item-help text uses this mapping; ordinary
// dialogue retains its own controls and illustrations.
constexpr ItemHelpButton item_help_button(int type, bool bowCombination, bool boomerang = false) {
    switch (type) {
    case 0: return ItemHelpButton::Action;
    case 1: return ItemHelpButton::Back;
    case 2: return ItemHelpButton::Stick;
    // With pointer input disabled, the same aiming instruction is rendered as
    // the animated stick-cross glyph instead of the Wii reticule glyph.
    case 9: return ItemHelpButton::Stick;
    // The aiming symbol used by Slingshot and Hero's Bow is sourced from the
    // Wii reticule tag even when the active controller UI presents it as the
    // right stick.
    case 69: return ItemHelpButton::Stick;
    case 3: return ItemHelpButton::Target;
    case 4: return boomerang ? ItemHelpButton::Trigger :
        bowCombination ? ItemHelpButton::Target : ItemHelpButton::Shoulder;
    case 5: return ItemHelpButton::ItemX;
    case 6: return ItemHelpButton::ItemY;
    case 7: return ItemHelpButton::Shoulder;
    default: return ItemHelpButton::None;
    }
}

constexpr bool item_help_shift_stick(bool clawshot, unsigned occurrence) {
    // The Clawshot help text reuses the animated aiming glyph twice. Its first
    // occurrence has the same excessive leading gap as Bow/Slingshot, while
    // the second occurrence is authored at the correct position already.
    return !clawshot || occurrence == 0;
}

// The replacement glyph artwork is wider than the message cell reserved by
// the TPHD body font. Pull every supported glyph left inside that cell so its
// visible right edge does not collide with punctuation or the following word.
// Wide shoulder/trigger/target caps need more compensation than the circular
// face buttons, while the native stick art has its own asymmetric texture box.
// Coordinates are local to the text box, so this scales with the card and with
// crowded-card font fitting.
constexpr float item_help_icon_x(float x, float width, ItemHelpButton button) {
    if (width <= 0.0f) return x;
    switch (button) {
    case ItemHelpButton::Action:
        return x - width * 0.35f;
    case ItemHelpButton::Back:
        return x - width * 0.3f;
    case ItemHelpButton::ItemX:
    case ItemHelpButton::ItemY:
        return x - width * 0.4f;
    case ItemHelpButton::Stick:
        return x - width * 0.4f;
    case ItemHelpButton::Target:
        return x - width * 0.6f;
    case ItemHelpButton::Trigger:
        return x - width * 0.2f;
    case ItemHelpButton::Shoulder:
        return x - width * 0.4f;
    case ItemHelpButton::None:
        return x;
    }
    return x;
}

// Out-font pictures are positioned from their texture box rather than the
// visible cap/glyph. Their lower shadow makes the replacement controls read a
// little below the TPHD text baseline. The tall native stick and the shallow
// ZL/ZR caps need slightly different optical corrections.
constexpr float item_help_icon_y(float y, float height, ItemHelpButton button) {
    if (height <= 0.0f) return y;
    switch (button) {
    case ItemHelpButton::Action:
        return y - height * 0.25f;
    case ItemHelpButton::Stick:
        return y - height * 0.35f;
    case ItemHelpButton::Target:
    case ItemHelpButton::Trigger:
        return y - height * 0.3f;
    case ItemHelpButton::Back:
    case ItemHelpButton::ItemX:
    case ItemHelpButton::ItemY:
    case ItemHelpButton::Shoulder:
        return y - height * 0.1f;
    case ItemHelpButton::None:
        return y;
    }
    return y;
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
