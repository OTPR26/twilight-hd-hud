#include "map_touch_portals.hpp"
#include "menu_shortcuts.hpp"

#include <cassert>
#include <cstdint>

int main() {
    twilight_hd_hud::MapTouchPortals touch;
    constexpr std::uint32_t portal = 0x10;
    constexpr std::uint32_t unrelated = 0x20;

    touch.observe(true, true, true);
    assert(touch.held() && touch.take_pressed());
    assert(!touch.take_pressed());
    assert(twilight_hd_hud::map_portal_buttons(unrelated, portal, touch.held()) ==
        (unrelated | portal));

    touch.observe(true, true, true);
    assert(touch.held() && !touch.take_pressed());
    touch.observe(false, false, true);
    assert(!touch.held() && !touch.take_pressed());
    touch.observe(true, true, true);
    assert(touch.take_pressed());

    touch.observe(true, false, true);  // Host blocked the virtual input.
    assert(!touch.held() && !touch.take_pressed());
    touch.observe(false, true, true);  // Physical L alone is not touch L.
    assert(!touch.held() && !touch.take_pressed());
    touch.observe(true, true, false);  // Dungeon map and other menus.
    assert(!touch.held() && !touch.take_pressed());

    touch.observe(true, true, true);
    touch.clear();
    assert(!touch.held() && !touch.take_pressed());
    assert(twilight_hd_hud::map_portal_buttons(unrelated | portal, portal, touch.held()) ==
        unrelated);
}
