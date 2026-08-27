#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace twilight_hd_hud {

// BMG tags contain zero bytes, so a message cannot be read with strlen.
// The caller supplies the remaining DAT1 block as a hard read boundary.
inline std::string three_button_item_help(std::string_view data) {
    constexpr std::size_t maxMessageBytes = 1024;
    std::size_t length = 0;
    while (length < data.size() && length < maxMessageBytes && data[length] != '\0') {
        std::size_t step = 1;
        if (data[length] == '\x1a') {
            if (length + 1 >= data.size()) return {};
            step = static_cast<unsigned char>(data[length + 1]);
            if (step < 5) return {};
        }
        if (step > data.size() - length || step > maxMessageBytes - length) return {};
        length += step;
    }
    if (length >= data.size() || length >= maxMessageBytes) return {};
    const auto message = data.substr(0, length);
    const auto itemButton = [&](std::size_t at) -> unsigned char {
        if (at + 5 > message.size() || message[at] != '\x1a' ||
            message[at + 2] != 0 || message[at + 3] != 0) return 0;
        const auto id = static_cast<unsigned char>(message[at + 4]);
        return id == 15 || id == 16 || id == 46 || id == 47 ? id : 0;
    };
    const auto space = [](char ch) { return ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t'; };
    std::string result;
    bool changed = false;
    for (std::size_t at = 0; at < message.size();) {
        const auto first = itemButton(at);
        const auto step = message[at] == '\x1a' ?
            static_cast<unsigned char>(message[at + 1]) : 1;
        auto next = at + step;
        const auto separator = next;
        while (next < message.size() && space(message[next])) ++next;
        const auto conjunction = next;
        if (first && next > separator && message.substr(next, 2) == "or") {
            next += 2;
            const auto afterOr = next;
            while (next < message.size() && space(message[next])) ++next;
            const auto second = itemButton(next);
            const bool pair = (first == 15 && second == 16) || (first == 16 && second == 15) ||
                (first == 46 && second == 47) || (first == 47 && second == 46);
            if (next > afterOr && pair) {
                // Fixed native Y/X/Z tags use the existing layout-aware art.
                // Preserve any authored line break between the original icons.
                result.append("\x1a\x05\x00\x00\x10", 5);
                result += ',';
                auto gap = std::string(message.substr(separator, conjunction - separator));
                gap += message.substr(afterOr, next - afterOr);
                const auto newline = gap.find('\n');
                result += newline == std::string::npos ? " " : gap.substr(newline);
                result.append("\x1a\x05\x00\x00\x0f", 5);
                result += " or ";
                result.append("\x1a\x05\x00\x00\x11", 5);
                at = next + static_cast<unsigned char>(message[next + 1]);
                changed = true;
                continue;
            }
        }
        result += message.substr(at, step);
        at += step;
    }
    if (!changed) return {};

    // Retain the already-tested shorter Clawshot wording.
    constexpr std::string_view longAim = ", aim at places where";
    if (const auto aim = result.find(longAim); aim != std::string::npos) {
        result.replace(aim, longAim.size(), ", aim where");
    }
    return result;
}

inline float item_help_fit_scale(float availableWidth, float measuredWidth) {
    if (!(availableWidth > 0.0f) || !(measuredWidth > availableWidth)) return 1.0f;
    return availableWidth / measuredWidth;
}

} // namespace twilight_hd_hud
