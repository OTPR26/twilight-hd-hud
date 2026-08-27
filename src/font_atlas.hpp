#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace twilight_hd_hud::font_atlas {

constexpr int cell = 128;
constexpr int ascent = 104;
constexpr int descent = 24;
constexpr int side = cell * 16;
// Zen/M PLUS have a 65-pixel H; Fira has 57 to leave room for its descenders.
// Normalize their visible capital height,
// not their padded texture cell, to 75% of the game's requested font height.
constexpr float opticalScale = 0.75f * cell / 65;
constexpr float firaOpticalScale = 0.75f * cell / 57;
constexpr std::size_t textureBytes = side * side / 2;
constexpr std::size_t infOffset = 32;
constexpr std::size_t widOffset = 64;
constexpr std::size_t mapOffset = 608;
constexpr std::size_t glyOffset = 640;
constexpr std::size_t fileBytes = glyOffset + 32 + textureBytes;

inline std::uint16_t be16(const std::uint8_t* p) {
    return (std::uint16_t(p[0]) << 8) | p[1];
}
inline std::uint32_t be32(const std::uint8_t* p) {
    return (std::uint32_t(be16(p)) << 16) | be16(p + 2);
}

// Accept only the deliberately small, single-page format emitted by our generator.
// This prevents malformed/partial resources reaching the native unchecked BFN reader.
inline bool valid(const void* data, std::size_t size) {
    if (!data || size != fileBytes) return false;
    const auto* p = static_cast<const std::uint8_t*>(data);
    return std::memcmp(p, "FONTbfn1", 8) == 0 && be32(p + 8) == size &&
        be32(p + 12) == 4 &&
        std::memcmp(p + infOffset, "INF1", 4) == 0 && be32(p + infOffset + 4) == 32 &&
        be16(p + infOffset + 8) == 0 && be16(p + infOffset + 10) == ascent &&
        be16(p + infOffset + 12) == descent && be16(p + infOffset + 14) == cell &&
        be16(p + infOffset + 16) == cell && be16(p + infOffset + 18) == '?' &&
        std::memcmp(p + widOffset, "WID1", 4) == 0 && be32(p + widOffset + 4) == 544 &&
        be16(p + widOffset + 8) == 0 && be16(p + widOffset + 10) == 255 &&
        std::memcmp(p + mapOffset, "MAP1", 4) == 0 && be32(p + mapOffset + 4) == 32 &&
        be16(p + mapOffset + 8) == 0 && be16(p + mapOffset + 10) == 0 &&
        be16(p + mapOffset + 12) == 255 && be16(p + mapOffset + 14) == 0 &&
        std::memcmp(p + glyOffset, "GLY1", 4) == 0 &&
        be32(p + glyOffset + 4) == textureBytes + 32 &&
        be16(p + glyOffset + 8) == 0 && be16(p + glyOffset + 10) == 255 &&
        be16(p + glyOffset + 12) == cell && be16(p + glyOffset + 14) == cell &&
        be32(p + glyOffset + 16) == textureBytes && be16(p + glyOffset + 20) == 0 &&
        be16(p + glyOffset + 22) == 16 && be16(p + glyOffset + 24) == 16 &&
        be16(p + glyOffset + 26) == side && be16(p + glyOffset + 28) == side;
}

inline bool supported(int code) {
    // CP1252 control slots (0x80..0x9F) are intentionally left to the game.
    return (code >= 0x20 && code <= 0x7e) || (code >= 0xa0 && code <= 0xff);
}

inline float advance(bool fixed, int fixedWidth, bool subsequent, int bearing,
    int width, float scale, int cellWidth) {
    const float units = scale / cellWidth;
    if (fixed) return fixedWidth * units;
    return (width + (subsequent ? 0 : bearing)) * units;
}

struct Placement { float x; float scaleX; };

inline Placement place(float x, float scaleX, int nativeCell, int nativeBearing,
    int nativeWidth, int newWidth, bool fixed, int fixedWidth, bool subsequent,
    float rasterScale = opticalScale) {
    const float available = (fixed ? fixedWidth : nativeWidth) * scaleX / nativeCell;
    const float natural = newWidth * scaleX * rasterScale / cell;
    const float fit = natural > 0 ? std::min(1.0f, available / natural) : 1.0f;
    const float indent = (fixed || !subsequent) ? nativeBearing * scaleX / nativeCell : 0;
    return {x + indent + (available - natural * fit) * 0.5f, scaleX * rasterScale * fit};
}

}  // namespace twilight_hd_hud::font_atlas
