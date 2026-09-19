from source_helpers import read_hook_source
"""Execute the production selection/input block with a repeat-controller double."""
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = read_hook_source()
selection = 'int caught_fish_in_direction(' + source.split(
    'int caught_fish_in_direction(', 1)[1].split('void ensure_fish_journal_overlay(', 1)[0]
movement = source.split('void after_fishing_move(', 1)[1].split('#if TARGET_PC', 1)[0]
movement = movement[movement.index('    const bool up'):]
program = r'''
#include <cassert>
#include <cstdlib>
constexpr int MAX_FINDABLE_FISHES = 6, PAD_1 = 0;
constexpr int kFishBookSlots[6] = {1, 5, 4, 2, 3, 0};
bool caught[6] = {true, true, true, true, true, true};
int dComIfGs_getFishNum(int index) { return caught[index]; }
struct { int selected; } s_fishJournal;
// Model STControl's shared per-axis repeat timer: a pending first trigger
// remains available until consumed, then waits before repeating while held.
struct Stick {
    int direction = -1, timer = 0;
    void frame(int held) { if (held != direction) timer = 0; else if (timer > 0) --timer; direction = held; }
    bool trigger(int d) { if (direction != d || timer > 0) return false; timer = 5; return true; }
    bool checkUpTrigger() { return trigger(0); }
    bool checkDownTrigger() { return trigger(1); }
    bool checkLeftTrigger() { return trigger(2); }
    bool checkRightTrigger() { return trigger(3); }
};
struct Menu { Stick* mpStick; };
struct mDoCPd_c {
    static inline int pressed = -1;
    static bool getTrigUp(int) { return pressed == 0; }
    static bool getTrigDown(int) { return pressed == 1; }
    static bool getTrigLeft(int) { return pressed == 2; }
    static bool getTrigRight(int) { return pressed == 3; }
};
'''+selection+'\nvoid move(Menu* menu) {\n'+movement+r'''
}
int main() {
    // Every edge of the two-column book, in both directions.
    for (auto column : {0, 1}) {
        const int top = column ? 0 : 5, middle = column ? 4 : 3, bottom = column ? 1 : 2;
        assert(caught_fish_in_direction(top, 0, 1) == middle);
        assert(caught_fish_in_direction(middle, 0, 1) == bottom);
        assert(caught_fish_in_direction(bottom, 0, -1) == middle);
        assert(caught_fish_in_direction(middle, 0, -1) == top);
        assert(caught_fish_in_direction(bottom, 0, 1) == bottom);
        assert(caught_fish_in_direction(top, 0, -1) == top);
        for (bool dpad : {false, true}) {
            Stick stick; Menu menu{&stick}; s_fishJournal.selected = top;
            stick.frame(1); mDoCPd_c::pressed = dpad ? 1 : -1; move(&menu);
            assert(s_fishJournal.selected == middle);
            mDoCPd_c::pressed = -1;
            for (int frame = 0; frame < 4; ++frame) {
                stick.frame(1); move(&menu);
                assert(s_fishJournal.selected == middle);
            }
            stick.frame(1); move(&menu); assert(s_fishJournal.selected == bottom);
            // Release/repress moves immediately, exactly once.
            stick.frame(-1); move(&menu);
            stick.frame(0); mDoCPd_c::pressed = dpad ? 0 : -1; move(&menu);
            assert(s_fishJournal.selected == middle);
            mDoCPd_c::pressed = -1; stick.frame(0); move(&menu);
            assert(s_fishJournal.selected == middle);
        }
        // Skip only genuinely uncaught entries, not the caught middle row.
        caught[middle] = false;
        assert(caught_fish_in_direction(top, 0, 1) != middle);
        caught[middle] = true;
    }
    for (auto left : {5, 3, 2}) {
        int right = caught_fish_in_direction(left, 1, 0);
        assert(kFishBookSlots[right] == kFishBookSlots[left] + 1);
        assert(caught_fish_in_direction(right, -1, 0) == left);
    }
    Menu fallback{nullptr}; s_fishJournal.selected = 5;
    mDoCPd_c::pressed = 1; move(&fallback); assert(s_fishJournal.selected == 3);
    mDoCPd_c::pressed = -1; move(&fallback); assert(s_fishJournal.selected == 3);
}
'''
program = '#include <initializer_list>\n' + program
compiler = shutil.which('clang++') or shutil.which('c++')
assert compiler, 'A C++ compiler is required for the input regression test'
with tempfile.TemporaryDirectory(prefix='twilight-fish-input-') as directory:
    test = Path(directory) / 'input.cpp'
    test.write_text(program)
    binary = Path(directory) / 'input-test'
    subprocess.run([compiler, '-std=c++17', str(test), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: fish navigation consumes each press once, repeats, and reaches all six fish')
