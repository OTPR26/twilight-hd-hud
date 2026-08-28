#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>

namespace twilight_hd_hud {

// Keep native item IDs: equipment actions and submenus still use the original
// grid coordinates. Only presentation and directional navigation change.
struct CollectionCell {
    int column;
    int row;
    float x;
    float y;
    float width;
};

inline constexpr std::array<CollectionCell, 19> kCollectionCells = {{
    {4, 2, 338, 80, 46}, {3, 2, 396, 80, 46}, {5, 2, 454, 80, 46},
    {3, 1, 260, 145, 46}, {4, 1, 320, 145, 46},
    {3, 0, 478, 145, 46}, {4, 0, 538, 145, 46},
    {5, 0, 158, 205, 112}, {6, 0, 655, 200, 80},
    {0, 3, 75, 310, 44}, {1, 3, 130, 310, 44},
    {0, 4, 185, 310, 44}, {1, 4, 240, 310, 44},
    {2, 3, 75, 370, 44}, {2, 4, 130, 370, 44},
    {3, 3, 185, 370, 44}, {3, 4, 240, 370, 44},
    {0, 5, 158, 416, 156}, {1, 5, 638, 416, 156},
}};

inline int collection_cell_index(int column, int row) {
    for (std::size_t i = 0; i < kCollectionCells.size(); ++i) {
        if (kCollectionCells[i].column == column && kCollectionCells[i].row == row)
            return static_cast<int>(i);
    }
    return -1;
}

enum class CollectionDirection { Left, Right, Up, Down };

inline int collection_neighbor(int current, CollectionDirection direction,
    const std::array<bool, kCollectionCells.size()>& available) {
    if (current < 0 || current >= static_cast<int>(kCollectionCells.size())) return current;
    const auto& origin = kCollectionCells[current];
    int best = current;
    float bestScore = std::numeric_limits<float>::max();
    const bool horizontal = direction == CollectionDirection::Left ||
        direction == CollectionDirection::Right;
    const float sign = direction == CollectionDirection::Left ||
        direction == CollectionDirection::Up ? -1.0f : 1.0f;
    for (std::size_t i = 0; i < kCollectionCells.size(); ++i) {
        if (!available[i] || static_cast<int>(i) == current) continue;
        const float dx = kCollectionCells[i].x - origin.x;
        const float dy = kCollectionCells[i].y - origin.y;
        const float forward = (horizontal ? dx : dy) * sign;
        const float sideways = std::fabs(horizontal ? dy : dx);
        if (forward < 1.0f) continue;
        // Prefer the same visual row/column without trapping the cursor when
        // an item is missing or crossing between equipment and collectibles.
        const float score = forward + 3.0f * sideways +
            (sideways >= 1.0f ? 10000.0f : 0.0f) +
            (sideways > forward ? 10000.0f : 0.0f);
        if (score < bestScore) {
            bestScore = score;
            best = static_cast<int>(i);
        }
    }
    return best;
}

struct CollectionViewport {
    float left;
    float top;
    float scale;
};

inline float collection_cursor_half_extent(float renderedSize, float scale) {
    // The bracket artwork already extends outward from its pane origin.
    return std::fmax(1.0f, renderedSize * 0.5f - 2.0f * scale);
}

inline constexpr std::int16_t collection_model_angle(std::int16_t nativeAngle, bool wolf) {
    const int nativeRest = wolf ? -0x510c : -0x5b1c;
    int angle = static_cast<int>(nativeAngle) - nativeRest - 32768;
    if (angle < -32768) angle += 65536;
    if (angle > 32767) angle -= 65536;
    return static_cast<std::int16_t>(angle);
}

inline constexpr float collection_inline_icon_scale(unsigned type) {
    // The replacement target/trigger cap occupies half its square texture.
    // Other controller icons need only a small increase beside the body font.
    return type == 3 ? 2.0f : type <= 8 ? 1.25f : 1.0f;
}

inline constexpr float kCollectionIconSpacing = 1.0f;
inline constexpr float kCollectionPromptCenterY = 48.0f;

inline bool collection_icon_slots_match(std::string_view drawn, std::string_view current,
    std::size_t slotCount, std::size_t iconCount) {
    return !drawn.empty() && drawn == current && slotCount > 0 &&
        slotCount <= 35 && slotCount == iconCount;
}

inline float collection_inline_icon_x(float prefixWidth, bool followsSpace) {
    // Authored spaces plus the texture's transparent rim made the leading gap
    // too wide. Use part of that space without touching unspaced text.
    return std::fmax(0.0f, prefixWidth + (followsSpace ? -2.0f : 0.5f));
}

struct CollectionTextPrefix {
    std::string line;
    std::string format;
};

inline CollectionTextPrefix collection_text_prefix(std::string_view text) {
    CollectionTextPrefix result;
    for (std::size_t i = 0; i < text.size();) {
        if (text[i] == '\n') {
            result.line = result.format;
            ++i;
        } else if (text[i] == '\x1b' && i + 3 < text.size() && text[i + 3] == '[') {
            const auto end = text.find(']', i + 4);
            if (end == std::string_view::npos) break;
            const auto code = text.substr(i + 1, 2);
            const auto escape = text.substr(i, end + 1 - i);
            result.line.append(escape);
            if (code == "FX" || code == "FY" || code == "SH" || code == "SV")
                result.format.append(escape);
            i = end + 1;
        } else {
            result.line += text[i++];
        }
    }
    return result;
}

// The native projection helper assumes a viewport whose top is -100. The
// Collection screen uses the full-height viewport at zero instead. Adjust
// coordinates at the helper boundary, for both Link and the mirror/crystal.
inline constexpr float collection_projection_y(float screenY) {
    return screenY - 100.0f;
}

// The mirror/crystal has a fixed camera depth of 600; Link's explicit mod
// placement can also reach that depth when scaled, so exclude its output.
inline constexpr bool collection_mirror_projection_call(float depth, bool explicitLink) {
    return depth == 600.0f && !explicitLink;
}

inline constexpr bool collection_mirror_needs_correction(bool active, bool hasModel,
    bool nativeHelperCorrected) {
    return active && hasModel && !nativeHelperCorrected;
}

inline CollectionViewport collection_viewport(float left, float top, float right, float bottom) {
    const float scale = std::fmin((right - left) / 796.0f, (bottom - top) / 448.0f);
    return {(left + right - 796.0f * scale) * 0.5f,
        (top + bottom - 448.0f * scale) * 0.5f, scale};
}

} // namespace twilight_hd_hud
