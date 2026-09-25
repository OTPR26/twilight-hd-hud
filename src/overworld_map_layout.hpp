#pragma once
#include "map_responsive_layout.hpp"
namespace twilight_hd_hud::overworld_map_layout {
struct Rect { float x, y, width, height; };
constexpr Rect transition_bounds(Rect bounds, float x, float y) {
    return {bounds.x + x, bounds.y + y, bounds.width, bounds.height};
}
// TPHD proportions expressed on the game's 608x448 layout canvas.
constexpr Rect frame{85, 70, 438, 334};
constexpr Rect content{91, 76, 426, 322};
constexpr Rect reference_content(float left, float width) {
    return {left + width * content.x / 608, content.y,
            width * content.width / 608, content.height};
}
struct PresentationFit { float sx, sy, dx, dy; };
constexpr PresentationFit presentation_fit(Rect from, Rect to, float slideX, float slideY) {
    const float sx = to.width / from.width, sy = to.height / from.height;
    return {sx, sy, to.x - from.x * sx + slideX * (1 - sx),
            to.y - from.y * sy + slideY * (1 - sy)};
}
// Fit the decorative texture's opening around native map bounds. The texture's
// inset scales with it; changing the frame must never resize the map itself.
constexpr Rect frame_for_content(Rect bounds) {
    const float sx = bounds.width / content.width;
    const float sy = bounds.height / content.height;
    return {bounds.x - (content.x - frame.x) * sx,
            bounds.y - (content.y - frame.y) * sy,
            frame.width * sx, frame.height * sy};
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
// Reference stagger: the lower B prompt sits left of the upper A prompt.
constexpr float zoomAY = 38;
constexpr float zoomBY = 60;
constexpr float zoomAFraction = 0.946f;
constexpr float zoomBFraction = 0.922f;
constexpr float zoomIconSize = 28;
constexpr float zoomLabelInset = 17;
// Eight non-overlapping pieces omit the opaque center of the frame texture.
// Wider corner pieces retain the curved inset; straight edges use the 6px rim.
constexpr Rect borderPieces[] = {
    {0, 0, 12, 12}, {426, 0, 12, 12},
    {0, 322, 12, 12}, {426, 322, 12, 12},
    {12, 0, 414, 6}, {12, 328, 414, 6},
    {0, 12, 6, 310}, {432, 12, 6, 310},
};
constexpr float portalY = 382;
constexpr float areaNameX = content.x + 10;
constexpr float areaNameY = content.y + 6;
constexpr float areaNameFontSize = 17.5f;
constexpr float backY = 411;
constexpr float poeY = 104;
// The overworld count's native baseline sits slightly above the icon center.
// Lower only the text so it matches the dungeon-map counter presentation.
constexpr float poeTextYOffset = 5;
// The Poe icon is 30 px wide; leave one pixel before the count.
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
