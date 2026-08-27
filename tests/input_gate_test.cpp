#include "input_gate.hpp"

#include <cassert>
#include <cstdint>

using twilight_hd_hud::InputGate;

int main() {
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
