#include "controller_prompts.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

using namespace twilight_hd_hud;

struct LiteralTextBox {
    std::string text = "%s";
    int writes = 0;
    void setString(int size, const char* value) {
        assert(size == 4);
        text = value;
        ++writes;
    }
};

int main() {
    LiteralTextBox combo;
    for (auto layout : {ButtonLayout::BayxFlipped, ButtonLayout::Xbox,
             ButtonLayout::PlayStation, ButtonLayout::Nintendo, ButtonLayout::Universal}) {
        refresh_item_combo_label(&combo, combo.text.c_str(), layout);
        assert(combo.text == item_combo_button_label(layout));
        assert(combo.text != "%s");
        const int previousWrites = combo.writes;
        for (int frame = 0; frame < 100; ++frame) {
            refresh_item_combo_label(&combo, combo.text.c_str(), layout);
        }
        assert(combo.writes == previousWrites);
    }
    assert(combo.writes == 3); // LT, L2, ZL; no reallocations for unchanged labels.
    refresh_item_combo_label<LiteralTextBox>(nullptr, nullptr, ButtonLayout::Xbox);
    for (auto layout : {ButtonLayout::Xbox, ButtonLayout::BayxFlipped}) {
        assert(uses_xbox_prompts(layout));
        assert(std::string(item_combo_button_label(layout)) == "LT");
    }
    for (auto layout : {ButtonLayout::Nintendo, ButtonLayout::Universal}) {
        assert(!uses_xbox_prompts(layout));
        assert(std::string(item_combo_button_label(layout)) == "ZL");
    }
    assert(!uses_xbox_prompts(ButtonLayout::PlayStation));
    assert(std::string(item_combo_button_label(ButtonLayout::PlayStation)) == "L2");

    constexpr ShoulderPrompt buttons[] = {
        ShoulderPrompt::L, ShoulderPrompt::R, ShoulderPrompt::Zl, ShoulderPrompt::Zr};
    constexpr const char* labels[] = {"lb", "rb", "lt", "rt"};
    for (int style = 0; style < 2; ++style) {
        for (int button = 0; button < 4; ++button) {
            const std::string path = kXboxShoulderPaths[style][static_cast<int>(buttons[button])];
            const std::string expected = std::string("hud/shoulder-button-xbox-") +
                labels[button] + (style == 1 ? "-black-pro.bti" : ".bti");
            assert(path == expected);
            std::ifstream file("res/" + path, std::ios::binary);
            assert(file);
            std::vector<unsigned char> bytes{
                std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            // Native RGBA8 BTI, 64x64, image starts immediately after header.
            assert(bytes.size() == 32 + 64 * 64 * 4);
            assert(bytes[0] == 6 && bytes[2] == 0 && bytes[3] == 64);
            assert(bytes[4] == 0 && bytes[5] == 64);
            assert(bytes[28] == 0 && bytes[29] == 0 && bytes[30] == 0 && bytes[31] == 32);
        }
    }
    std::cout << "PASS: literal combo text and refresh, both Xbox layouts, all eight shoulder asset mappings and BTI payloads\n";
}
