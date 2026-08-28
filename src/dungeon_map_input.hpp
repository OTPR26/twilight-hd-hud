#pragma once
#include <cstdint>

namespace twilight_hd_hud {
constexpr std::uint32_t dungeon_map_navigation_buttons(std::uint32_t buttons,
    std::uint32_t directions) {
    return buttons & ~directions;
}
constexpr bool dungeon_map_back_requested(bool active, bool blocked, bool locked,
    std::uint32_t pressed, std::uint32_t up) {
    return active && !blocked && !locked && (pressed & up) != 0;
}
}
