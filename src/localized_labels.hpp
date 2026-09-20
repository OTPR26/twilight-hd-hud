#pragma once

#include <string>
#include <string_view>

namespace twilight_hd_hud {

// Message IDs are shared by the game's language archives. Read their native
// encoding rather than inserting UTF-8 into a legacy game-font text box.
enum class MenuLabel : unsigned {
    Options = 0x5f, Save = 0x60, Items = 0x61, Minimap = 0x62,
    Assign = 0x380, Collection = 0x3e1, Back = 0x3f9, Confirm = 0x40c,
    Letters = 0x4d6, FishJournal = 0x5a1, GoldenBugs = 0x5ba, Skills = 0x6a4,
};

inline std::string single_line_label(std::string_view text) {
    std::string result;
    for (char c : text) {
        if (c == '\r' || c == '\n' || c == '\t') c = ' ';
        if (c == ' ' && (result.empty() || result.back() == ' ')) continue;
        result += c;
    }
    while (!result.empty() && result.back() == ' ') result.pop_back();
    return result;
}

inline std::string collection_shortcut_label(std::string_view collection, std::string_view save) {
    return single_line_label(collection) + "/\n" + single_line_label(save);
}

} // namespace twilight_hd_hud
