#include "hud_scaling.hpp"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using namespace twilight_hd_hud;

int main() {
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
