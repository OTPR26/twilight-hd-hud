#pragma once

#include <cstdint>

namespace twilight_hd_hud {

constexpr std::int64_t clamp_hud_percent(std::int64_t percent) {
    return percent < 50 ? 50 : percent > 125 ? 125 : percent;
}

constexpr std::int64_t legacy_hud_percent(std::int64_t selection) {
    return selection == 0 ? 75 : selection == 2 ? 125 : 100;
}

constexpr float hud_size_multiplier(std::int64_t percent) {
    return static_cast<float>(clamp_hud_percent(percent)) / 100.0f;
}

constexpr std::int64_t effective_hud_percent(std::int64_t overall,
    std::int64_t individual) {
    return clamp_hud_percent(overall) != 100 ? clamp_hud_percent(overall) :
        clamp_hud_percent(individual);
}

struct HudScales {
    float overall;
    float controllerDiamond;
    float dpad;
    float hearts;
};

constexpr HudScales compose_hud_scales(std::int64_t overall,
    std::int64_t diamond = 100, std::int64_t dpad = 100, std::int64_t hearts = 100)
{
    return {hud_size_multiplier(overall),
        hud_size_multiplier(effective_hud_percent(overall, diamond)),
        hud_size_multiplier(effective_hud_percent(overall, dpad)),
        hud_size_multiplier(effective_hud_percent(overall, hearts))};
}

}  // namespace twilight_hd_hud
