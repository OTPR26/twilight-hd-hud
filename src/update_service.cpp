#include "update_service.hpp"

#include "config.hpp"
#include "service_imports.hpp"

#ifdef _WIN32
#include <windows.h>
#include <wininet.h>
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#include <array>
#include <atomic>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

namespace twilight_hd_hud {
namespace {

constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/OTPR26/twilight-hd-hud/releases/latest";

#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
constexpr const char* kReleaseAsset = "Twilight-HD-HUD-iOS-tvOS.dusk";
#else
constexpr const char* kReleaseAsset = "Twilight-HD-HUD-Desktop-Android.dusk";
#endif

enum class State { Idle, Checking, CheckReady, Downloading, DownloadReady };

struct Result {
    bool succeeded = false;
    bool updateAvailable = false;
    bool manual = false;
    bool installSucceeded = false;
    std::string currentVersion;
    std::string latestVersion;
    std::string downloadUrl;
    std::string message;
};

std::atomic<State> s_state{State::Idle};
std::mutex s_mutex;
Result s_result;
std::thread s_worker;

void on_check_setting_changed(ModContext*, ConfigVarHandle,
    const ConfigVarValue* value, const ConfigVarValue*, void*) {
    if (value != nullptr && value->type == CONFIG_VAR_BOOL && value->bool_value) {
        request_update_check(nullptr, nullptr);
    }
}

std::string json_string_after(const std::string& json, const std::string& key,
    std::size_t start = 0) {
    const auto keyAt = json.find('"' + key + '"', start);
    if (keyAt == std::string::npos) return {};
    auto at = json.find(':', keyAt + key.size() + 2);
    if (at == std::string::npos) return {};
    at = json.find('"', at + 1);
    if (at == std::string::npos) return {};
    const auto end = json.find('"', at + 1);
    return end == std::string::npos ? std::string{} : json.substr(at + 1, end - at - 1);
}

std::string release_asset_url(const std::string& json) {
    std::size_t at = 0;
    while ((at = json.find("\"name\"", at)) != std::string::npos) {
        const auto name = json_string_after(json, "name", at);
        const auto nextAsset = json.find("\"name\"", at + 6);
        if (name == kReleaseAsset) {
            const auto urlAt = json.find("\"browser_download_url\"", at);
            if (urlAt != std::string::npos &&
                (nextAsset == std::string::npos || urlAt < nextAsset)) {
                return json_string_after(json, "browser_download_url", urlAt);
            }
        }
        at += 6;
    }
    return {};
}

bool parse_version(const std::string& text, std::array<int, 4>& parts) {
    std::size_t at = (!text.empty() && (text[0] == 'v' || text[0] == 'V')) ? 1 : 0;
    bool found = false;
    for (auto& part : parts) {
        if (at >= text.size() || !std::isdigit(static_cast<unsigned char>(text[at]))) break;
        found = true;
        while (at < text.size() && std::isdigit(static_cast<unsigned char>(text[at]))) {
            part = part * 10 + text[at++] - '0';
        }
        if (at >= text.size() || text[at] != '.') break;
        ++at;
    }
    return found;
}

bool version_is_newer(const std::string& latest, const std::string& current) {
    std::array<int, 4> lhs{};
    std::array<int, 4> rhs{};
    return parse_version(latest, lhs) && parse_version(current, rhs) && lhs > rhs;
}

std::string fetch_url(const std::string& url) {
    std::string response;
#ifdef _WIN32
    HINTERNET internet = InternetOpenA("TwilightHDUpdater", INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr, nullptr, 0);
    if (internet == nullptr) return {};
    HINTERNET request = InternetOpenUrlA(internet, url.c_str(),
        "Accept: application/vnd.github+json", -1L,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (request != nullptr) {
        char buffer[4096];
        DWORD read = 0;
        while (InternetReadFile(request, buffer, sizeof(buffer), &read) && read != 0) {
            response.append(buffer, read);
        }
        InternetCloseHandle(request);
    }
    InternetCloseHandle(internet);
#elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    (void)url;
#else
    const std::string command = "curl -fsSL --connect-timeout 8 --max-time 30 "
        "-A 'TwilightHDUpdater' -H 'Accept: application/vnd.github+json' '" +
        url + "'";
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr) return {};
    char buffer[4096];
    while (const auto size = std::fread(buffer, 1, sizeof(buffer), pipe)) {
        response.append(buffer, size);
    }
    if (pclose(pipe) != 0) response.clear();
#endif
    return response;
}

bool download_file(const std::string& url, const std::filesystem::path& path) {
#ifdef _WIN32
    HINTERNET internet = InternetOpenA("TwilightHDUpdater", INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr, nullptr, 0);
    if (internet == nullptr) return false;
    HINTERNET request = InternetOpenUrlA(internet, url.c_str(),
        nullptr, 0,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, 0);
    if (request == nullptr) {
        InternetCloseHandle(internet);
        return false;
    }
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    char buffer[8192];
    DWORD read = 0;
    while (output && InternetReadFile(request, buffer, sizeof(buffer), &read) && read != 0) {
        output.write(buffer, read);
    }
    const bool ok = output.good();
    output.close();
    InternetCloseHandle(request);
    InternetCloseHandle(internet);
    return ok;
#elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
    (void)url;
    (void)path;
    return false;
#else
    const std::string command = "curl -fsSL --connect-timeout 8 --max-time 120 "
        "-A 'TwilightHDUpdater' -o '" + path.string() + "' '" + url + "'";
    return std::system(command.c_str()) == 0;
#endif
}

std::filesystem::path update_target() {
    const char* dataDir = nullptr;
    if (svc_host == nullptr ||
        svc_host->data_dir(mod_ctx, &dataDir) != MOD_OK || dataDir == nullptr || *dataDir == 0) {
        return {};
    }
    const auto mods = std::filesystem::path(dataDir).parent_path().parent_path() / "mods";
    const auto canonical = mods / kReleaseAsset;
    std::error_code ec;
    if (std::filesystem::exists(canonical, ec)) return canonical;

    std::filesystem::path candidate;
    std::size_t count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(mods, ec)) {
        if (ec || !entry.is_regular_file() || entry.path().extension() != ".dusk") continue;
        std::string name = entry.path().filename().string();
        for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (name.find("twilight-hd") != std::string::npos ||
            name.find("twilight_hd_hud") != std::string::npos) {
            candidate = entry.path();
            ++count;
        }
    }
    return count == 1 ? candidate : canonical;
}

bool valid_download(const std::filesystem::path& path) {
    std::error_code ec;
    if (std::filesystem::file_size(path, ec) < 1024 * 1024 || ec) return false;
    std::ifstream input(path, std::ios::binary);
    char magic[4]{};
    input.read(magic, sizeof(magic));
    return input.gcount() == 4 && magic[0] == 'P' && magic[1] == 'K' &&
        magic[2] == 3 && magic[3] == 4;
}

bool download_and_replace(const std::string& url) {
    namespace fs = std::filesystem;
    const fs::path target = update_target();
    if (target.empty()) return false;
    const fs::path temporary = target.string() + ".download";
    const fs::path backup = target.string() + ".previous";
    std::error_code ec;
    fs::create_directories(target.parent_path(), ec);
    fs::remove(temporary, ec);
    if (!download_file(url, temporary) || !valid_download(temporary)) {
        fs::remove(temporary, ec);
        return false;
    }

    fs::remove(backup, ec);
    ec.clear();
    if (fs::exists(target, ec)) {
        fs::rename(target, backup, ec);
        if (ec) {
            fs::remove(temporary, ec);
            return false;
        }
    }
    ec.clear();
    fs::rename(temporary, target, ec);
    if (!ec) return true;

    fs::remove(temporary, ec);
    if (fs::exists(backup, ec)) {
        ec.clear();
        fs::rename(backup, target, ec);
    }
    return false;
}

void show_dialog(const char* title, const std::string& body, UiDialogVariant variant,
    const UiDialogAction* actions, std::size_t count) {
    UiDialogDesc desc = UI_DIALOG_DESC_INIT;
    desc.title = title;
    desc.body_rml = body.c_str();
    desc.variant = variant;
    desc.actions = actions;
    desc.action_count = count;
    svc_ui->dialog_push(mod_ctx, &desc, nullptr);
}

void confirm_update(ModContext*, UiDialogHandle, void*) {
    std::string url;
    {
        std::lock_guard lock{s_mutex};
        url = s_result.downloadUrl;
    }
    if (url.empty()) return;
    if (s_worker.joinable()) s_worker.join();
    s_state.store(State::Downloading);
    s_worker = std::thread([url] {
        const bool ok = download_and_replace(url);
        {
            std::lock_guard lock{s_mutex};
            s_result.installSucceeded = ok;
        }
        s_state.store(State::DownloadReady, std::memory_order_release);
    });
}

}  // namespace

