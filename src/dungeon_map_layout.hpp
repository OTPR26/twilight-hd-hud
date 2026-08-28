#pragma once

namespace twilight_hd_hud::dungeon_map_layout {
struct Point { float x, y; };
struct Rect { float x, y, width, height; };

// Coordinates must be in the SAME parent space as the source center pane.
constexpr Rect frame_around(Rect center) {
    return {center.x + center.width / 2 - 178,
        center.y + center.height / 2 - 156, 356, 312};
}

// Global extents include every ancestor's scale. Native corner artwork extends
// beyond its anchor: a small vertical inset keeps the visible tips close.
constexpr Point cursor_corner(Rect row, int corner) {
    return {corner < 2 ? row.x - 3 : row.x + row.width + 3,
        corner % 2 == 0 ? row.y + 1 : row.y + row.height - 1};
}

inline constexpr Rect banner{16, 14, 236, 38};
constexpr Rect back_hint(Rect frame, float scale = 1) {
    return {frame.x + 10 * scale, frame.y + frame.height + 4 * scale, 110 * scale, 24 * scale};
}
// The native 128x64 crest repeats at its original 2:1 aspect ratio. Keep
// the entire strip inside the banner's curved lower corners and top cap.
inline constexpr int banner_motif_count = 5;
constexpr Rect banner_motif(int index) { return {24.0f + 44.0f * index, 25, 44, 22}; }
inline constexpr float band_top[2] = {24, 425};
inline constexpr float band_height = 6;
inline constexpr float band_edge = 0.7f;
inline constexpr float prompt_group_x = 467.5f;
inline constexpr float prompt_group_y = 31.5f;
constexpr Point prompt_icon(int index) { return {index == 0 ? 147.0f : 125.0f,
    index == 0 ? 10.0f : 29.0f}; }
}
