#pragma once

#include <optional>

namespace twilight_hd_hud {

// File select does not set the in-game pause/window flags. A live player
// alone is also insufficient while returning to the title/file-select screen.
constexpr bool gameplay_shortcuts_active(bool hasPlayer, bool fileSelect,
    bool menuOrPause, bool inputBlocked) {
    return hasPlayer && !fileSelect && !menuOrPause && !inputBlocked;
}

struct MinimapRestore {
    bool preference;
    bool visible;
};

// One snapshot per manually opened map. Keep it through zooms and close
// animation frames, but apply it only once when the map starts closing.
class MinimapReturnState {
public:
    void begin(bool preference) {
        if (active_) return;
        active_ = true;
        preference_ = preference;
    }

    std::optional<MinimapRestore> close(bool closing, bool canDisplay) {
        if (!active_ || restored_ || !closing) return std::nullopt;
        restored_ = true;
        return MinimapRestore{preference_, preference_ && canDisplay};
    }

    bool active() const { return active_; }
    void reset() { *this = {}; }

private:
    bool active_ = false;
    bool restored_ = false;
    bool preference_ = false;
};

} // namespace twilight_hd_hud