void request_update_check(ModContext*, void* userData) {
    State expected = State::Idle;
    if (!s_state.compare_exchange_strong(expected, State::Checking)) return;
    if (s_worker.joinable()) s_worker.join();
    const bool manual = userData != nullptr;
    const char* version = svc_host->mod_version(mod_ctx);
    const std::string current = version != nullptr ? version : "unknown";
    s_worker = std::thread([manual, current] {
        Result result;
        result.manual = manual;
        result.currentVersion = current;
        const auto response = fetch_url(kLatestReleaseApi);
        result.latestVersion = json_string_after(response, "tag_name");
        result.downloadUrl = release_asset_url(response);
        result.succeeded = !result.latestVersion.empty() && !result.downloadUrl.empty();
        result.updateAvailable = result.succeeded &&
            version_is_newer(result.latestVersion, result.currentVersion);
        {
            std::lock_guard lock{s_mutex};
            s_result = std::move(result);
        }
        s_state.store(State::CheckReady, std::memory_order_release);
    });
}

bool update_service_busy(ModContext*, void*) {
    return s_state.load(std::memory_order_acquire) != State::Idle;
}

void initialize_update_service() {
    svc_config->subscribe(mod_ctx, check_for_updates_config_var(),
        on_check_setting_changed, nullptr, nullptr);
    if (check_for_updates_enabled()) {
        request_update_check(nullptr, nullptr);
    }
}

