from pathlib import Path


root = Path(__file__).resolve().parent.parent
source = (root / "src/item_slot_hooks.cpp").read_text()

overlay = source.split("void ensure_golden_bugs_overlay(", 1)[1].split(
    "void after_insect_create(", 1
)[0]
wait = source.split("HookAction before_insect_wait_move(", 1)[1].split(
    "void ensure_golden_bugs_overlay(", 1
)[0]
move = source.split("void after_insect_move(", 1)[1].split(
    "void after_collect_move(", 1
)[0]
bounds = source.split("JGeometry::TBox2<f32> golden_bug_slot_bounds(", 1)[1].split(
    "void position_golden_bug_cursor(", 1
)[0]

assert "position_golden_bug_cursor(menu, group)" in overlay
assert "position_cursor_outside_frame(menu->mpDrawCursor, frame, 4.0f, 3.0f)" in source
assert "menu->setAButtonString(0)" in overlay
assert '"Details"' not in overlay
assert "hd_gbqp" in overlay and '"Give this bug?"' in overlay
assert "hd_gbat" in overlay and '"Confirm"' in overlay
assert "golden-bug-male.bti" in source and "golden-bug-female.bti" in source
assert "clean_golden_bug_description" not in source
assert "before_golden_bugs_screen_draw" in source
assert "screen == s_activeGoldenBugsDrawMenu->mpExpScreen" in source
assert "s_goldenBugsExplainAlphaSuppressed" in source
assert "after_insect_draw" in source
assert "mConfirmOptionPosX_4x3 + 550.0f" in source
assert "mConfirmOptionPosY_4x3 + 187.0f" in source
assert "s_goldenBugsChoiceDraw ? 212.0f : 156.0f" in source
assert "s_goldenBugsChoiceDraw ? 0.5f : 4.0f" in source
assert "MULTI_CHAR('hd_gc00')" in overlay
assert "resource_texture(s_goldenBugConnectorResource)" in overlay
assert "position_cursor_outside_frame(menu->mpDrawCursor, frame, -3.0f, -3.0f)" in source
assert "question->setFont(mDoExt_getMesgFont())" in overlay
assert "make_hd_prompt_label(MULTI_CHAR('hd_gbat')" in overlay
assert "make_hd_prompt_label(MULTI_CHAR('hd_gbbt')" in overlay
assert "resource_texture(s_promptConfirmLabelResource)" in source
assert "resource_texture(s_promptBackLabelResource)" in source
assert "description->setFontSize(11.5f, 11.5f)" in overlay
assert "menu->cursorMove()" in wait
assert "if (menu->field_0xf6 == 1) return HOOK_CONTINUE;" in wait
assert "getTrigA" not in wait
assert "HOOK_SKIP_ORIGINAL" in wait
assert "field_0xf3 = 1" not in move
assert "{-4.0f, 60.0f, 144.0f, 208.0f, 292.0f, 356.0f}" in bounds
assert "update_menu_face_button(menu->mpScreen, MULTI_CHAR('hd_gbab'), true)" in overlay
assert "update_menu_face_button(menu->mpScreen, MULTI_CHAR('hd_gbbb'), false)" in overlay
assert "fit_collection_submenu_overlay(group, MULTI_CHAR('hd_gbbg')" in overlay
assert 'collection_submenu_global_bounds(slot)' in move

for name in ("golden-bug-male.bti", "golden-bug-female.bti"):
    data = (root / "res/menu" / name).read_bytes()
    assert data[0] == 6
    assert int.from_bytes(data[2:4], "big") == 32
    assert int.from_bytes(data[4:6], "big") == 32

connector = (root / "res/menu/golden-bug-connector.bti").read_bytes()
assert connector[0] == 6
assert int.from_bytes(connector[2:4], "big") == 64
assert int.from_bytes(connector[4:6], "big") == 48

for name, width in (("prompt-label-back.bti", 128),
                    ("prompt-label-confirm.bti", 192)):
    data = (root / "res/menu" / name).read_bytes()
    assert data[0] == 6
    assert int.from_bytes(data[2:4], "big") == width
    assert int.from_bytes(data[4:6], "big") == 48

font_override = (root / "src/font_override.cpp").read_text()
assert "kMaleSymbolCode = 0xB2" in font_override
assert "kFemaleSymbolCode = 0xB3" in font_override
assert "code == kMaleSymbolCode || code == kFemaleSymbolCode" in font_override

print("PASS: Golden Bugs uses the TPHD gift layout and preserves native sex-symbol glyphs")
