#pragma once

#include "font_atlas.hpp"

namespace twilight_hd_hud {

inline bool is_zelda64rus_font(const void* resource) {
    if (!resource) return false;
    const auto* data = static_cast<const std::uint8_t*>(resource);
    if (std::memcmp(data, "FONTbfn1", 8) != 0 || font_atlas::be32(data + 8) != 74336)
        return false;
    // Zelda64rus v2.0 supplies both fonts with its own character mapping.
    std::uint32_t hash = 2166136261u;
    for (std::size_t i = 0; i < 74336; ++i) hash = (hash ^ data[i]) * 16777619u;
    return hash == 0x0d7e1687u || hash == 0x17f183ecu;
}

}  // namespace twilight_hd_hud
