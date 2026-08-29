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
assert "hd_gbat" not in overlay and '"Details"' not in overlay
assert "golden-bug-male.bti" in source and "golden-bug-female.bti" in source
assert "clean_golden_bug_description(description, female)" in overlay
assert "menu->cursorMove()" in wait
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

print("PASS: Golden Bugs cursor follows HD slots, A is inert, and sex symbols are explicit art")
