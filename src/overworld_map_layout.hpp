#pragma once
#include "map_responsive_layout.hpp"
namespace twilight_hd_hud::overworld_map_layout {
struct Rect { float x, y, width, height; };
// TPHD screenshot proportions on the game's 608x448 layout canvas.
constexpr Rect frame{85, 70, 438, 334};
constexpr Rect content{91, 76, 426, 322};
constexpr float map_origin_x(float mapSize) {
    return content.x + (content.width - mapSize) * 0.5f;
}
constexpr float topRule = 23;
constexpr float bottomRule = 424;
constexpr float bannerWidth = 232;
constexpr float bannerY = 18;
constexpr float bannerHeight = 36;
// The glossy top rim is not part of the dark title area.
constexpr float bannerTitleTop = 10;
constexpr float bannerTitleBottom = bannerHeight - 2;
constexpr float bannerTitleHeight = bannerTitleBottom - bannerTitleTop;
constexpr float zoomAY = 38;
constexpr float zoomBY = 60;
constexpr float portalY = 382;
constexpr float areaNameX = content.x + 10;
constexpr float areaNameY = content.y + 6;
constexpr float areaNameFontSize = 17.5f;
constexpr float backY = 411;
constexpr float poeY = 104;
// The overworld count's native baseline sits slightly above the icon center.
// Lower only the text so it matches the dungeon-map counter presentation.
constexpr float poeTextYOffset = 5;
// The Poe icon is 30 px wide. Leave the same one-pixel icon-to-count gap as
// the dungeon map instead of the former detached eight-pixel extra spacing.
constexpr float poeIconToText = 31;
constexpr float safe_x(float left, float width, float fraction) {
    return left + width * fraction;
}
constexpr Rect back_hint(float left, float width) {
    // Back sits below the frame; Portals remains inside its lower-left corner.
    const float size = map_responsive_layout::scale(width);
    return {safe_x(left, width, 0.047f) - 12 * size, backY - 12 * size, 110 * size, 24 * size};
}
} // namespace twilight_hd_hud::overworld_map_layout
