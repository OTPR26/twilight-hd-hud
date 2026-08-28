#include "collection_layout.hpp"

#include <cassert>
#include <iostream>
#include <queue>

using namespace twilight_hd_hud;

int main() {
    const std::string card = "Press \x1b" "CR[14] to open";
    assert(collection_icon_slots_match(card, card, 1, 1));
    assert(collection_icon_slots_match(card, card, 2, 2));
    assert(!collection_icon_slots_match(card, "New selection", 1, 1));
    assert(!collection_icon_slots_match(card, card, 2, 1));
    assert(!collection_icon_slots_match(card, card, 1, 2));
    assert(!collection_icon_slots_match(card, card, 0, 0));
    assert(!collection_icon_slots_match("", "", 1, 1));
    assert(!collection_icon_slots_match(card, card, 36, 36));
    assert(collection_inline_icon_x(50, true) == 48);
    assert(collection_inline_icon_x(50, false) == 50.5f);
    assert(collection_inline_icon_x(0, true) == 0);
    assert(collection_inline_icon_x(1, true) == 0);
    assert(kCollectionIconSpacing == 1.0f);
    assert(kCollectionPromptCenterY == 48.0f);
    auto prefix = collection_text_prefix("Longer previous line\nSwing with ");
    assert(prefix.line == "Swing with ");
    assert(prefix.format.empty());
    prefix = collection_text_prefix("\x1b" "FX[11]Title\nSwing with \x1b" "CR[16] and sheathe with ");
    assert(prefix.line == "\x1b" "FX[11]Swing with \x1b" "CR[16] and sheathe with ");
    assert(prefix.format == "\x1b" "FX[11]");
    prefix = collection_text_prefix("\x1b" "FX[11]First\n\x1b" "SH[1]Second\nThird");
    assert(prefix.line == "\x1b" "FX[11]\x1b" "SH[1]Third");
    assert(collection_text_prefix("").line.empty());
    assert(collection_model_angle(-0x5b1c, false) == -32768);
    assert(collection_model_angle(-0x510c, true) == -32768);
    for (bool wolf : {false, true}) {
        for (int angle = -32768; angle < 32768; ++angle) {
            const int next = angle == 32767 ? -32768 : angle + 1;
            const int currentOutput = collection_model_angle(static_cast<std::int16_t>(angle), wolf);
            const int nextOutput = collection_model_angle(static_cast<std::int16_t>(next), wolf);
            assert(nextOutput - currentOutput == 1 || nextOutput - currentOutput == -65535);
        }
    }
    assert(collection_inline_icon_scale(3) == 2.0f);
    for (unsigned type : {0, 1, 2, 4, 5, 6, 7, 8}) assert(collection_inline_icon_scale(type) == 1.25f);
    for (unsigned type : {9, 30, 47, 69}) assert(collection_inline_icon_scale(type) == 1.0f);
    for (float scale : {0.75f, 1.0f, 1.5f}) {
        for (float width : {28.0f, 52.0f, 156.0f}) {
            const float extent = collection_cursor_half_extent(width * scale, scale);
            assert(extent > 0);
            assert(std::fabs(extent - (width * 0.5f - 2.0f) * scale) < 0.001f);
        }
    }
    assert(collection_cursor_half_extent(0, 1) == 1);
    for (float y : {0.0f, 80.0f, 200.0f, 405.0f, 448.0f}) {
        // Native helper adds 100 before normalizing; a zero-origin viewport
        // must project back to the requested screen position.
        assert(collection_projection_y(y) + 100.0f == y);
    }
    assert(collection_mirror_projection_call(600.0f, false));
    assert(!collection_mirror_projection_call(600.0f, true)); // scaled Link
    for (float depth : {400.0f, 450.0f, 550.0f, 580.0f}) {
        assert(!collection_mirror_projection_call(depth, false));
    }
    for (bool active : {false, true}) {
        for (bool hasModel : {false, true}) {
            for (bool helperRan : {false, true}) {
                const bool fallback = collection_mirror_needs_correction(active, hasModel, helperRan);
                assert(fallback == (active && hasModel && !helperRan));
                if (!active || !hasModel) continue;
                // Android's inlined move and Mac's hooked call must apply
                // exactly one correction on every newly computed frame.
                for (float y : {0.0f, 80.0f, 200.0f, 405.0f}) {
                    float result = helperRan ? collection_projection_y(y) : y;
                    if (fallback) result += collection_projection_y(0.0f);
                    assert(result == y - 100.0f);
                }
            }
        }
    }
    std::array<bool, kCollectionCells.size()> available{};
    available.fill(true);
    const auto right = CollectionDirection::Right;
    const auto left = CollectionDirection::Left;
    const auto up = CollectionDirection::Up;
    const auto down = CollectionDirection::Down;
    assert(collection_neighbor(collection_cell_index(4, 2), right, available) == collection_cell_index(3, 2));
    assert(collection_neighbor(collection_cell_index(3, 1), right, available) == collection_cell_index(4, 1));
    assert(collection_neighbor(collection_cell_index(3, 0), right, available) == collection_cell_index(4, 0));
    assert(collection_neighbor(collection_cell_index(0, 5), right, available) == collection_cell_index(1, 5));
    assert(collection_neighbor(-1, down, available) == -1);

    const int save = collection_cell_index(0, 5);
    const int heart = collection_cell_index(5, 0);
    const int lowerItem = collection_cell_index(3, 3); // second row, nearest Save's center
    const int upperItem = collection_cell_index(0, 4); // first row, nearest heart's center
    const int mirror = collection_cell_index(6, 0);
    const int options = collection_cell_index(1, 5);
    assert(collection_neighbor(save, up, available) == lowerItem);
    assert(collection_neighbor(lowerItem, up, available) == upperItem);
    assert(collection_neighbor(upperItem, up, available) == heart);
    assert(collection_neighbor(heart, down, available) == upperItem);
    assert(collection_neighbor(upperItem, down, available) == lowerItem);
    assert(collection_neighbor(lowerItem, down, available) == save);
    assert(collection_neighbor(options, up, available) == mirror);
    assert(collection_neighbor(mirror, down, available) == options);

    // Every subset of the eight collectibles: never skip an available row
    // between the broad heart/Save targets, even when only an outer cell exists.
    for (unsigned mask = 0; mask < 256; ++mask) {
        available.fill(true);
        for (int i = 0; i < 8; ++i) available[9 + i] = (mask & (1u << i)) != 0;
        const int fromSave = collection_neighbor(save, up, available);
        const int fromHeart = collection_neighbor(heart, down, available);
        const float expectedUp = (mask & 0xf0) ? 370 : (mask & 0x0f) ? 310 : 205;
        const float expectedDown = (mask & 0x0f) ? 310 : (mask & 0xf0) ? 370 : 416;
        assert(kCollectionCells[fromSave].y == expectedUp);
        assert(kCollectionCells[fromHeart].y == expectedDown);
        assert(available[fromSave] && available[fromHeart]);
        assert(collection_neighbor(save, right, available) == options);
        assert(collection_neighbor(options, left, available) == save);
    }

    // Every unlocked cell must remain reachable, including when any single
    // equipment slot/collectible is absent. Also exercise sparse early saves.
    for (int missing = -2; missing < static_cast<int>(available.size()); ++missing) {
        available.fill(true);
        if (missing >= 0) available[missing] = false;
        if (missing == -2) {
            available.fill(false);
            for (int i : {7, 9, 17, 18}) available[i] = true;
        }
        for (int start = 0; start < static_cast<int>(available.size()); ++start) {
            if (!available[start]) continue;
            std::array<bool, kCollectionCells.size()> visited{};
            std::queue<int> pending;
            visited[start] = true;
            pending.push(start);
            while (!pending.empty()) {
                const int current = pending.front();
                pending.pop();
                for (auto direction : {left, right, up, down}) {
                    const int next = collection_neighbor(current, direction, available);
                    assert(next >= 0 && next < static_cast<int>(available.size()));
                    assert(available[next]);
                    if (!visited[next]) { visited[next] = true; pending.push(next); }
                }
            }
            for (std::size_t i = 0; i < available.size(); ++i) assert(!available[i] || visited[i]);
        }
    }
    for (float width : {608.0f, 796.0f, 1060.0f}) {
        const auto viewport = collection_viewport(-30, 8, width - 30, 456);
        assert(viewport.scale > 0);
        assert(viewport.left >= -30.001f && viewport.top >= 7.999f);
        assert(viewport.left + 796 * viewport.scale <= width - 30 + 0.001f);
        assert(viewport.top + 448 * viewport.scale <= 456.001f);
    }
    std::cout << "PASS: Collection navigation, missing items and aspect-ratio bounds\n";
}
