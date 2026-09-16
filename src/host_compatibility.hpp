#pragma once

#include <array>
#include <charconv>
#include <string_view>

namespace twilight_hd_hud {
inline constexpr const char* kHostRequirement =
    "Twilight HD requires Dusklight Nightly v1.4.1-402 or newer. "
    "Update Dusklight, then restart.";

inline bool supported_host_version(const char* version) {
    if (version == nullptr) return false;
    std::string_view text(version);
    if (text.starts_with('v')) text.remove_prefix(1);
    std::array<unsigned, 4> parts{};
    for (unsigned i = 0; i < parts.size(); ++i) {
        const char* begin = text.data();
        const char* end = begin + text.size();
        const auto parsed = std::from_chars(begin, end, parts[i]);
        if (parsed.ec != std::errc{} || parsed.ptr == begin) return false;
        text.remove_prefix(parsed.ptr - begin);
        if (text.empty()) {
            if (i < 2) return false;
            break;
        }
        if (i == 3 || (text.front() != '.' && !(i == 2 && text.front() == '-')))
            return false;
        text.remove_prefix(1);
    }
    return parts >= std::array<unsigned, 4>{1, 4, 1, 402};
}
} // namespace twilight_hd_hud
