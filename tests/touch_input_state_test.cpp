#include "touch_input_state.hpp"

#include <cassert>
#include <cstdint>

int main() {
    constexpr std::uint32_t items = 1u << 0;
    constexpr std::uint32_t collections = 1u << 1;
    constexpr std::uint32_t z = 1u << 2;
    constexpr std::uint32_t l = 1u << 3;
    constexpr std::uint32_t r = 1u << 4;

    twilight_hd_hud::TouchInputState state;
    state.update(items, items, collections, z, l, r);
    assert(state.items_held());
    assert(state.items_triggered());
    assert(!state.z_held());

    state.update(items, items, collections, z, l, r);
    assert(state.items_held());
    assert(!state.items_triggered());

    state.update(items | z, items, collections, z, l, r);
    assert(state.z_held());
    assert(state.z_triggered());

    state.update(collections | z | l | r, items, collections, z, l, r);
    assert(!state.items_held());
    assert(state.collections_held());
    assert(state.collections_triggered());
    assert(state.z_held());
    assert(!state.z_triggered());
    assert(state.l_held());
    assert(state.l_triggered());
    assert(state.r_held());
    assert(state.r_triggered());

    state.update(collections | l | r, items, collections, z, l, r);
    assert(state.collections_held());
    assert(!state.collections_triggered());
    assert(!state.z_held());
    assert(state.l_held());
    assert(!state.l_triggered());
    assert(state.r_held());
    assert(!state.r_triggered());

    state.clear();
    assert(!state.items_held());
    assert(!state.items_triggered());
    assert(!state.collections_held());
    assert(!state.collections_triggered());
    assert(!state.z_held());
    assert(!state.z_triggered());
    assert(!state.l_held());
    assert(!state.l_triggered());
    assert(!state.r_held());
    assert(!state.r_triggered());
}
