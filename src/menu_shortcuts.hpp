#pragma once

#include <cstdint>

namespace twilight_hd_hud {

constexpr bool menu_shortcuts_active(unsigned windowStatus, bool inputBlocked) {
    return windowStatus == 0 && !inputBlocked;
}

// Used only inside the no-menu dispatcher, never as a saved controller remap.
// Native Start opens Collection; native Down opens Items. Capture both source
// bits before translating so simultaneous presses cannot cascade through twice.
constexpr std::uint32_t menu_shortcut_buttons(std::uint32_t buttons,
    std::uint32_t collection, std::uint32_t down, std::uint32_t start,
    std::uint32_t suppress = 0) {
    const auto source = buttons & ~suppress;
    return (source & ~(collection | down | start)) |
        ((source & collection) ? start : 0) |
        ((source & start) ? down : 0);
}

constexpr std::uint32_t restore_menu_shortcut_buttons(std::uint32_t current,
    std::uint32_t original, std::uint32_t mask) {
    return (current & ~mask) | (original & mask);
}

} // namespace twilight_hd_hud
