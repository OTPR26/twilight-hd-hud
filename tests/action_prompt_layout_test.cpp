#include "action_prompt_layout.hpp"
#include "hud_scaling.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace twilight_hd_hud;
int main() {
    static_assert(action_prompt_layout::kBottleScale > 1.199f &&
        action_prompt_layout::kBottleScale < 1.201f);
    constexpr float nativeCanvas = 56.0f / action_prompt_layout::kFaceCoverage;
    for (float center : {250.0f, 960.0f, 1200.0f})
    for (float nativeText : {20.0f, 95.0f, 250.0f})
    for (int percent = 50; percent <= 125; ++percent) {
        const float width = nativeText * action_text_multiplier(hud_size_multiplier(percent));
        const auto row = action_prompt_layout::arrange(center, nativeCanvas, width);
        assert(std::abs(row.faceWidth - 46.666667f) < 0.001f);
        assert(std::abs(row.gap - 14.0f) < 0.001f);
        assert(row.textLeft > row.buttonCenter + row.faceWidth * 0.5f);
        assert(std::abs(row.textLeft - row.buttonCenter - row.faceWidth * 0.5f - row.gap) < 0.001f);
        assert(std::abs((row.buttonCenter - row.faceWidth * 0.5f + row.textLeft + width) * 0.5f - center) < 0.001f);
    }
    for (float center : {250.0f, 960.0f, 1200.0f})
    for (float nativeText : {20.0f, 95.0f, 250.0f}) {
        constexpr float nativeCanvas = 64.0f;
        const float width = nativeText * action_text_multiplier(1.0f);
        const float visible = nativeCanvas * action_prompt_layout::kTriggerScale *
            action_prompt_layout::kTriggerCoverage;
        const auto row = action_prompt_layout::arrange_visible(center, visible, width);
        assert(std::abs(row.faceWidth - 47.232f) < 0.001f);
        assert(row.textLeft > row.buttonCenter + row.faceWidth * 0.5f);
        assert(std::abs((row.buttonCenter - row.faceWidth * 0.5f +
            row.textLeft + width) * 0.5f - center) < 0.001f);
    }
    std::cout << "PASS: A on left, TPHD size/gap, centered variable-length labels at all scales\n";
}
