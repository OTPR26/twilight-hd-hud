#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

namespace twilight_hd_hud {

// Native inventory slot IDs, not item IDs or the wheel's compacted indices.
// The main bank matches TPHD's three rows of seven. Extra quest slots appear
// below it only when occupied, without moving the main items.
inline constexpr int kBankColumns = 7;
inline constexpr int kBankRows = 3;
inline constexpr int kBankOoccooCell = 20;
inline constexpr std::array<unsigned char, 24> kBankSlots = {
    20, 23, 1, 0, 17, 11, 12,
    3, 4, 9, 2, 16, 13, 14,
    6, 8, 22, 5, 15, 21, 18,
    19, 7, 10,
};
static_assert(kBankSlots[kBankOoccooCell] == 18); // Native Ooccoo inventory slot.

inline constexpr int bank_cell(unsigned slot, bool singleClawshotPresent = false) {
    // The double clawshot upgrades the existing cell. Retain an overflow cell
    // if an unusual inventory actually contains both versions.
    if (slot == 10 && !singleClawshotPresent) return 9;
    for (std::size_t i = 0; i < kBankSlots.size(); ++i)
        if (kBankSlots[i] == slot) return static_cast<int>(i);
    return -1;
}
inline constexpr float bank_x(int cell) {
    return cell < 21 ? 146.0f + (cell % 7) * 84.0f : 506.0f + (cell - 21) * 66.0f;
}
inline constexpr float bank_y(int cell) {
    return cell < 21 ? 190.0f + (cell / 7) * 84.0f : 416.0f;
}
inline constexpr float bank_cell_size(int cell) {
    return cell < 21 ? 54.0f : 32.0f;
}
inline constexpr float bank_cursor_half_extent(int cell, float scale, float phase) {
    const float pulse = 0.96f + 0.04f * (phase < 10.0f ? phase / 10.0f : (20.0f - phase) / 10.0f);
    // The corner artwork itself extends beyond its origin.
    return (bank_cell_size(cell) * 0.5f - 2.0f) * scale * pulse;
}
inline constexpr float bank_icon_extent(unsigned slot, int cell, bool selected) {
    if (cell >= 21) return 30.0f;
    // Long tools cross the frame, while bottles and compact items stay inside.
    const bool oversized = slot == 20 || slot == 1 || slot == 0 ||
        slot == 9 || slot == 10 || slot == 6 || slot == 8;
    return (oversized ? 64.0f : 51.0f) + (selected ? 2.0f : 0.0f);
}
inline constexpr float kBankComboX = 119.0f;
inline constexpr float kBankComboY = 403.0f;
inline constexpr unsigned char bank_combo_button_alpha(bool prompts, bool available) {
    return !prompts ? 0 : available ? 255 : 95;
}
enum class BankDirection { Left, Right, Up, Down };

inline int bank_neighbor(int current, BankDirection direction,
    const std::array<bool, 24>& occupied) {
    if (current < 0 || current >= 24) return current;
    const bool horizontal = direction == BankDirection::Left || direction == BankDirection::Right;
    const int sign = direction == BankDirection::Left || direction == BankDirection::Up ? -1 : 1;
    float bestScore = std::numeric_limits<float>::max();
    int best = current;
    for (int i = 0; i < 24; ++i) {
        if (!occupied[i] || i == current) continue;
        const float dx = bank_x(i) - bank_x(current);
        const float dy = bank_y(i) - bank_y(current);
        const float forward = (horizontal ? dx : dy) * sign;
        const float sideways = std::fabs(horizontal ? dy : dx);
        if (forward <= 0) continue;
        // Left/right stays in the row. Up/down visits the nearest occupied
        // row, then its closest column. Preferring a distant aligned column
        // can make a sparse intermediate row completely unreachable.
        if (horizontal && sideways > 0.5f) continue;
        const float score = horizontal ? forward : forward * 1000.0f + sideways;
        if (score < bestScore) { bestScore = score; best = i; }
    }
    return best;
}

// Convert a visual cell back to the existing wheel entry. Never reorder the
// inventory or hand a blank bank cell to assignment/combination code.
template <std::size_t N>
inline int bank_entry(int cell, const unsigned char (&slots)[N], unsigned count) {
    if (cell < 0 || cell >= 24 || count > N) return -1;
    bool singleClawshotPresent = false;
    for (unsigned i = 0; i < count; ++i) singleClawshotPresent |= slots[i] == 9;
    for (unsigned i = 0; i < count; ++i)
        if (bank_cell(slots[i], singleClawshotPresent) == cell) return static_cast<int>(i);
    return -1;
}
} // namespace twilight_hd_hud
