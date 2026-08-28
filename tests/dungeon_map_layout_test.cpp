#include "dungeon_map_layout.hpp"
#include "map_responsive_layout.hpp"
#include "dungeon_map_input.hpp"
#include "menu_shortcuts.hpp"
#include <cassert>
#include <cmath>
#include <initializer_list>

using namespace twilight_hd_hud::dungeon_map_layout;
int main() {
    using namespace twilight_hd_hud;
    for (float width = 320; width <= 1064; width += 1) {
        const float left = (608 - width) / 2;
        const float size = map_responsive_layout::scale(width);
        assert(size > 0 && size <= 1);
        if (width < map_responsive_layout::referenceWidth) {
            const float frameLeft = map_responsive_layout::x(130, left, width);
            const float frameRight = frameLeft + 356 * size;
            const float equipmentLeft = left + width * .872f - 35 * size;
            const float floorLeft = map_responsive_layout::x(-68, left, width);
            const float floorRight = map_responsive_layout::x(120, left, width);
            assert(floorLeft > left && floorRight < frameLeft);
            assert(frameRight < equipmentLeft);
            assert(equipmentLeft + 70 * size < left + width);
            const auto hint = back_hint({frameLeft, map_responsive_layout::y(81, width),
                356 * size, 312 * size}, size);
            assert(hint.y + hint.height < band_top[1]);
            // A map-local marker and the same world position share one transform.
            const float mapLeft = map_responsive_layout::x(160, left, width);
            assert(std::abs(mapLeft + 74 * size - map_responsive_layout::x(234, left, width)) < .001f);
        } else {
            assert(size == 1); // no scaling change at 16:9 or wider
        }
    }
    // Authored center_n is centered inside mapspace, not ROOT.
    const Rect frame = frame_around({7, 6, 292, 300});
    assert(frame.x == -25 && frame.y == 0);
    assert(frame.width == 356 && frame.height == 312);
    // Parent pivot (-154,-158), translation (433,223), root shift (-124,16).
    assert(frame.x - 154 + 433 - 124 == 130);
    assert(frame.y - 158 + 223 + 16 == 81);
    for (float scale : {0.9f, 1.0f, 1.2f}) {
        const Rect row{20, 200, 138 * scale, 32 * scale};
        const Point tl = cursor_corner(row, 0);
        const Point bl = cursor_corner(row, 1);
        const Point tr = cursor_corner(row, 2);
        const Point br = cursor_corner(row, 3);
        assert(tl.x < row.x && tr.x > row.x + row.width);
        assert(tl.y == row.y + 1 && bl.y == row.y + row.height - 1);
        assert(tl.x == row.x - 3 && tr.x == row.x + row.width + 3);
        assert(tl.x == bl.x && tr.x == br.x && tr.y == tl.y && br.y == bl.y);
        // Repeated updates cannot accumulate padding or drift.
        assert(cursor_corner(row, 0).x == tl.x);
    }
    assert(prompt_icon(1).x < prompt_icon(0).x);
    assert(prompt_icon(1).y > prompt_icon(0).y);
    assert(banner.width == 236 && banner.height == 38);
    for (int tile = 0; tile < banner_motif_count; ++tile) {
        const auto motif = banner_motif(tile);
        assert(motif.width == 2 * motif.height); // Native crest is 128x64.
        assert(motif.x >= banner.x + 8);
        assert(motif.x + motif.width <= banner.x + banner.width - 8);
        assert(motif.y >= banner.y + 10); // Below the metallic top cap.
        assert(motif.y + motif.height <= banner.y + banner.height - 5);
        if (tile > 0) {
            const auto previous = banner_motif(tile - 1);
            assert(motif.x == previous.x + previous.width);
        }
    }
    assert(band_top[0] == 24 && band_top[1] + band_height == 431);
    const auto hint = back_hint({130, 81, 356, 312});
    assert(hint.x == 140 && hint.y == 397);
    assert(hint.y + hint.height < band_top[1]);
    const auto moved = back_hint({150, 85, 356, 312});
    assert(moved.x == hint.x + 20 && moved.y == hint.y + 4);

    using namespace twilight_hd_hud;
    constexpr unsigned up = 8, directions = 15;
    for (unsigned buttons = 0; buttons < 0x2000; ++buttons) {
        const auto filtered = dungeon_map_navigation_buttons(buttons, directions);
        assert((filtered & directions) == 0);
        assert((filtered & ~directions) == (buttons & ~directions));
        assert(restore_menu_shortcut_buttons(filtered, buttons, directions) == buttons);
        assert(dungeon_map_back_requested(true, false, false, buttons, up) == bool(buttons & up));
        assert(!dungeon_map_back_requested(false, false, false, buttons, up)); // opening/closing
        assert(!dungeon_map_back_requested(true, true, false, buttons, up)); // host settings
        assert(!dungeon_map_back_requested(true, false, true, buttons, up)); // loading/dialogue
    }
    // A held opening press has no new trigger: do not close or queue a close.
    assert(!dungeon_map_back_requested(true, false, false, 0, up));
}
