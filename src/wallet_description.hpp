#pragma once

#include <string>
#include <string_view>

namespace twilight_hd_hud {

// The caller scopes this to the Collection wallet description and supplies the
// remaining BMG DAT1 bytes. Keep every word and control tag; only the capacity
// changes. An empty result means no safe edit is needed or possible.
inline std::string wallet_description_capacity(std::string_view data, unsigned capacity) {
    if (capacity == 0 || capacity > 65535) return {};
    constexpr std::size_t limit = 1024;
    std::size_t length = 0;
    while (length < data.size() && length < limit && data[length] != '\0') {
        std::size_t step = 1;
        if (data[length] == '\x1a') {
            if (length + 1 >= data.size()) return {};
            step = static_cast<unsigned char>(data[length + 1]);
            if (step < 5) return {};
        }
        if (step > data.size() - length || step > limit - length) return {};
        length += step;
    }
    if (length >= data.size() || length >= limit) return {};
    const auto message = data.substr(0, length);
    const auto digit = [](char c) { return c >= '0' && c <= '9'; };
    std::size_t amountStart = std::string_view::npos, amountEnd = 0;
    unsigned original = 0;
    char grouping = 0;
    for (std::size_t at = 0; at < length;) {
        if (message[at] == '\x1a') {
            at += static_cast<unsigned char>(message[at + 1]);
            continue;
        }
        if (!digit(message[at])) { ++at; continue; }
        const auto start = at;
        unsigned value = 0;
        while (at < length && digit(message[at])) {
            value = value > 65535 ? 65536 : value * 10 + (message[at] - '0');
            ++at;
        }
        char separator = 0;
        // Preserve existing locale grouping, including a prepatched ISO's
        // "1,000", "1.000", "1 000", or single-byte nonbreaking space.
        if (at - start <= 2 && at + 3 < length &&
            (message[at] == ',' || message[at] == '.' || message[at] == ' ' ||
             static_cast<unsigned char>(message[at]) == 0xa0) &&
            digit(message[at + 1]) && digit(message[at + 2]) && digit(message[at + 3]) &&
            (at + 4 == length || !digit(message[at + 4]))) {
            separator = message[at++];
            for (int i = 0; i < 3; ++i) value = value * 10 + (message[at++] - '0');
        }
        if (value != 300 && value != 500 && value != 600 && value != 1000 && value != 2000)
            continue;
        if (amountStart != std::string_view::npos) return {}; // Ambiguous custom wording.
        amountStart = start;
        amountEnd = at;
        original = value;
        grouping = separator;
    }
    if (amountStart == std::string_view::npos || original == capacity) return {};
    auto amount = std::to_string(capacity);
    if (grouping != 0 && amount.size() > 3) amount.insert(amount.size() - 3, 1, grouping);
    auto result = std::string(message);
    result.replace(amountStart, amountEnd - amountStart, amount);
    return result;
}

} // namespace twilight_hd_hud
