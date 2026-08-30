#include "overworld_map_layout.hpp"
#include "dungeon_map_layout.hpp"
#include "dungeon_map_input.hpp"
#include "menu_shortcuts.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
using namespace twilight_hd_hud::overworld_map_layout;
int main() {
    assert(content.x > frame.x && content.y > frame.y);
    assert(content.x + content.width < frame.x + frame.width);
    assert(content.y + content.height < frame.y + frame.height);
    assert(zoomBY + 8 < frame.y);
    assert(poeY > content.y && poeY < content.y + content.height);
    assert(poeTextYOffset == 2);
    assert(poeIconToText == 31);
    assert(portalY > content.y && portalY + 12 < content.y + content.height);
    for (float width = 320; width < 1064; width += 1) {
        const float left = (608 - width) / 2;
        const float size = twilight_hd_hud::map_responsive_layout::scale(width);
        const auto hint = back_hint(left, width);
        // 32px bounds cover the rendered English Back label at 12.5px.
        const float backRight = hint.x + (31 + 32) * size;
        const float portalLeft = safe_x(left, width, .181f) - 22 * size;
        assert(portalLeft - backRight > 7 * size);
        assert(hint.x >= left);
        assert(safe_x(left, width, .946f) + 14 * size < left + width);
        assert(safe_x(left, width, .05625f) + bannerWidth * size <
            safe_x(left, width, .946f) - 130 * size);
    }
    for (float width : {608.0f, 798.0f, 1064.0f}) {
        const float left = (608 - width) * 0.5f;
        assert(std::abs(safe_x(left, width, 0.5f) - 304) < .001f);
        assert(safe_x(left, width, .946f) > left + width * .86f);
        assert(safe_x(left, width, .05625f) + bannerWidth <
            safe_x(left, width, .78f));
        const auto hint = back_hint(left, width);
        const float size = twilight_hd_hud::map_responsive_layout::scale(width);
        assert(std::abs(hint.x + 12 * size - safe_x(left, width, .077f)) < .001f);
        assert(hint.y == 383 - 12 * size && hint.height == 24 * size);
        const auto dungeon = twilight_hd_hud::dungeon_map_layout::back_hint({0, 0, 356, 312});
        assert(hint.width == dungeon.width * size && hint.height == dungeon.height * size);
    }
    using namespace twilight_hd_hud;
    constexpr unsigned up = 8, leftButton = 1, directions = 15;
    for (unsigned buttons = 0; buttons < 0x2000; ++buttons) {
        const auto navigation = dungeon_map_navigation_buttons(buttons, directions);
        assert((navigation & directions) == 0); // no D-pad cursor movement
        assert((navigation & ~directions) == (buttons & ~directions)); // A/B/stick unchanged
        const bool close = dungeon_map_back_requested(true, false, false, buttons, up);
        const auto statusCheck = close ? navigation | leftButton : navigation;
        assert(bool(statusCheck & leftButton) == bool(buttons & up));
        const auto afterStatus = restore_menu_shortcut_buttons(statusCheck, navigation, leftButton);
        assert(afterStatus == navigation);
        assert(restore_menu_shortcut_buttons(afterStatus, buttons, directions) == buttons);
        assert(!dungeon_map_back_requested(false, false, false, buttons, up));
        assert(!dungeon_map_back_requested(true, true, false, buttons, up));
        assert(!dungeon_map_back_requested(true, false, true, buttons, up));
    }
    assert(!dungeon_map_back_requested(true, false, false, 0, up)); // held Up never repeats
    std::cout << "PASS: overworld frame/content bounds, prompt separation and wide-aspect anchors\n";
}
