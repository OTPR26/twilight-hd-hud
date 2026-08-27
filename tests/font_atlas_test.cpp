#include "font_atlas.hpp"
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

using namespace twilight_hd_hud::font_atlas;

int main(int argc, char** argv) {
    assert(argc == 4);
    for (int i = 1; i < argc; ++i) {
        std::ifstream stream(argv[i], std::ios::binary);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(stream)), {});
        assert(valid(bytes.data(), bytes.size()));
        assert(!valid(nullptr, bytes.size()));
        assert(!valid(bytes.data(), bytes.size() - 1));
        for (std::size_t offset : {0u, 8u, 12u, 32u, 36u, 40u, 42u, 44u, 46u, 48u,
                50u, 64u, 68u, 72u, 74u, 608u, 612u, 616u, 618u, 620u, 622u,
                640u, 644u, 648u, 650u, 652u, 654u, 656u, 660u, 662u, 664u, 666u, 668u}) {
            bytes[offset] ^= 0x01;
            assert(!valid(bytes.data(), bytes.size()));
            bytes[offset] ^= 0x01;
        }
    }
    assert(supported('A') && supported(0xe9) && supported(0xff));
    assert(!supported(-1) && !supported(0x19) && !supported(0x81) && !supported(0x100));
    assert(!supported(0x82a0));
    assert(std::abs(65 * opticalScale / cell - 0.75f) < .00001f);
    assert(std::abs(57 * firaOpticalScale / cell - 0.75f) < .00001f);
    // Match the native drawChar_scale return formula across both first/subsequent
    // character paths and fixed-width rendering. No changes to line-wrap inputs.
    for (bool fixed : {false, true}) for (bool flag : {false, true})
    for (int bearing : {0, 2, 6}) for (int width : {0, 8, 16, 24})
    for (float scale : {11.5f, 17.f, 24.f, 36.f}) {
        float expected = 22 * (scale / 24);
        if (!fixed) expected = (width + (flag ? 0 : bearing)) * (scale / 24);
        assert(advance(fixed, 22, flag, bearing, width, scale, 24) == expected);
        for (float rasterScale : {opticalScale, firaOpticalScale})
        for (int newWidth : {0, 30, 65, 110}) {
            const auto p = place(100, scale, 24, bearing, width, newWidth, fixed, 22, flag, rasterScale);
            assert(std::isfinite(p.x) && std::isfinite(p.scaleX));
            assert(p.scaleX >= 0 && p.scaleX <= scale * rasterScale);
            assert(newWidth * p.scaleX / cell <= (fixed ? 22 : width) * scale / 24 + .0001f);
        }
    }
    std::cout << "PASS: all three atlases, malformed resources, coverage, native advances, glyph fitting\n";
}
