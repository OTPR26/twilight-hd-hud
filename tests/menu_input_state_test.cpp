#include "menu_input_state.hpp"

#include <cassert>
#include <cstdint>
#include <initializer_list>

using namespace twilight_hd_hud;

int main() {
    for (bool player : {false, true}) {
        for (bool fileSelect : {false, true}) {
            for (bool paused : {false, true}) {
                for (bool blocked : {false, true}) {
                    const bool active = gameplay_shortcuts_active(player, fileSelect, paused, blocked);
                    assert(active == (player && !fileSelect && !paused && !blocked));
                    // The fixed gameplay mask used to eat Up/Left/Right on
                    // file select; Down survived. Test held/trigger combinations.
                    for (std::uint32_t buttons = 0; buttons < 16; ++buttons) {
                        const auto remaining = active ? buttons & ~0xbu : buttons;
                        if (fileSelect || !player) assert(remaining == buttons);
                        if (active) assert(remaining == (buttons & 4u));
                    }
                }
            }
        }
    }

    // Applies equally to dungeon and field maps and every native close path
    // (B, D-pad Up, touch, or confirmed warp): the input does not choose state.
    for (bool shown : {false, true}) {
        for (bool canDisplay : {false, true}) {
            MinimapReturnState state;
            assert(!state.close(true, canDisplay)); // scripted/no captured map
            state.begin(shown);
            for (int frame = 0; frame < 30; ++frame) {
                state.begin(!shown); // native state changes must not resnapshot
                assert(!state.close(false, canDisplay)); // open/zoom/navigation
            }
            const auto restore = state.close(true, canDisplay);
            assert(restore);
            assert(restore->preference == shown);
            assert(restore->visible == (shown && canDisplay));
            for (int frame = 0; frame < 30; ++frame) {
                assert(!state.close(true, canDisplay)); // no forced visibility loop
            }
            state.reset(); // completed close, new owner/meter, reset, or unload
            assert(!state.active());
            assert(!state.close(true, canDisplay));
            state.begin(!shown); // a new manual toggle is honored next time
            assert(state.close(true, true)->visible == !shown);
            state.reset();
            state.begin(shown);
            state.reset(); // aborted opening/scene transition
            assert(!state.close(true, true));
        }
    }
}
