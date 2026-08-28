#pragma once

namespace twilight_hd_hud::action_prompt_layout {

// At 1920x1080: our visible A face is 56px; TPHD is about 46-47px.
constexpr float kButtonScale = 5.0f / 6.0f;
// The complete 88px button texture has a 60px face, with transparent margins.
constexpr float kFaceCoverage = 60.0f / 88.0f;

struct Row {
    float buttonCenter;
    float textLeft;
    float faceWidth;
    float gap;
};

constexpr Row arrange(float center, float nativeCanvasWidth, float textWidth) {
    const float face = nativeCanvasWidth * kButtonScale * kFaceCoverage;
    // Roughly 14px to the text origin, 16px to the glyph ink at 1080p.
    const float gap = face * 0.30f;
    const float left = center - (face + gap + textWidth) * 0.5f;
    return {left + face * 0.5f, left + face + gap, face, gap};
}

} // namespace twilight_hd_hud::action_prompt_layout
