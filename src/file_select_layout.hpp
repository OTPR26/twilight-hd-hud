#pragma once

namespace twilight_hd_hud::file_select_layout {

// File Selection and Save use the same header geometry.
constexpr float kHeaderFontSize = 24.0f;
constexpr float kHeaderBannerTop = 24.0f;
constexpr float kHeaderBannerBottom = 80.0f;
constexpr float kHeaderTextTop = 30.0f;
constexpr float kHeaderTextBottom = 66.0f;

// Both quest-log textures are 80 pixels tall. The lower divider occupies
// pixel 57; the bottom border begins at 76. Center the text in that clear
// band, independently of heart count, selection animation, or screen size.
constexpr float kPlayTimeCenter = (58.0f + 76.0f) * 0.5f / 80.0f;

// The action panels already have a thin gold border. Keep the selection
// ornaments just beyond that border instead of using the roomy generic-menu
// cursor padding, which makes them float away from Copy/Start/Erase at 1080p.
constexpr float kActionCursorPaddingX = 2.0f;
constexpr float kActionCursorPaddingY = 1.5f;

struct PromptGroupLayout {
    float scaleX;
    float centerX;
    float centerY;
};

constexpr PromptGroupLayout prompt_group_layout(float parentScaleX, float parentScaleY,
    float width, float height, float right, float top) {
    return {parentScaleY / parentScaleX,
        right - width * parentScaleY * 0.5f,
        top + height * parentScaleY * 0.5f};
}

constexpr float play_time_center(float rowTop, float rowBottom) {
    return rowTop + (rowBottom - rowTop) * kPlayTimeCenter;
}

}  // namespace twilight_hd_hud::file_select_layout
