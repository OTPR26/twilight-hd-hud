from pathlib import Path


root = Path(__file__).resolve().parent.parent
source = (root / "src/item_slot_hooks.cpp").read_text()

overlay = source.split("void ensure_hidden_skills_overlay(", 1)[1].split(
    "HookAction before_skill_wait_move(", 1
)[0]
cursor = source.split("void position_hidden_skill_cursor(", 1)[1].split(
    "void ensure_hidden_skills_overlay(", 1
)[0]
wait = source.split("HookAction before_skill_wait_move(", 1)[1].split(
    "void after_skill_create(", 1
)[0]
move = source.split("void after_skill_move(", 1)[1].split(
    "constexpr const char* kGoldenBugTextureNames", 1
)[0]

assert "menu->mSkillNum" in overlay
assert "menu->mpFTagPicture[index][0]" in overlay
assert "menu->mpFTagPicture[index][2]" in overlay
assert "position_hidden_skill_cursor(menu, group)" in overlay
assert "position_cursor_outside_frame(menu->mpDrawCursor, frame, 4.0f, 3.0f)" in cursor
assert "menu->setAButtonString(0)" in overlay
assert "kHiddenSkillDescriptionIds[selected]" in overlay
assert "s_hiddenSkillPanelResources[selected]" in overlay
assert "update_hidden_skill_inline_icons(menu, group, description)" in overlay
assert "hidden_skill_inline_icon_texture" in source
assert "styled_zl_button_texture()" in source
assert "styled_r_button_texture()" in source
assert "menu_face_button_texture(true)" in source
assert "menu_face_button_texture(false)" in source
assert "update_menu_face_button(menu->mpMenuScreen, MULTI_CHAR('hd_hsab'), true)" in overlay
assert "update_menu_face_button(menu->mpMenuScreen, MULTI_CHAR('hd_hsbb'), false)" in overlay
assert "fit_collection_submenu_overlay(group, MULTI_CHAR('hd_hsbg')" in overlay
assert 'collection_submenu_global_bounds(row)' in move
assert "JGeometry::TBox2<f32>(-82.0f, top, 178.0f, top + 42.0f)" in source
assert "getTrigA" not in wait
assert "mDoCPd_c::getHoldUp(PAD_1)" in wait
assert "mDoCPd_c::getHoldDown(PAD_1)" in wait
assert "dpadUpTrig" in wait and "dpadDownTrig" in wait
assert "!dpadUpHeld" in wait and "!dpadDownHeld" in wait
assert "HOOK_SKIP_ORIGINAL" in wait
assert "s_menuPointerConsumeClick()" in move
assert "after_skill_draw" not in source
assert 'ADD_POST(SkillDrawHook' not in source

names = (
    "ending-blow.bti",
    "shield-attack.bti",
    "back-slice.bti",
    "helm-splitter.bti",
    "mortal-draw.bti",
    "jump-strike.bti",
    "great-spin.bti",
)
for name in names:
    data = (root / "res/menu/hidden-skills" / name).read_bytes()
    assert data[0] == 6
    assert int.from_bytes(data[2:4], "big") == 328
    assert int.from_bytes(data[4:6], "big") == 200

assert "hidden-skill-scroll" not in source

print("PASS: Hidden Skills uses the clean parchment, TPHD panels, localized inline icons, and inert A input")
