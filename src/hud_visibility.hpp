#pragma once

#include <cstdint>

namespace twilight_hd_hud {

// Dusklight's touch overlay replaces these native gameplay prompts with its
// own controls. Keeping both presentations visible creates duplicate controls
// with conflicting placement, particularly around the hearts.
constexpr bool show_native_touch_replaced_hud(bool touchControlsActive) {
    return !touchControlsActive;
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
