#pragma once
#include <cstdint>

namespace twilight_hd_hud::map_palette {
// Only saturated green terrain: leave water, gold markers and neutral art alone.
constexpr bool terrain(int r, int g, int b) {
    return g > 24 && g * 4 > r * 5 && g * 3 > b * 5;
}
constexpr std::uint16_t muted(std::uint16_t packed) {
    const bool opaque = (packed & 0x8000) != 0;
    const int r = opaque ? ((packed >> 10) & 31) * 8 : ((packed >> 8) & 15) * 16;
    const int g = opaque ? ((packed >> 5) & 31) * 8 : ((packed >> 4) & 15) * 16;
    const int b = opaque ? (packed & 31) * 8 : (packed & 15) * 16;
    if (!terrain(r, g, b)) return packed;
    const int green = 56 + g / 2, red = green * 58 / 100, blue = green * 85 / 100;
    return opaque ? 0x8000 | ((red >> 3) << 10) | ((green >> 3) << 5) | (blue >> 3) :
        (packed & 0x7000) | ((red >> 4) << 8) | ((green >> 4) << 4) | (blue >> 4);
}
}
