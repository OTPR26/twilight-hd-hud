#pragma once

namespace twilight_hd_hud::file_select_layout {

// Save/overwrite-question banner on the native 608-by-448 canvas. At 1080p the
// textured rules land near Y=108/181, matching the TPHD reference. Keep a
// small optical drop for the centered text, without changing other headers.
constexpr float kSaveQuestionFontSize = 16.5f;
constexpr float kSaveQuestionBannerTop = 43.5f;
constexpr float kSaveQuestionBannerBottom = 76.5f;
constexpr float kSaveQuestionTextCenter =
    (kSaveQuestionBannerTop + kSaveQuestionBannerBottom) * 0.5f + 0.5f;
constexpr float kSaveQuestionTextHeight = 24.0f;

// Both quest-log textures are 80 pixels tall. The lower divider occupies
// pixel 57; the bottom border begins at 76. Center the text in that clear
// band, independently of heart count, selection animation, or screen size.
constexpr float kPlayTimeCenter = (58.0f + 76.0f) * 0.5f / 80.0f;

constexpr float play_time_center(float rowTop, float rowBottom) {
    return rowTop + (rowBottom - rowTop) * kPlayTimeCenter;
}

}  // namespace twilight_hd_hud::file_select_layout
