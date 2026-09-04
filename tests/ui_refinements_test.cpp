#include "ui_refinements.hpp"
#include "item_help_text.hpp"

#include <cassert>
#include <cmath>
#include <initializer_list>

using namespace twilight_hd_hud;

int main() {
    // The confirmation replaces the whole Options title and gives the
    // centered message equal padding across native parent scales/languages.
    for (float titleScale : {0.75f, 1.0f, 1.5f}) {
        for (float textScale : {0.5f, 1.0f, 2.0f}) {
            for (float frameScale : {0.5f, 1.0f, 1.5f}) {
                for (float messageWidth : {60.0f, 210.0f, 330.0f}) {
                    for (float messageHeight : {18.0f, 42.0f}) {
                        const auto fitted = option_warning_size(176.0f * titleScale,
                            33.0f * titleScale, messageWidth * textScale,
                            messageHeight * textScale, textScale, textScale);
                        assert(fitted.width >= 176.0f * titleScale);
                        assert(fitted.height >= 33.0f * titleScale);
                        assert(fitted.width >= (messageWidth + 28.0f) * textScale);
                        assert(fitted.height >= (messageHeight + 16.0f) * textScale);
                        const float localWidth = fitted.width / frameScale;
                        const float localHeight = fitted.height / frameScale;
                        assert(std::fabs(localWidth * frameScale - fitted.width) < 0.001f);
                        assert(std::fabs(localHeight * frameScale - fitted.height) < 0.001f);
                    }
                }
            }
        }
    }

    // The visible cross, not the legacy Items pane origin, owns this anchor.
    // Test differing text widths, screen offsets, HUD scales and repeated draws.
    for (float screenX : {-200.0f, 0.0f, 180.0f}) {
        for (float scale : {0.5f, 0.75f, 1.0f, 1.25f, 2.0f}) {
            for (float textWidth : {90.0f, 120.0f, 180.0f}) {
                const float crossLeft = screenX + 40.0f * scale;
                const float crossRight = crossLeft + 48.0f * scale;
                const float crossBottom = 130.0f * scale;
                const float textLeft = screenX - 20.0f * scale;
                const float textRight = textLeft + textWidth * scale;
                const float textTop = 125.0f * scale;
                const auto offset = collection_dpad_offset(crossLeft, crossRight,
                    crossBottom, textLeft, textRight, textTop);
                assert(std::fabs((textLeft + textRight) * 0.5f + offset.x -
                    (crossLeft + crossRight) * 0.5f) < 0.001f);
                assert(std::fabs(textTop + offset.y - crossBottom + 4.8f * scale) < 0.001f);
                // Compared with preview 6, tighten the Collection gap and
                // lower Minimap without changing either horizontal position.
                const float previousTop = crossBottom + 6.72f * scale;
                assert(std::fabs(previousTop - (textTop + offset.y) - 11.52f * scale) < 0.001f);
                assert(std::fabs(minimap_dpad_optical_offset(crossRight - crossLeft) -
                    5.76f * scale) < 0.001f);
                const auto repeat = collection_dpad_offset(crossLeft, crossRight,
                    crossBottom, textLeft + offset.x, textRight + offset.x, textTop + offset.y);
                assert(std::fabs(repeat.x) < 0.001f && std::fabs(repeat.y) < 0.001f);
            }
        }
    }
    assert(minimap_dpad_optical_offset(0.0f) == 0.0f);

    const auto icon = [](unsigned char id) {
        std::string tag("\x1a\x05\0\0", 4);
        tag += static_cast<char>(id);
        return tag;
    };
    const std::string tail = "\nyou see " + icon(32) + " with " + icon(19) +
        ", and release the\nbutton to fly to that place.";
    const std::string expected = "A chain with a claw at one end.\nHold it with " +
        icon(16) + ", " + icon(15) + " or " + icon(17) + ", aim where" + tail;
    for (const auto pair : {std::pair{15, 16}, std::pair{16, 15}, std::pair{46, 47}}) {
        const std::string original = "A chain with a claw at one end.\nHold it with " +
            icon(pair.first) + " or " + icon(pair.second) + ", aim at places where" + tail;
        const std::string terminated = original + '\0';
        assert(three_button_item_help(terminated) == expected);
        assert(three_button_item_help(original).empty()); // missing terminator
        assert(three_button_item_help(expected + '\0').empty()); // idempotent
        // A malformed or truncated tag cannot read beyond the supplied block.
        for (std::size_t size = 0; size < terminated.size() - 1; ++size) {
            assert(three_button_item_help(std::string_view(terminated).substr(0, size)).empty());
        }
    }
    assert(three_button_item_help(std::string("\x1a\x04\0\0\0", 5)).empty());
    assert(three_button_item_help(std::string("\x1a\xff\0", 3)).empty());
    assert(three_button_item_help(std::string("Autre langue\0", 13)).empty());

    const auto trio = icon(16) + ", " + icon(15) + " or " + icon(17);
    const auto dominionRod = "Hold it with " + icon(16) + " or " + icon(15) +
        ", aim with " + icon(2) + ", and use it to give light and life to statues.";
    assert(three_button_item_get_help(dominionRod + '\0') ==
        "Hold it with " + trio + ", aim with " + icon(2) +
        ", and use it to give light and life to statues.");
    // Shared item wording: bottles, bombs, Spinner, boots, rods, and ranged items.
    for (const std::string verb : {"Use ", "Drink with ", "Lift one with ",
            "Ride it with ", "Put them on with ", "Hold it with ", "Hold down "}) {
        const auto original = verb + icon(15) + " or " + icon(16) + " to use it.";
        assert(three_button_item_help(original + '\0') == verb + trio + " to use it.");
    }
    const auto soup = "Set it to and drink it with " + icon(15) + " or " +
        icon(16) + " to restore 2 hearts.";
    assert(three_button_item_help(soup + '\0') ==
        "Set it to and drink it with " + trio + " to restore 2 hearts.");
    const auto superbSoup = "Set it to and drink it with " + icon(16) + " or " +
        icon(15) + " to replenish 8 hearts.";
    const auto multilineSuperbSoup = "Set it to and drink it with\n" + icon(16) + " or " +
        icon(15) + " to replenish 8 hearts.";
    assert(three_button_item_help(superbSoup + '\0') ==
        "Set it to and drink it with " + trio + " to replenish 8 hearts.");
    const std::string layoutTag("\x1a\x06\0\x01\x02\x03", 6);
    const auto taggedSoup = "Set it to and drink it with " + layoutTag + icon(16) +
        layoutTag + " or " + layoutTag + icon(15) + " to replenish 8 hearts.";
    const auto taggedPhraseSoup = "Set it " + layoutTag + "to and " + layoutTag +
        "drink it with " + layoutTag + icon(16) + layoutTag + " or " + layoutTag +
        icon(15) + " to replenish 8 hearts.";
    assert(three_button_item_help(taggedSoup + '\0').empty());
    assert(three_button_soup_item_help(taggedSoup + '\0') ==
        "Set it to and drink it with " + trio + " to replenish 8 hearts.");
    assert(three_button_soup_item_help(taggedPhraseSoup + '\0') ==
        "Set it " + layoutTag + "to and " + layoutTag + "drink it with " + trio +
        " to replenish 8 hearts.");
    assert(three_button_soup_item_help(superbSoup + '\0') ==
        "Set it to and drink it with " + trio + " to replenish 8 hearts.");
    assert(three_button_soup_item_help(multilineSuperbSoup + '\0') ==
        "Set it to and drink it with\n" + trio + " to replenish 8 hearts.");
    const auto ooccoo = "Set her to and call her with " + icon(16) + " or " +
        icon(15) + ".";
    assert(three_button_soup_item_help(ooccoo + '\0') ==
        "Set and call her with " + trio + ".");
    assert(three_button_item_get_help(ooccoo + '\0') ==
        "Set and call her with " + trio + ".");
    assert(three_button_soup_item_help(("Unrelated " + icon(16) + " or " + icon(15)) +
        '\0').empty());
    const auto twoPairs = icon(15) + " or " + icon(16) + ".\nAgain: " + icon(47) + " or " + icon(46);
    assert(three_button_item_help(twoPairs + '\0') == trio + ".\nAgain: " + trio);
    for (const std::string gap : {" \nor ", " or\n ", "\n  or "}) {
        const auto expanded = three_button_item_help(icon(15) + gap + icon(16) + '\0');
        assert(expanded.find('\n') != std::string::npos);
        assert(expanded.find(icon(17)) != std::string::npos);
        assert(three_button_item_help(expanded + '\0').empty());
    }
    // Action, lock-on, combo, single-button, already-updated and non-English text stay unchanged.
    for (const auto unchanged : {icon(10) + " or " + icon(11), icon(13) + " or " + icon(14),
            icon(15), icon(15) + " or " + icon(15), icon(15) + " ou " + icon(16), trio}) {
        assert(three_button_item_help(unchanged + '\0').empty());
    }
    const auto mixed = "Lift with " + icon(15) + " or " + icon(16) + ", press " + icon(10) +
        ". Combine with " + icon(14) + "; target with " + icon(13) + '.';
    const auto updated = three_button_item_help(mixed + '\0');
    assert(updated == "Lift with " + trio + ", press " + icon(10) +
        ". Combine with " + icon(14) + "; target with " + icon(13) + '.');
    assert(item_help_fit_scale(400.0f, 380.0f) == 1.0f);
    assert(item_help_fit_scale(400.0f, 400.0f) == 1.0f);
    assert(std::fabs(item_help_fit_scale(400.0f, 440.0f) * 440.0f - 400.0f) < 0.001f);
    assert(item_help_fit_scale(0.0f, 440.0f) == 1.0f);

    assert(item_help_button(0, false) == ItemHelpButton::Action);
    assert(kItemHelpArtworkScale == 1.5f);
    assert(kItemHelpTitleFontSize == 28.0f);
    assert(item_help_button(1, false) == ItemHelpButton::Back);
    assert(item_help_button(2, false) == ItemHelpButton::Stick);
    assert(item_help_button(9, false) == ItemHelpButton::Stick);
    assert(item_help_button(69, false) == ItemHelpButton::Stick);
    assert(item_help_shift_stick(false, 0));
    assert(item_help_shift_stick(false, 1));
    assert(item_help_shift_stick(true, 0));
    assert(!item_help_shift_stick(true, 1));
    assert(item_help_button(3, false) == ItemHelpButton::Target);
    assert(item_help_button(4, false) == ItemHelpButton::Shoulder);
    assert(item_help_button(4, true) == ItemHelpButton::Target);
    assert(item_help_button(5, false) == ItemHelpButton::ItemX);
    assert(item_help_button(6, false) == ItemHelpButton::ItemY);
    assert(item_help_button(7, false) == ItemHelpButton::Shoulder);
    // Assignment R/RB/R1 and targeting ZR/RT/R2 are different native glyphs
    // even when they appear together in the same boomerang description.
    assert(item_help_button(4, false, true) == ItemHelpButton::Trigger);
    assert(item_help_button(7, false, true) == ItemHelpButton::Shoulder);
    assert(item_help_button(3, false, true) == ItemHelpButton::Target);
    assert(item_help_button(5, false, true) == ItemHelpButton::ItemX);
    assert(item_help_button(6, false, true) == ItemHelpButton::ItemY);
    for (float scale : {0.5f, 0.75f, 1.0f, 1.5f, 2.0f}) {
        const float x = 100.0f * scale;
        const float width = 20.0f * scale;
        assert(std::fabs(item_help_icon_x(x, width, ItemHelpButton::Back) -
            94.0f * scale) < 0.001f);
        for (ItemHelpButton button : {ItemHelpButton::ItemX, ItemHelpButton::ItemY}) {
            assert(std::fabs(item_help_icon_x(x, width, button) - 92.0f * scale) < 0.001f);
        }
        assert(std::fabs(item_help_icon_x(x, width, ItemHelpButton::Action) -
            93.0f * scale) < 0.001f);
        assert(std::fabs(item_help_icon_x(x, width, ItemHelpButton::Stick) -
            92.0f * scale) < 0.001f);
        assert(std::fabs(item_help_icon_x(x, width, ItemHelpButton::Target) -
            88.0f * scale) < 0.001f);
        assert(std::fabs(item_help_icon_x(x, width, ItemHelpButton::Trigger) -
            96.0f * scale) < 0.001f);
        for (ItemHelpButton button : {ItemHelpButton::Shoulder}) {
            assert(std::fabs(item_help_icon_x(x, width, button) - 92.0f * scale) < 0.001f);
        }
        assert(item_help_icon_x(x, width, ItemHelpButton::None) == x);
        for (ItemHelpButton button : {ItemHelpButton::Back,
                 ItemHelpButton::ItemX, ItemHelpButton::ItemY, ItemHelpButton::Shoulder}) {
            assert(std::fabs(item_help_icon_y(x, width, button) - 98.0f * scale) < 0.001f);
        }
        assert(std::fabs(item_help_icon_y(x, width, ItemHelpButton::Action) -
            95.0f * scale) < 0.001f);
        assert(std::fabs(item_help_icon_y(x, width, ItemHelpButton::Stick) -
            93.0f * scale) < 0.001f);
        for (ItemHelpButton button : {ItemHelpButton::Target, ItemHelpButton::Trigger}) {
            assert(std::fabs(item_help_icon_y(x, width, button) - 94.0f * scale) < 0.001f);
        }
        assert(item_help_icon_y(x, width, ItemHelpButton::None) == x);
    }
    assert(item_help_icon_x(100.0f, 0.0f, ItemHelpButton::Shoulder) == 100.0f);
    assert(item_help_icon_y(100.0f, 0.0f, ItemHelpButton::Shoulder) == 100.0f);
    assert(item_help_button(8, true) == ItemHelpButton::None);
    for (int type = 10; type < 69; ++type) {
        assert(item_help_button(type, true) == ItemHelpButton::None);
    }

    assert(show_ring_assignment_prompts(false, 0));
    assert(!show_ring_assignment_prompts(true, 0));
    for (int status = 1; status <= 5; ++status) {
        assert(!show_ring_assignment_prompts(false, status));
        assert(!show_ring_assignment_prompts(true, status));
    }

    // Resizing and scaling cannot pull a full meter frame off its anchor.
    for (float width : {608.0f, 796.0f, 1045.0f}) {
        const float safeLeft = (608.0f - width) * 0.5f;
        const float safeRight = safeLeft + width;
        for (float scale : {0.5f, 0.75f, 1.0f, 1.25f, 2.0f}) {
            const float left = 20.0f;
            const float right = left + 140.0f * scale;
            const float top = 90.0f;
            const float bottom = top + 12.0f * scale;
            const auto offset = top_meter_offset(safeLeft, safeRight, 0.0f,
                left, top, right, bottom);
            assert(std::fabs((left + right) * 0.5f + offset.x - 304.0f) < 0.001f);
            assert(std::fabs((top + bottom) * 0.5f + offset.y - 54.0f) < 0.001f);
            const auto repeat = top_meter_offset(safeLeft, safeRight, 0.0f,
                left + offset.x, top + offset.y, right + offset.x, bottom + offset.y);
            assert(std::fabs(repeat.x) < 0.001f && std::fabs(repeat.y) < 0.001f);
        }
    }
    const auto safeArea = top_meter_offset(20.0f, 600.0f, 10.0f,
        240.0f, 58.0f, 380.0f, 70.0f);
    assert(safeArea.x == 0.0f && safeArea.y == 0.0f);

    // Native kantera layout: 115-wide end containers are centered at 24/153;
    // their actual ornaments start at -25 and are only 26 units wide.
    // The old container union therefore biased the visible center by -12.
    for (float canvasWidth : {608.0f, 796.0f, 1045.0f})
    for (float scale : {0.35f, 0.525f, 0.7f, 0.875f, 1.4f}) {
        const float safeLeft = (608.0f - canvasWidth) * 0.5f;
        const float safeRight = safeLeft + canvasWidth;
        const float artLeft = (24.0f - 25.0f) * scale;
        const float artRight = (153.0f - 25.0f + 26.0f) * scale;
        const float oldLeft = (24.0f - 115.0f * 0.5f) * scale;
        const float oldRight = (153.0f + 115.0f * 0.5f) * scale;
        const auto oldOffset = top_meter_offset(safeLeft, safeRight, 0,
            oldLeft, 0, oldRight, 16 * scale);
        assert(std::fabs((artLeft + artRight) * 0.5f + oldOffset.x -
            (304.0f - 12.0f * scale)) < 0.001f);
        const auto corrected = top_meter_offset(safeLeft, safeRight, 0,
            artLeft, 0, artRight, 16 * scale);
        assert(std::fabs(artLeft + corrected.x - safeLeft -
            (safeRight - artRight - corrected.x)) < 0.001f);
        assert(std::fabs((artLeft + artRight) * 0.5f + corrected.x - 304.0f) < 0.001f);
    }
}
