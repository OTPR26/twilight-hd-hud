from pathlib import Path
root = Path(__file__).resolve().parents[1]
source = (root / 'src/overworld_map_screen.inc').read_text()
hooks = (root / 'src/item_slot_hooks.cpp').read_text()
assert 'screen->appendChild(picture)' in source
assert 'mpBaseRoot->' not in source  # never add children to the fixed native cache
assert 'position_fmap_viewport(map->mpDraw2DBack)' in hooks
for metric in ('2DWidth', '2DHeight', '2DPosH', '2DPosV'):
    assert f'dMeter2Info_set{metric}' in source
assert 'getFmapPoeCount(' not in source  # counts/visibility stay in native draw
assert 'styled_zr_button_texture()' in source  # actual current portal binding
assert 'mpBackTex->changeTexture(background' not in hooks
assert 'ADD_POST(FmapTopDrawHook, after_fmap_top_draw' in hooks
assert 's_fmapTopDrawing = nullptr' in source
assert 'mSelectRegionNo =' not in source
hint = source.split('void add_fmap_back_hint(', 1)[1].split('void position_fmap_viewport(', 1)[0]
assert 'resource_texture(s_dmapBackDpadResource)' in hint
assert 'JGeometry::TBox2<f32>(0, 0, 24, 24)' in hint
assert 'text->setFontSize(12.5f, 12.5f)' in hint
assert "search(MULTI_CHAR('btn_i_n'))) nativeHints->hide()" in hint
assert 'overlay->appendChild(group)' in hint
assert 'mpDpadAlpha->getAlphaRate()' in hint
assert 'group->scale(size, size)' in hint  # uniform narrow-window fitting only
assert 'map_responsive_layout::scale(' in hint
assert 'group->scale(mDoGph_gInf_c::hudAspectScaleDown' not in hint
execute = hooks.split('HookAction before_menu_window_execute(', 1)[1].split(
    'void after_menu_window_execute(', 1)[0]
assert 'window->mMenuProc == dMw_c::FMAP_MOVE' in execute
assert 's_fmapBackTriggered = dungeon_map_back_requested' in execute
assert '!fmap_accepts_back(window->mpMenuFmap)' in execute
assert 'PROC_PORTAL_WARP_SELECT' in execute
assert 'field_map_suppressed_directions(' in execute
assert execute.index('s_fmapInputScope = window->mpMenuFmap') < execute.index('menu_shortcuts_active(')
status = hooks.split('HookAction before_fmap_next_status(', 1)[1].split(
    'HookAction before_dmap_next_status(', 1)[0]
assert 'pad.mPressedButtonFlags |= PAD_BUTTON_LEFT' in status
assert 'return HOOK_CONTINUE' in status and 'HOOK_SKIP_ORIGINAL' not in status
assert 's_fmapCloseOriginalTrig, PAD_BUTTON_LEFT' in status
assert 'ADD_POST(FmapNextStatusHook, after_fmap_next_status' in hooks
assert all(name in source for name in ('PROC_ALL_MAP', 'PROC_REGION_MAP', 'PROC_SPOT_MAP', 'PROC_PORTAL_WARP_MAP'))
assert 'DEFINE_HOOK(&dMsgScrnExplain_c::draw, ExplainDrawHook)' in hooks
warp_text = hooks.split('HookAction before_fmap_explain_draw(', 1)[1].split(
    'HookAction before_fmap_next_status(', 1)[0]
assert 'map->mpDraw2DTop->mpScrnExplain != screen' in warp_text
assert 'kTphdWarpTextScale = 0.76f' in warp_text
assert all(layer in warp_text for layer in ('mpTm_c', 'mpTmr_c', 'mpMg_c'))
assert 'after_fmap_explain_draw' in warp_text
assert 'ADD_PRE(ExplainDrawHook, before_fmap_explain_draw' in hooks
assert 'ADD_POST(ExplainDrawHook, after_fmap_explain_draw' in hooks
print('PASS: native counts/control binding retained, frame caches safe, projection bounds synchronized')
