#pragma once

#include "config.hpp"
#include <cstring>

namespace twilight_hd_hud {

constexpr bool uses_xbox_prompts(ButtonLayout layout) {
    return layout == ButtonLayout::Xbox || layout == ButtonLayout::BayxFlipped;
}

enum class ShoulderPrompt { L, R, Zl, Zr };

// Indices match ShoulderPrompt; rows match Silver and Black Pro.
inline constexpr const char* kXboxShoulderPaths[2][4] = {
    {"hud/shoulder-button-xbox-lb.bti", "hud/shoulder-button-xbox-rb.bti",
        "hud/shoulder-button-xbox-lt.bti", "hud/shoulder-button-xbox-rt.bti"},
    {"hud/shoulder-button-xbox-lb-black-pro.bti", "hud/shoulder-button-xbox-rb-black-pro.bti",
        "hud/shoulder-button-xbox-lt-black-pro.bti", "hud/shoulder-button-xbox-rt-black-pro.bti"},
};

constexpr const char* item_combo_button_label(ButtonLayout layout) {
    return uses_xbox_prompts(layout) ? "LT" :
        layout == ButtonLayout::PlayStation ? "L2" : "ZL";
}

template <typename TextBox>
void refresh_item_combo_label(TextBox* text, const char* current, ButtonLayout layout) {
    const char* label = item_combo_button_label(layout);
    // J2DTextBox ignores its varargs: it copies literal text. It also allocates
    // a new buffer, so only refresh when the selected label actually changes.
    if (text != nullptr && (current == nullptr || std::strcmp(current, label) != 0)) {
        text->setString(4, label);
    }
}

} // namespace twilight_hd_hud
