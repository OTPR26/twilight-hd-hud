#include "hud_visibility.hpp"

#include <cassert>
#include <cstdint>
#include <initializer_list>

using namespace twilight_hd_hud;

struct Pane {
    Pane* parent = nullptr;
    bool visible = true;
    std::uint8_t alpha = 255;
    Pane* getParentPane() const { return parent; }
    bool isVisible() const { return visible; }
    std::uint8_t getAlpha() const { return alpha; }
};

int main() {
    assert(show_native_touch_replaced_hud(false));
    assert(!show_native_touch_replaced_hud(true));
    assert(third_slot_item_usable(0x42, 0xFF, 0x04, 0x04));
    assert(!third_slot_item_usable(0x42, 0xFF, 0x00, 0x04));
    assert(!third_slot_item_usable(0x00, 0xFF, 0x04, 0x04));
    assert(!third_slot_item_usable(0xFF, 0xFF, 0x04, 0x04));

    Pane root;
    Pane keyGroup{&root};
    Pane rupees{&keyGroup};
    assert(rupee_counter_alpha<Pane>(nullptr, false) == 0);
    // Transformation fade out/in, including reduced user HUD opacity.
    for (int opacity : {255, 180, 90}) {
        for (int frame : {5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5}) {
            rupees.alpha = static_cast<std::uint8_t>(opacity * frame / 5);
            for (int draw = 0; draw < 20; ++draw) {
                const auto before = rupees.alpha;
                assert(rupee_counter_alpha(&rupees, false) == before);
                assert(rupee_counter_alpha(&rupees, true) == 0);
                assert(rupees.alpha == before); // never reset the animation
            }
        }
    }
    rupees.alpha = 128;
    keyGroup.alpha = 180;
    assert(rupee_counter_alpha(&rupees, false) == 128); // not double-faded
    for (auto* pane : {&root, &keyGroup, &rupees}) {
        pane->visible = false;
        assert(rupee_counter_alpha(&rupees, false) == 0);
        pane->visible = true;
        const auto savedAlpha = pane->alpha;
        pane->alpha = 0;
        assert(rupee_counter_alpha(&rupees, false) == 0);
        pane->alpha = savedAlpha;
        assert(rupee_counter_alpha(&rupees, false) == 128);
    }
}
