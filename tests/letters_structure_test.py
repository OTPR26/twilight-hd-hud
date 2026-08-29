from pathlib import Path
import struct


root = Path(__file__).resolve().parent.parent
source = (root / "src/item_slot_hooks.cpp").read_text()

overlay = source.split("void ensure_letters_overlay(", 2)[2].split(
    "HookAction before_letter_wait_move(", 1
)[0]
cursor = source.split("void position_letters_cursor(", 1)[1].split(
    "void ensure_letters_overlay(", 1
)[0]
wait = source.split("HookAction before_letter_wait_move(", 1)[1].split(
    "void after_letter_move(", 1
)[0]

assert '"Letters"' in overlay
assert '"ni_item_icon_letter.bti"' in overlay
assert "dMenu_Letter::getLetterSubject(letter)" in overlay
assert "dMenu_Letter::getLetterName(letter)" in overlay
assert "dMenu_Letter::getLetterText(letter)" in overlay
assert "getStringPage" in overlay
assert "dComIfGs_onLetterReadFlag(letter)" in overlay
assert "menu->mpIconScreen->hide()" in overlay
assert "position_letters_cursor(menu, group)" in overlay
assert "position_cursor_outside_frame(menu->mpDrawCursor, frame, 4.0f, 3.0f)" in cursor
assert "getTrigA" not in wait
assert "getTrigL" not in wait and "getTrigR" not in wait
assert "physical_button_held(kSdlLeftShoulderButton)" in wait
assert "physical_button_held(kSdlRightShoulderButton)" in wait
assert "contentPage < s_lettersOverlay.contentPages" in wait
assert "getTrigB" in wait
assert "checkUpTrigger" in wait and "checkDownTrigger" in wait
assert "dpadUpTrig" in wait and "dpadDownTrig" in wait
assert "mDoCPd_c::getHoldUp(PAD_1)" in wait
assert "mDoCPd_c::getHoldDown(PAD_1)" in wait
assert "!dpadUpHeld" in wait and "!dpadDownHeld" in wait
assert "selectedSlot" in wait and "scrollOffset" in wait
assert "letters_page_count" not in source
assert "s_lettersScrollArrowResource" in overlay
assert "s_lettersScrollStateResources" in overlay
assert "s_lettersCornerResource" in overlay
assert "MULTI_CHAR('hd_ltsa')" in overlay
assert "MULTI_CHAR('hd_ltsb')" in overlay
assert "MULTI_CHAR('hd_lu00') + stop" in overlay
assert "resource_texture(s_lettersScrollStateResources[stop])" in overlay
assert "activeStop" in overlay
assert "thumb->place" not in overlay
assert 'subject->setString(128, "")' in overlay
assert '"Page %d/%d"' in overlay
assert "styled_l_button_texture()" in overlay
assert "styled_r_button_texture()" in overlay
assert "update_menu_face_button(menu->mpBaseScreen, MULTI_CHAR('hd_ltab'), true)" in overlay
assert "update_menu_face_button(menu->mpBaseScreen, MULTI_CHAR('hd_ltbb'), false)" in overlay
assert "fit_collection_submenu_overlay(group, MULTI_CHAR('hd_ltbg')" in overlay
assert 'collection_submenu_global_bounds(row)' in source.split(
    'void after_letter_move(', 1)[1].split(
    'constexpr u32 kHiddenSkillDescriptionIds', 1)[0]
assert "HOOK_SKIP_ORIGINAL" in wait
assert "ADD_POST(LetterMoveHook, after_letter_move" in source
assert "ADD_PRE(LetterWaitMoveHook, before_letter_wait_move" in source
assert "hidden-skill-scroll" not in source

assets = {
    "letters-corner.bti": (96, 96),
    "letters-scroll-arrow.bti": (24, 24),
}
assets.update({f"letters-scroll-state-{stop}.bti": (16, 256) for stop in range(11)})
for name, dimensions in assets.items():
    data = (root / "res/menu" / name).read_bytes()
    assert data[:2] == bytes((6, 1))
    assert struct.unpack_from(">HH", data, 2) == dimensions
    assert len(data) == 32 + dimensions[0] * dimensions[1] * 4
    assert any(data[offset] == 0 for offset in range(32, len(data), 2))

print("PASS: Letters is a persistent localized journal with continuous scrolling and unified input")
