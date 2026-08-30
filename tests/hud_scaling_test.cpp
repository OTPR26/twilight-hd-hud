#include "hud_scaling.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace twilight_hd_hud;

int main() {
    for (int master = 50; master <= 125; ++master)
    for (int size = 50; size <= 125; ++size) {
        const auto result = compose_hud_scales(master, 83, 92, 75, 87, 92, size, 175 - size);
        assert(result.rupees == (master == 100 ? size : master) / 100.0f);
        assert(result.minimap == (master == 100 ? 175 - size : master) / 100.0f);
        assert(std::fabs(rupee_icon_multiplier(result.overall, result.rupees) *
            result.overall - result.rupees) < 0.0001f);
        for (int digits : {3, 4}) {
            // Scaling cannot push the counter farther right/offscreen.
            const float right = rupee_icon_right(530, digits, result.rupees);
            assert(std::fabs(right + rupee_strip_width(digits, result.rupees) -
                (530 + rupee_strip_width(digits, 1))) < 0.0001f);
            assert(std::fabs(rupee_digit_size(result.rupees) / 11 - result.rupees) < 0.0001f);
        }
        const float originalHeight = 180;
        const float height = originalHeight * minimap_multiplier(result.minimap);
        const float y = 240 + originalHeight - height + 12;
        assert(std::fabs(y + height - (240 + originalHeight + 12)) < 0.0001f);
    }
    assert(rupee_icon_multiplier(1, 1) == 1);
    assert(rupee_icon_right(530, 4, 1) == 530);
    assert(rupee_digit_size(1) == 11 && rupee_digit_step(1) == 13 && rupee_digit_gap(1) == 5);
    assert(minimap_multiplier(1) == 0.70f);
    assert(std::fabs(action_text_multiplier(1.0f) - 0.8f) < 0.0001f);
    assert(std::fabs(action_text_multiplier(1.25f) - 1.0f) < 0.0001f);
    assert(std::fabs(dialogue_text_multiplier(1.0f) - 0.68f) < 0.0001f);
    for (int master = 50; master <= 125; ++master)
    for (int dialogue = 50; dialogue <= 125; ++dialogue) {
        const auto result = compose_hud_scales(master, 83, 92, 75, 87, dialogue);
        assert(result.dialogueText == dialogue / 100.0f);
        assert(result.actionText == 0.87f);
    }
    for (int master = 50; master <= 125; ++master)
    for (int action = 50; action <= 125; ++action) {
        const auto result = compose_hud_scales(master, 83, 92, 75, action);
        assert(result.actionText == action / 100.0f);
        assert(result.controllerDiamond == (master == 100 ? 83 : master) / 100.0f);
        const auto factor = action_text_multiplier(result.actionText);
        assert(factor >= 0.4f && factor <= 1.0f);
    }
    assert(legacy_hud_percent(0) == 75);
    assert(legacy_hud_percent(1) == 100);
    assert(legacy_hud_percent(2) == 125);
    assert(legacy_hud_percent(-1) == 100 && legacy_hud_percent(3) == 100);
    assert(clamp_hud_percent(std::numeric_limits<std::int64_t>::min()) == 50);
    assert(clamp_hud_percent(std::numeric_limits<std::int64_t>::max()) == 125);
    for (int master = 50; master <= 125; ++master)
    for (int diamond = 50; diamond <= 125; ++diamond)
    for (int dpad = 50; dpad <= 125; ++dpad)
    for (int hearts = 50; hearts <= 125; ++hearts) {
        const auto result = compose_hud_scales(master, diamond, dpad, hearts);
        assert(result.overall == master / 100.0f);
        assert(result.controllerDiamond == (master == 100 ? diamond : master) / 100.0f);
        assert(result.dpad == (master == 100 ? dpad : master) / 100.0f);
        assert(result.hearts == (master == 100 ? hearts : master) / 100.0f);
        assert(result.controllerDiamond >= 0.5f && result.controllerDiamond <= 1.25f);
    }
    std::cout << "PASS: all 33,362,176 percentage combinations, override/no multiplication, limits, legacy conversion\n";
}
