#include "file_select_layout.hpp"
#include <cassert>
#include <cmath>
#include <iostream>

using namespace twilight_hd_hud::file_select_layout;

int main() {
    for (float parentX : {0.75f, 1.0f, 4.0f / 3.0f, 2.0f})
    for (float parentY : {0.75f, 1.0f, 1.25f})
    for (float right : {608.0f, 702.0f, 1000.0f}) {
        const auto group = prompt_group_layout(parentX, parentY,
            608.0f, 448.0f, right, 12.0f);
        assert(std::abs(parentX * group.scaleX - parentY) < 0.0001f);
        assert(std::abs(group.centerX + 304.0f * parentY - right) < 0.0001f);
        assert(std::abs(group.centerY - 224.0f * parentY - 12.0f) < 0.0001f);
    }
    assert(kActionCursorPaddingX == 2.0f);
    assert(kActionCursorPaddingY == 1.5f);
    assert(kActionCursorPaddingX < 7.0f);
    assert(kActionCursorPaddingY < 5.0f);
    // Heading and compact rules share the same scalable canvas. The text
    // cell matches the File Selection title position inside a fixed banner.
    // The native-to-custom heading transition must preserve its font size.
    assert(kSaveQuestionBannerTop == 24.0f && kSaveQuestionBannerBottom == 80.0f);
    assert(kSaveHeaderFontSize == kHeaderFontSize);
    assert(kSaveQuestionTextCenter - kSaveQuestionTextHeight * 0.5f == kHeaderTextTop);
    assert(kSaveQuestionTextCenter + kSaveQuestionTextHeight * 0.5f == kHeaderTextBottom);
    assert(kSaveQuestionFontSize == kSaveHeaderFontSize);
    for (float scale : {0.5f, 1.0f, 1080.0f / 448.0f, 3.0f}) {
        const float top = kSaveQuestionBannerTop * scale;
        const float bottom = kSaveQuestionBannerBottom * scale;
        const float center = kSaveQuestionTextCenter * scale;
        const float halfHeight = kSaveQuestionTextHeight * scale * 0.5f;
        assert(center - halfHeight > top);
        assert(center + halfHeight < bottom);
        assert(std::abs(center - (kHeaderTextTop + kHeaderTextBottom) * 0.5f * scale) < 0.001f);
    }
    assert(play_time_center(0.0f, 80.0f) == 67.0f);
    // Short/tall cards, all slot positions, selection scale and resolution.
    for (float height : {70.0f, 86.0f})
    for (float top : {100.0f, 200.0f, 300.0f})
    for (float scale : {0.1f, 0.9f, 1.0f, 1.1f, 2.25f, 3.0f}) {
        const float center = play_time_center(top * scale, (top + height) * scale);
        const float divider = (top + height * 58.0f / 80.0f) * scale;
        const float border = (top + height * 76.0f / 80.0f) * scale;
        assert(center > divider && center < border);
        assert(std::abs((center - divider) - (border - center)) < 0.001f);
        assert(std::abs(center - play_time_center(top, top + height) * scale) < 0.001f);
    }
    std::cout << "PASS: play time centered below divider for every card height/scale\n";
}
