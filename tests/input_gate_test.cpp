#include "input_gate.hpp"
#include "menu_shortcuts.hpp"

#include <cassert>
#include <cstdint>
#include <string_view>

using twilight_hd_hud::InputGate;

int main() {
    using namespace twilight_hd_hud;
    constexpr std::uint32_t down = 0x4, start = 0x1000, right = 0x2, a = 0x100;
    assert(menu_shortcuts_active(0, false));
    for (unsigned window = 1; window < 256; ++window) {
        assert(!menu_shortcuts_active(window, false)); // all native menu navigation
        assert(!menu_shortcuts_active(window, true));
    }
    assert(!menu_shortcuts_active(0, true)); // host settings/mod manager
    assert(menu_shortcut_buttons(down, down, down, start) == start);
    assert(menu_shortcut_buttons(start, down, down, start) == down);
    assert(menu_shortcut_buttons(start | down, down, down, start) == (start | down));
    // Follow-mode Midna on Down relocates Collection to Right; Start still
    // opens Items, and the Midna press cannot also open either menu.
    assert(menu_shortcut_buttons(right, right, down, start, down) == start);
    assert(menu_shortcut_buttons(down, right, down, start, down) == 0);
    assert(menu_shortcut_buttons(start, right, down, start, down) == down);
    assert(menu_shortcut_buttons(start, down, down, start, start) == 0);
    assert(menu_shortcut_buttons(a | down, down, down, start, a) == start);
    assert(std::string_view(dpad_menu_label(true)) == "Collection/\nSave");
    assert(std::string_view(dpad_menu_label(false)) == "Items");
    assert(menu_shortcut_buttons(down, down, down, start, 0, false) == down);
    assert(menu_shortcut_buttons(start, down, down, start, 0, false) == start);
    assert(menu_shortcut_buttons(start | down, down, down, start, 0, false) == (start | down));
    // Custom Midna retains its own button in either mode; Right opens Items
    // when swapping is off. Suppression applies before either translation.
    assert(menu_shortcut_buttons(right, right, down, start, down, false) == down);
    assert(menu_shortcut_buttons(down, right, down, start, down, false) == 0);
    assert(menu_shortcut_buttons(start, right, down, start, down, false) == start);
    assert(menu_shortcut_buttons(start, down, down, start, start, false) == 0);
    assert(menu_shortcut_buttons(a | down, down, down, start, a, false) == down);
    // Exhaust combinations of unrelated buttons, held state, and trigger
    // state. Restore exactly the touched bits without resurrecting unrelated
    // input that the native menu may have consumed.
    for (std::uint32_t original = 0; original < 0x2000; ++original) {
        assert(menu_shortcut_buttons(original, down, down, start, 0, false) == original);
        for (bool swap : {false, true}) {
            const auto custom = menu_shortcut_buttons(original, right, down, start, down, swap);
            assert((custom & ~(right | down | start)) == (original & ~(right | down | start)));
            assert((custom & right) == 0);
            assert(bool(custom & (swap ? start : down)) == bool(original & right));
            assert(bool(custom & (swap ? down : start)) == bool(original & start));
            assert(restore_menu_shortcut_buttons(custom, original, right | down | start) == original);
            assert(restore_menu_shortcut_buttons(custom & ~a, original, right | down | start) ==
                (original & ~a));
        }
        const auto mapped = menu_shortcut_buttons(original, down, down, start);
        assert((mapped & ~(down | start)) == (original & ~(down | start)));
        assert(menu_shortcut_buttons(mapped, down, down, start) == original);
        assert(restore_menu_shortcut_buttons(mapped, original, down | start) == original);
        const auto consumed = restore_menu_shortcut_buttons(mapped & ~a, original, down | start);
        assert(consumed == (original & ~a));
    }

    constexpr std::uint32_t all = 0xf;
    for (std::uint32_t button = 1; button <= 8; button <<= 1) {
        InputGate gate;
        gate.update(false, button);
        assert(gate.held(button));
        assert(!gate.blocked());

        // Opening a menu cuts off a control already held during gameplay.
        gate.update(true, button);
        assert(gate.blocked());
        assert(!gate.held(all));
        gate.update(true, 0);
        gate.update(true, button);
        assert(!gate.held(all));

        // Closing the menu must not turn a menu press into a gameplay press.
        gate.update(false, button);
        assert(!gate.blocked());
        assert(!gate.held(all));
        assert(gate.suppressed(button));
        gate.update(false, button);
        assert(!gate.held(all));
        gate.update(false, 0);
        assert(!gate.suppressed(all));
        gate.update(false, button);
        assert(gate.held(button));

        // Catch a close-menu press even if no pad read sampled it while open.
        gate.update(true, 0);
        gate.update(false, button);
        assert(!gate.held(all));
        gate.update(false, 0);
        gate.update(false, button);
        assert(gate.held(button));
    }

    // Controls re-arm independently: releasing L must not re-arm a held ZR.
    for (std::uint32_t released = 0; released <= all; ++released) {
        InputGate gate;
        gate.update(true, all);
        gate.update(false, all);
        assert(!gate.held(all));
        gate.update(false, all & ~released);
        gate.update(false, all);
        for (std::uint32_t button = 1; button <= 8; button <<= 1) {
            assert(gate.held(button) == ((released & button) != 0));
        }
    }

    // Nested menu transitions stay blocked; disconnecting releases the hold.
    InputGate gate;
    gate.update(true, all);
    gate.update(true, all);
    assert(!gate.held(all));
    gate.update(false, 0);
    gate.update(false, all);
    assert(gate.held(all));

    // Ordinary gameplay has no delay or suppression.
    InputGate gameplay;
    for (std::uint32_t held = 0; held <= all; ++held) {
        gameplay.update(false, held);
        for (std::uint32_t button = 1; button <= 8; button <<= 1) {
            assert(gameplay.held(button) == ((held & button) != 0));
        }
    }
}