void update_update_service() {
    const State state = s_state.load(std::memory_order_acquire);
    if (state != State::CheckReady && state != State::DownloadReady) return;
    if (s_worker.joinable()) s_worker.join();

    Result result;
    {
        std::lock_guard lock{s_mutex};
        result = s_result;
    }
    s_state.store(State::Idle, std::memory_order_release);

    const UiDialogAction ok{"OK", nullptr, nullptr, false};
    if (state == State::DownloadReady) {
        if (result.installSucceeded) {
            show_dialog("Update Complete",
                "<b>Twilight HD</b> has been updated to <b>" + result.latestVersion +
                    "</b>.<br/><br/>Restart Dusklight to apply the update.",
                UI_DIALOG_NORMAL, &ok, 1);
        } else {
            show_dialog("Update Failed",
                "The update could not be downloaded or installed. Your existing mod was preserved.",
                UI_DIALOG_DANGER, &ok, 1);
        }
        return;
    }

    if (!result.succeeded) {
        if (result.manual) {
            show_dialog("Update Check Failed",
                "Dusklight could not reach or read the Twilight HD release.",
                UI_DIALOG_WARNING, &ok, 1);
        }
    } else if (result.updateAvailable) {
        static UiDialogAction actions[2];
        actions[0] = {"Update Now", confirm_update, nullptr, false};
        actions[1] = {"Not Now", nullptr, nullptr, false};
        show_dialog("Mod Update Available",
            "A newer version of <b>Twilight HD</b> is available.<br/><br/>Installed: <b>v" +
                result.currentVersion + "</b><br/>Latest: <b>" + result.latestVersion +
                "</b><br/><br/>Download and install it now?",
            UI_DIALOG_NORMAL, actions, 2);
    } else if (result.manual) {
        show_dialog("Twilight HD Is Up to Date",
            "Installed: <b>v" + result.currentVersion + "</b><br/>Latest: <b>" +
                result.latestVersion + "</b>", UI_DIALOG_NORMAL, &ok, 1);
    }
}

void shutdown_update_service() {
    if (s_worker.joinable()) s_worker.join();
    s_state.store(State::Idle, std::memory_order_release);
}

}  // namespace twilight_hd_hud
