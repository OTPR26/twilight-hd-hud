#pragma once

namespace twilight_hd_hud::action_prompt_layout {

// At 1920x1080: our visible A face is 56px; TPHD is about 46-47px.
constexpr float kButtonScale = 5.0f / 6.0f;
// The complete 88px button texture has a 60px face, with transparent margins.
constexpr float kFaceCoverage = 60.0f / 88.0f;
// The contextual trigger art is already authored at its intended TPHD size.
// Its square texture has modest transparent padding around the wide button.
constexpr float kTriggerScale = 0.9f;
constexpr float kTriggerCoverage = 0.82f;
// The bottle artwork has more transparent padding than its pane suggests.
// Give its visible silhouette a modest lift relative to the A-button face.
constexpr float kBottleScale = 1.20f;

struct Row {
    float buttonCenter;
    float textLeft;
    float faceWidth;
    float gap;
};

constexpr Row arrange_visible(float center, float visibleButtonWidth,
                              float textWidth) {
    const float face = visibleButtonWidth;
    // Roughly 14px to the text origin, 16px to the glyph ink at 1080p.
    const float gap = face * 0.30f;
    const float left = center - (face + gap + textWidth) * 0.5f;
    return {left + face * 0.5f, left + face + gap, face, gap};
}

constexpr Row arrange(float center, float nativeCanvasWidth, float textWidth) {
    return arrange_visible(center,
        nativeCanvasWidth * kButtonScale * kFaceCoverage, textWidth);
}

} // namespace twilight_hd_hud::action_prompt_layout
