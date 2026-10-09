#pragma once

#include "config.hpp"
#include <cstring>

namespace twilight_hd_hud {

constexpr bool uses_xbox_prompts(ButtonLayout layout) {
    return layout == ButtonLayout::Xbox || layout == ButtonLayout::BayxFlipped ||
        layout == ButtonLayout::XboxBotw || layout == ButtonLayout::BayxFlippedBotw;
}

// Native action identifiers: A=Action, B=Attack, X/Y=item slots. This is
// presentation only: the player configures the corresponding binds in Dusklight.
constexpr char face_position_for_action(ButtonLayout layout, char action) {
    if (layout == ButtonLayout::PlayStationFlipped && (action == 'X' || action == 'Y'))
        return action == 'X' ? 'Y' : 'X';
    if (layout == ButtonLayout::PlayStationSwapped || layout == ButtonLayout::PlayStationFlipped)
        return action == 'A' ? 'B' : action == 'B' ? 'A' : action;
    if (!is_botw_layout(layout)) return action;
    if (has_flipped_botw_items(layout)) {
        if (action == 'X') return 'A'; // East item
        if (action == 'Y') return 'X'; // North item
    }
    return action == 'A' ? 'B' : action == 'B' ? 'Y' : action == 'Y' ? 'A' : action;
}

constexpr char face_letter_for_action(ButtonLayout layout, char action) {
    const char position = face_position_for_action(layout, action);
    // Preserve the established flipped preset's menu/instruction labels.
    if (layout == ButtonLayout::Xbox || layout == ButtonLayout::XboxBotw ||
        layout == ButtonLayout::BayxFlippedBotw || layout == ButtonLayout::SteamDeckBotw)
        return position == 'A' ? 'B' : position == 'B' ? 'A' :
            position == 'X' ? 'Y' : position == 'Y' ? 'X' : position;
    return position;
}

constexpr int companion_face_symbol_index(ButtonLayout layout, char action) {
    const char position = face_position_for_action(layout, action);
    return position == 'A' ? 0 : position == 'B' ? 1 : position == 'X' ? 2 : 3;
}

constexpr const char* companion_item_label(ButtonLayout layout, char action) {
    if (is_playstation_layout(layout)) {
        switch (companion_face_symbol_index(layout, action)) {
        case 0: return "Circle";
        case 1: return "Cross";
        case 2: return "Triangle";
        default: return "Square";
        }
    }
    switch (face_letter_for_action(layout, action)) {
    case 'A': return "A";
    case 'B': return "B";
    case 'X': return "X";
    default: return "Y";
    }
}

constexpr const char* companion_shoulder_label(ButtonLayout layout, bool left) {
    if (uses_xbox_prompts(layout)) return left ? "LB" : "RB";
    if (is_playstation_layout(layout) || is_steam_deck_layout(layout))
        return left ? "L1" : "R1";
    return left ? "L" : "R";
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
        (is_playstation_layout(layout) || is_steam_deck_layout(layout)) ? "L2" : "ZL";
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
