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
// uses BTN_Z. Input probes can also mark this bit before rejecting an action,
// so the native stage restriction must independently allow the selected item.
constexpr bool third_slot_item_usable(std::uint8_t selectedItem,
    std::uint8_t noItem, std::uint8_t useButtonFlags, std::uint8_t thirdSlotFlag,
    bool stageAllowsItem, bool actionAllowsItem) {
    return selectedItem != 0 && selectedItem != noItem &&
        stageAllowsItem && actionAllowsItem && (useButtonFlags & thirdSlotFlag) != 0;
}

// Detached combo artwork shares its primary layer's opacity. A still-nested
// layer must keep its local opacity because it already inherits the fade.
template <class Pane>
void sync_detached_item_alpha(Pane* primary, Pane* secondary) {
    if (primary == nullptr || secondary == nullptr ||
        primary->getParentPane() == nullptr ||
        primary->getParentPane() != secondary->getParentPane()) return;
    secondary->setAlpha(primary->getAlpha());
}

// The native rupee group already includes the configured HUD opacity and its
// transition fade. Copy that result, rather than multiplying the same parent
// opacity twice or restarting the game's animation. Visibility still belongs
// to the whole ancestor chain, even for the separately drawn number strip.
template <class Pane>
std::uint8_t rupee_counter_alpha(Pane* nativeGroup, bool overlayHidden) {
    if (nativeGroup == nullptr || overlayHidden) return 0;
    for (auto* pane = nativeGroup; pane != nullptr; pane = pane->getParentPane()) {
        if (!pane->isVisible() || pane->getAlpha() == 0) return 0;
    }
    return nativeGroup->getAlpha();
}

} // namespace twilight_hd_hud
