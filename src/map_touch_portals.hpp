#pragma once

namespace twilight_hd_hud {

class MapTouchPortals {
public:
    void observe(bool virtualLeft, bool acceptedLeft, bool fieldMapOpen) noexcept {
        const bool held = virtualLeft && acceptedLeft && fieldMapOpen;
        mPressed = held && !mHeld;
        mHeld = held;
    }

    bool held() const noexcept { return mHeld; }

    bool take_pressed() noexcept {
        const bool pressed = mPressed;
        mPressed = false;
        return pressed;
    }

    void clear() noexcept { mHeld = mPressed = false; }

private:
    bool mHeld = false;
    bool mPressed = false;
};

}  // namespace twilight_hd_hud
