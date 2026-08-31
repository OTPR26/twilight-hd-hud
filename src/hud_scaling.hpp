#pragma once

#include <cstdint>

namespace twilight_hd_hud {

constexpr std::int64_t clamp_hud_percent(std::int64_t percent) {
    return percent < 50 ? 50 : percent > 125 ? 125 : percent;
}

constexpr std::int64_t legacy_hud_percent(std::int64_t selection) {
    return selection == 0 ? 75 : selection == 2 ? 125 : 100;
}

constexpr float hud_size_multiplier(std::int64_t percent) {
    return static_cast<float>(clamp_hud_percent(percent)) / 100.0f;
}

constexpr std::int64_t effective_hud_percent(std::int64_t overall,
    std::int64_t individual) {
    return clamp_hud_percent(overall) != 100 ? clamp_hud_percent(overall) :
        clamp_hud_percent(individual);
}

struct HudScales {
    float overall;
    float controllerDiamond;
    float dpad;
    float hearts;
    float actionText;
    float dialogueText;
    float rupees;
    float minimap;
};

constexpr HudScales compose_hud_scales(std::int64_t overall,
    std::int64_t diamond = 100, std::int64_t dpad = 100, std::int64_t hearts = 100,
    std::int64_t actionText = 100, std::int64_t dialogueText = 100,
    std::int64_t rupees = 100, std::int64_t minimap = 100)
{
    return {hud_size_multiplier(overall),
        hud_size_multiplier(effective_hud_percent(overall, diamond)),
        hud_size_multiplier(effective_hud_percent(overall, dpad)),
        hud_size_multiplier(effective_hud_percent(overall, hearts)),
        // Overall HUD Size is an icon/layout control. Text remains independent
        // so changing the visual HUD cannot silently change readability.
        hud_size_multiplier(actionText),
        hud_size_multiplier(dialogueText),
        hud_size_multiplier(effective_hud_percent(overall, rupees)),
        hud_size_multiplier(effective_hud_percent(overall, minimap))};
}

constexpr float minimap_multiplier(float settingScale) { return 0.70f * settingScale; }

// Digit dimensions are in draw coordinates; the icon inherits Overall from
// the shared rupee/key parent. Compensate only the icon, leaving keys alone.
constexpr float rupee_icon_multiplier(float overall, float rupees) { return rupees / overall; }
constexpr float rupee_digit_size(float scale) { return 11.0f * scale; }
constexpr float rupee_digit_step(float scale) { return 13.0f * scale; }
constexpr float rupee_digit_gap(float scale) { return 5.0f * scale; }
constexpr float rupee_strip_width(int digits, float scale) {
    return rupee_digit_gap(scale) + (digits - 1) * rupee_digit_step(scale) + rupee_digit_size(scale);
}
constexpr float rupee_icon_right(float originalRight, int digits, float scale) {
    return originalRight + rupee_strip_width(digits, 1.0f) - rupee_strip_width(digits, scale);
}

// Screenshot comparison: the contextual labels are about 25% larger than TPHD.
// 100% is the new TPHD-sized baseline; 125% recovers the previous text size.
constexpr float action_text_multiplier(float settingScale) { return 0.8f * settingScale; }

// The same dialogue line measures 1036px here versus 710px in TPHD at 1080p.
// Some messages carry a top-level scale above 100% (notably Yeto). Remove that
// source variation before applying the common TPHD baseline; sub-100% emphasis
// remains intact.
constexpr float dialogue_text_multiplier(float settingScale,
                                          float sourceMessageScale = 1.0f) {
    const float normalizedSource = sourceMessageScale > 1.0f ? sourceMessageScale : 1.0f;
    return 0.68f * settingScale / normalizedSource;
}

}  // namespace twilight_hd_hud
