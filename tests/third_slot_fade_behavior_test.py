"""Exercise the actual native and companion HUD decisions with mocked game state."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / "src/item_slot_hooks.cpp").read_text()
helpers = "bool third_slot_action_available(" + source.split(
    "bool third_slot_action_available(", 1)[1].split("void update_z_hud_item_alpha(", 1)[0]
dual = (root / "src/dual_screen_hud.inc").read_text()
decision = "const bool usable =" + dual.split("const bool usable =", 1)[1].split(
    "update_z_hud_item_alpha(meter, usable);", 1)[0]
fixture = r'''
#include "hud_visibility.hpp"
#include <cassert>
using namespace twilight_hd_hud;
using u8 = unsigned char;
constexpr u8 kZItemSlot=2, dItemNo_NONE_e=255;
struct daAlink_c {
    enum {BTN_Z=4};
    u8 mEquipItem=255, mSelectItemId=0, mUseButtonFlags=4;
    bool wolf=false, stage=true, ready=false;
    int action=1;
    bool checkWolf(){return wolf;}
    bool checkCastleTownUseItem(u8){return stage;}
    bool checkReadyItem(){return ready;}
    int checkNewItemChange(u8 slot){assert(slot==2);return action;}
};
struct dMeter2Draw_c {};
bool hostResult=true;
bool hostUsable(dMeter2Draw_c*,int slot){assert(slot==2);return hostResult;}
bool (*s_companionItemUsable)(dMeter2Draw_c*,int)=nullptr;
''' + helpers + r'''
bool companionDecision(daAlink_c* link, u8 item) {
    dMeter2Draw_c storage;
    auto* meter=&storage;
''' + decision + r'''
    return usable;
}
int main() {
    daAlink_c link;
    constexpr u8 bow=0x43;
    assert(third_slot_native_usable(&link,bow));
    // Native denial wins even when an unrelated probe marks the input bit.
    link.action=0;
    assert(!third_slot_native_usable(&link,bow));
    assert(!companionDecision(&link,bow));
    s_companionItemUsable=hostUsable;
    assert(!companionDecision(&link,bow));
    // An equipped, readied item needs no new equip action to remain usable.
    link.mEquipItem=bow;link.mSelectItemId=2;link.ready=true;
    assert(third_slot_native_usable(&link,bow));
    assert(companionDecision(&link,bow));
    link.stage=false;
    assert(!third_slot_native_usable(&link,bow));
    assert(!companionDecision(&link,bow));
    link.stage=true;link.mUseButtonFlags=0;
    assert(!third_slot_native_usable(&link,bow));
    // The companion host owns its native availability flag when exported.
    hostResult=false;
    assert(!companionDecision(&link,bow));
    s_companionItemUsable=nullptr;
    assert(!companionDecision(&link,bow)); // no always-usable fallback
    link.mUseButtonFlags=4;link.mSelectItemId=0;
    assert(!third_slot_native_usable(&link,bow));
    link.action=1;
    assert(third_slot_native_usable(&link,bow));
    assert(!third_slot_native_usable(&link,0));
    assert(!third_slot_native_usable(&link,255));
    link.wolf=true;
    assert(!third_slot_native_usable(&link,bow));
    assert(!companionDecision(&link,bow));
    assert(!companionDecision(nullptr,bow));
    assert(!third_slot_native_usable(nullptr,bow));
    assert(link.mEquipItem==bow&&link.mSelectItemId==0); // queries preserve equipment
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / "fade.cpp"
    binary = Path(directory) / "fade"
    cpp.write_text(fixture)
    subprocess.run(["c++", "-std=c++20", "-I", str(root / "src"), str(cpp),
                    "-o", str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print("PASS: actual R-slot and companion fade decisions, native denials, ready items and fallback")
