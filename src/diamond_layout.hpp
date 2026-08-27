#pragma once

#include <algorithm>
#include <array>
#include <cmath>

namespace twilight_hd_hud {

// HUD ancestors are axis-aligned. Move independent subtrees once; nested
// labels/art inherit the change. Convert world deltas to parent-local units.
template <typename Pane, std::size_t N>
void offset_diamond_group(const std::array<Pane*, N>& panes, float dx, float dy) {
    for (Pane* pane : panes) {
        if (pane == nullptr) continue;
        bool inherited = false;
        float parentScaleX = 1.0f;
        float parentScaleY = 1.0f;
        for (Pane* parent = pane->getParentPane(); parent != nullptr;
             parent = parent->getParentPane()) {
            if (std::find(panes.begin(), panes.end(), parent) != panes.end()) inherited = true;
            parentScaleX *= parent->getScaleX();
            parentScaleY *= parent->getScaleY();
        }
        if (!inherited && std::fabs(parentScaleX) > 0.001f &&
            std::fabs(parentScaleY) > 0.001f) {
            pane->add(dx / parentScaleX, dy / parentScaleY);
        }
    }
}

}  // namespace twilight_hd_hud
