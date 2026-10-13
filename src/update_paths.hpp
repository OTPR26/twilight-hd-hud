#pragma once

#include <filesystem>
#include <vector>
#include <string_view>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

namespace twilight_hd_hud::update_paths {
namespace fs = std::filesystem;

inline fs::path from_utf8(std::string_view text) {
    return fs::path(std::u8string(text.begin(), text.end()));
}

inline fs::path executable() {
#if defined(_WIN32)
    std::vector<wchar_t> buffer(32768);
    const auto size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    return size != 0 && size < buffer.size() ? fs::path(std::wstring(buffer.data(), size)) : fs::path{};
#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::vector<char> buffer(size);
    return size != 0 && _NSGetExecutablePath(buffer.data(), &size) == 0
        ? from_utf8(buffer.data()) : fs::path{};
#elif defined(__linux__) && !defined(__ANDROID__)
    std::error_code ec;
    return fs::read_symlink("/proc/self/exe", ec);
#else
    return {};
#endif
}

inline std::vector<fs::path> roots(const fs::path& data, const fs::path& scratch,
    const fs::path& app) {
    std::vector<fs::path> result;
    if (!data.empty()) result.push_back(data.parent_path().parent_path() / "mods");
    // Desktop loader scratch paths retain the primary mods directory, including --mods.
    if (scratch.filename() == "data" && scratch.parent_path().filename() == "org.twilight.hd_hud" &&
        scratch.parent_path().parent_path().filename() == ".cache") {
        result.push_back(scratch.parent_path().parent_path().parent_path());
    }
    if (!app.empty()) {
        const auto folder = app.parent_path();
        if (folder.filename() == "MacOS" && folder.parent_path().filename() == "Contents") {
            result.push_back(folder.parent_path().parent_path().parent_path() / "mods");
            result.push_back(folder.parent_path() / "Resources/mods");
        } else {
            result.push_back(folder / "mods");
        }
    }
    return result;
}
}
