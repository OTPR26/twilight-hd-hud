#pragma once

#include <cstdint>

namespace twilight_hd_hud {

// Dusklight's touch overlay replaces these native gameplay prompts with its
// own controls. Keeping both presentations visible creates duplicate controls
// with conflicting placement, particularly around the hearts.
constexpr bool show_native_touch_replaced_hud(bool touchControlsActive) {
    return !touchControlsActive;
}

// Link's item-action scan marks each assignment bit only when that item can
// perform an action in the current room/state. The extended third assignment
// uses BTN_Z, so its HUD must follow that native result instead of treating
// every non-empty slot as usable.
constexpr bool third_slot_item_usable(std::uint8_t selectedItem,
    std::uint8_t noItem, std::uint8_t useButtonFlags, std::uint8_t thirdSlotFlag) {
    return selectedItem != 0 && selectedItem != noItem &&
        (useButtonFlags & thirdSlotFlag) != 0;
}

// The native rupee group already includes the configured HUD opacity and its
// transition fade. Copy that result, rather than multiplying the same parent
// opacity twice or restarting the game's animation. Visibility still belongs
// to the whole ancestor chain, even for our separately drawn number strip.
template <class Pane>
std::uint8_t rupee_counter_alpha(Pane* nativeGroup, bool overlayHidden) {
    if (nativeGroup == nullptr || overlayHidden) return 0;
    for (auto* pane = nativeGroup; pane != nullptr; pane = pane->getParentPane()) {
        if (!pane->isVisible() || pane->getAlpha() == 0) return 0;
    }
    return nativeGroup->getAlpha();
}

} // namespace twilight_hd_hud
