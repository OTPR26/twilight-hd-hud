#pragma once
#include "map_responsive_layout.hpp"
namespace twilight_hd_hud::overworld_map_layout {
struct Rect { float x, y, width, height; };
// TPHD screenshot proportions on the game's 608x448 layout canvas.
constexpr Rect frame{85, 70, 438, 334};
constexpr Rect content{91, 76, 426, 322};
constexpr float topRule = 23;
constexpr float bottomRule = 424;
constexpr float bannerWidth = 232;
constexpr float bannerY = 18;
constexpr float bannerHeight = 36;
constexpr float zoomAY = 38;
constexpr float zoomBY = 60;
constexpr float portalY = 382;
constexpr float poeY = 104;
// The overworld count's native baseline sits slightly above the icon center.
// Lower only the text so it matches the dungeon-map counter presentation.
constexpr float poeTextYOffset = 2;
// The Poe icon is 30 px wide. Leave the same one-pixel icon-to-count gap as
// the dungeon map instead of the former detached eight-pixel extra spacing.
constexpr float poeIconToText = 31;
constexpr float safe_x(float left, float width, float fraction) {
    return left + width * fraction;
}
constexpr Rect back_hint(float left, float width) {
    // Keep pass 11's Back icon center (~148,923 at 1080p), using the
    // dungeon map's 24px layout canvas and 31px text offset.
    const float size = map_responsive_layout::scale(width);
    return {safe_x(left, width, 0.077f) - 12 * size, 383 - 12 * size, 110 * size, 24 * size};
}
} // namespace twilight_hd_hud::overworld_map_layout
