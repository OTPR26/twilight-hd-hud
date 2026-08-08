#pragma once

namespace twilight_hd_hud {

struct HudTransform {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float scale = 1.0f;
};

struct HudItemLayout {
    float itemScale = 1.0f;
    float itemOffsetX = 0.0f;
    float itemOffsetY = 0.0f;
    float ammoScale = 1.0f;
    float ammoOffsetX = 0.0f;
    float ammoOffsetY = 0.0f;
};

inline constexpr HudTransform kRItemTransform = {
    .offsetX = 29.0f,
    .offsetY = -11.0f,
};

inline constexpr HudItemLayout kRItemLayout = {
    .ammoScale = 0.55f,
    .ammoOffsetX = -9.0f,
    .ammoOffsetY = -22.0f,
};

}  // namespace twilight_hd_hud
