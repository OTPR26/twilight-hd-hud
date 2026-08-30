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
    std::uint32_t dpadMenu, std::uint32_t down, std::uint32_t start,
    std::uint32_t suppress = 0, bool swap = true,
    bool preserveVirtualCollection = false) {
    const auto source = buttons & ~suppress;
    // Even with swapping off, a custom Midna-on-Down binding relocates the
    // D-Pad menu shortcut to Right and must not also open Items on Down.
    return (source & ~(dpadMenu | down | start)) |
        ((source & dpadMenu) ? (swap ? start : down) : 0) |
        ((source & start) ?
            (preserveVirtualCollection ? start : (swap ? down : start)) : 0);
}

constexpr const char* dpad_menu_label(bool swap) {
    return swap ? "Collection/\nSave" : "Items";
}

constexpr std::uint32_t restore_menu_shortcut_buttons(std::uint32_t current,
    std::uint32_t original, std::uint32_t mask) {
    return (current & ~mask) | (original & mask);
}

} // namespace twilight_hd_hud
