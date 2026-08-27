#include "ui_refinements.hpp"
#include "item_help_text.hpp"

#include <cassert>
#include <cmath>
#include <initializer_list>

using namespace twilight_hd_hud;

int main() {
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
    // Shared item wording: bottles, bombs, Spinner, boots, rods, and ranged items.
    for (const std::string verb : {"Use ", "Drink with ", "Lift one with ",
            "Ride it with ", "Put them on with ", "Hold it with ", "Hold down "}) {
        const auto original = verb + icon(15) + " or " + icon(16) + " to use it.";
        assert(three_button_item_help(original + '\0') == verb + trio + " to use it.");
    }
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
    assert(item_help_button(1, false) == ItemHelpButton::Back);
    assert(item_help_button(3, false) == ItemHelpButton::Target);
    assert(item_help_button(4, false) == ItemHelpButton::Shoulder);
    assert(item_help_button(4, true) == ItemHelpButton::Target);
    assert(item_help_button(5, false) == ItemHelpButton::ItemX);
    assert(item_help_button(6, false) == ItemHelpButton::ItemY);
    assert(item_help_button(7, false) == ItemHelpButton::Shoulder);
    for (int type = 8; type < 70; ++type) {
        assert(item_help_button(type, true) == ItemHelpButton::None);
    }
    assert(item_help_button(2, true) == ItemHelpButton::None);

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
}
