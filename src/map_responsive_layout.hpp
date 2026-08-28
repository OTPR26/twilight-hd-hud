#pragma once

namespace twilight_hd_hud::map_responsive_layout {
// Fit the approved 16:9 composition inside the current safe width. Wider
// displays retain their existing layout; narrow displays scale uniformly.
inline constexpr float referenceWidth = 448.0f * 16.0f / 9.0f;
constexpr float scale(float width) { return width < referenceWidth ? width / referenceWidth : 1.0f; }
constexpr float x(float original, float left, float width) {
    return left + width * 0.5f + (original - 304.0f) * scale(width);
}
constexpr float y(float original, float width) {
    return 237.0f + (original - 237.0f) * scale(width);
}
}
