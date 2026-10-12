#pragma once

#include <string>
#include <string_view>

namespace twilight_hd_hud {
inline std::string companion_item_help(std::string_view text) {
    // Limit English control names to descriptions containing English instructions.
    bool english = false;
    for (auto word : {"Set ", "set ", "Press ", "press ", "Use ", "use ", "Hold ", "hold ", "Wear ", "wear "})
        if (text.find(word) != std::string_view::npos) english = true;
    if (!english) return {};
    std::string result;
    bool changed = false;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\x02' || i + 1 >= text.size()) {
            result += text[i];
            continue;
        }
        const auto type = static_cast<unsigned char>(text[i + 1]) - 1;
        const char* label = nullptr;
        switch (type) {
        case 0: label = "the action button"; break;
        case 1: label = "the attack button"; break;
        case 2: case 9: case 69: label = "the right stick"; break;
        case 3: label = "the target button"; break;
        case 4: label = "the shoulder button"; break;
        case 5: case 6: case 7: label = "an item button"; break;
        case 8: label = "the D-pad"; break;
        case 43: label = ""; break; // Native continuation indentation; let the reader wrap it.
        }
        if (type == 5 || type == 6) {
            const auto pair = text.substr(i, 8);
            if (pair == "\x02\x06 or \x02\x07" || pair == "\x02\x07 or \x02\x06") {
                label = "any item slot";
                i += 6;
            }
        }
        if (label != nullptr) {
            result += label;
            ++i;
            changed = true;
        } else {
            result += text.substr(i, 2);
            ++i;
        }
    }
    std::string flowed;
    const auto space = [](char c) { return c == ' ' || c == '\n' || c == '\r' || c == '\t'; };
    for (std::size_t i = 0; i < result.size();) {
        if (result[i] == '\x02' && i + 1 < result.size()) {
            flowed += result.substr(i, 2);
            i += 2;
            continue;
        }
        if (!space(result[i])) {
            flowed += result[i++];
            continue;
        }
        bool lineBreak = false;
        while (i < result.size() && space(result[i])) {
            lineBreak |= result[i] == '\n' || result[i] == '\r';
            ++i;
        }
        if (flowed.empty() || i == result.size()) continue;
        const bool bullet = result[i] == '\x02' && i + 1 < result.size() &&
            result[i + 1] == 43;
        flowed += lineBreak && bullet ? '\n' : ' ';
    }
    result = std::move(flowed);
    struct Phrase { std::string_view from, to; };
    constexpr Phrase phrases[] = {
        {"Set this to and drink it with any item slot", "Equip this in any item slot, then drink it"},
        {"Set this to any item slot", "Equip this in any item slot"},
        {"Set it to any item slot", "Equip it in any item slot"},
        {"Lift one with any item slot", "Use the equipped item to lift one"},
        {"Wear them with any item slot", "Equip them in any item slot to wear them"},
    };
    for (const auto& phrase : phrases) {
        const auto at = result.find(phrase.from);
        if (at != std::string::npos) result.replace(at, phrase.from.size(), phrase.to);
    }
    return changed ? result : std::string{};
}
} // namespace twilight_hd_hud
