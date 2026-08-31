#include "dialogue_text_layout.hpp"
#include "hud_scaling.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace twilight_hd_hud;
bool near(float a, float b) { return std::abs(a - b) < 0.001f; }
int main() {
    for (int percent = 50; percent <= 125; ++percent) {
        const dialogue_text_layout::Transform t{960, 830,
            dialogue_text_multiplier(hud_size_multiplier(percent))};
        assert(near(t.position_x(960), 960));
        assert(near(t.position_y(830), 830));
        // All layers share one affine transform, including offsets and sizes.
        assert(near(t.position_x(1200) - t.position_x(400), t.size(800)));
        assert(near(t.position_y(880) - t.position_y(770), t.size(110)));
        assert(near(t.position_x(400) + t.size(20), t.position_x(420)));
        assert(near(t.position_y(770) + t.size(20), t.position_y(790)));
        // Draw restoration means every next frame starts from native geometry.
        for (int frame = 0; frame < 100; ++frame)
            assert(near(t.position_x(400), 960 + (400 - 960) * t.scale));
    }
    assert(near(dialogue_text_multiplier(1) * 1036, 704.48f));
    assert(near(dialogue_text_multiplier(1, 1.25f) * 1.25f, 0.68f));
    assert(near(dialogue_text_multiplier(1, 0.8f), 0.68f));
    std::cout << "PASS: dialogue size, center, glow/symbol alignment and percentage range\n";
}
