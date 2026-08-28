#pragma once

namespace twilight_hd_hud::dialogue_text_layout {

struct Transform {
    float x, y, scale;
    constexpr float position_x(float value) const { return x + (value - x) * scale; }
    constexpr float position_y(float value) const { return y + (value - y) * scale; }
    constexpr float size(float value) const { return value * scale; }
};

} // namespace twilight_hd_hud::dialogue_text_layout
