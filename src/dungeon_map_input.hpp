#pragma once
#include <cstdint>

namespace twilight_hd_hud {
constexpr std::uint32_t dungeon_map_navigation_buttons(std::uint32_t buttons,
    std::uint32_t directions) {
    return buttons & ~directions;
}
constexpr std::uint32_t field_map_suppressed_directions(bool portalWarpChoice,
    std::uint32_t directions) {
    // The field map normally owns the stick while this mod reserves the D-pad
    // for its Back hint.  The portal confirmation is a native yes/no menu,
    // however, and must retain the D-pad as an alternate choice input.
    return portalWarpChoice ? 0 : directions;
}
constexpr bool dungeon_map_back_requested(bool active, bool blocked, bool locked,
    std::uint32_t pressed, std::uint32_t up) {
    return active && !blocked && !locked && (pressed & up) != 0;
}
}
