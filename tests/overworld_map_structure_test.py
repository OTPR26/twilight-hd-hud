from source_helpers import read_hook_source
from pathlib import Path
root = Path(__file__).resolve().parents[1]
source = (root / 'src/overworld_map_screen.inc').read_text()
hooks = read_hook_source()
assert 'screen->appendChild(picture)' in source
assert 'mpBaseRoot->' not in source  # never add children to the fixed native cache
assert 'style_fmap_cursor(map);' in source
# Styling must not resize native clipping panes or change map-layer transforms.
for forbidden in ('position_fmap_viewport', 'sync_fmap_projection',
                  'FmapCreateHook', 'FmapViewportWidthHook', 'mMapTopLeftPos',
                  'mpMapBlack', 'calcRenderingScale(',
                  'calcRenderingPos(', 'mRegionMinMap', 'mRegionMapSize',
                  'mpMenuFmapMap->field_0x20', 'mpMenuFmapMap->field_0x22'):
    assert forbidden not in source + hooks, forbidden
assert 'map->mpMapArea->getGlobalVtx(' in source
assert 'frame_for_content({minX, minY, maxX - minX, maxY - minY})' in source
assert 'picture->resize(outer.width, outer.height)' in source
assert 'picture->move(outer.x, outer.y)' in source
assert 'ADD_POST(FmapBackDrawHook, after_fmap_back_draw' in hooks
registration = hooks.split('// Resolve the same native functions without MSVC', 1)[1].split(
    'DEFINE_HOOK(&J2DGrafContext::setScissor', 1)[0]
assert 'DEFINE_HOOK_SYMBOL("dMenu_Fmap2DBack_c::draw", void(dMenu_Fmap2DBack_c*), FmapBackDrawHook)' in registration
assert 'DEFINE_HOOK_SYMBOL("dMenu_Fmap2DBack_c::regionTextureDraw", void(dMenu_Fmap2DBack_c*), FmapRegionDrawHook)' in registration
assert '#else\nDEFINE_HOOK(&dMenu_Fmap2DBack_c::draw, FmapBackDrawHook);' in registration
assert 'DEFINE_HOOK(&dMenu_Fmap2DBack_c::regionTextureDraw, FmapRegionDrawHook);\n#endif' in registration
assert 'ADD_PRE(FmapRegionDrawHook, before_fmap_region_draw' in hooks
assert 'ADD_POST(FmapScissorHook, after_fmap_scissor' in hooks
assert 'GXSetProjectionv(s_fmapPresentation.projection)' in source
assert 'picture == menu->mpDraw2DBack->mpBackTex' in source
assert 'overworld_map_layout::presentation_fit(' in source
assert 'overworld_map_layout::reference_content(' in source
presentation = source.split('HookAction before_fmap_region_draw(', 1)[1].split('void style_fmap_portal(', 1)[0]
assert presentation.index('GXSetProjectionv(projection)') < presentation.index('J2DFillBox(')
assert presentation.index('after_fmap_scissor(') < presentation.index('J2DFillBox(')
assert 'J2DFillBox(tl.x + map->mTransX, tl.y + map->mTransZ,' in presentation
assert 'br.x - tl.x, br.y - tl.y' in presentation
assert 'JUtility::TColor(0, 0, 0,' in presentation
assert 'std::clamp(map->mAlphaRate, 0.0f, 1.0f)' in presentation
frame_style = source.split('void style_fmap_frame(', 1)[1].split('void after_fmap_back_draw(', 1)[0]
assert 'picture->hide();' in frame_style
foreground = source.split('void after_fmap_back_draw(', 1)[1].split('void refine_fmap_top(', 1)[0]
assert 'picture->draw(' not in foreground
assert 'overworld_map_layout::borderPieces' in foreground
assert foreground.index('picture->show();') < foreground.index('picture->drawOut(') < foreground.index('picture->hide();')
assert 'MTXIdentity(identity);' in foreground
assert 'i == 0 ? 0.181f : i == 1 ? zoomAFraction : zoomBFraction' in source
assert 'set_text_binding(text, i == 0 ? HBIND_LEFT : HBIND_RIGHT, VBIND_CENTER)' in source
assert 'x - (115 + zoomLabelInset) * size' in source
assert 'naturalWidth * fit' not in source
assert 'zoomPromptGap' not in source
assert 'overworld_map_layout::transition_bounds(' in foreground
assert 'map->mTransX, map->mTransZ' in foreground
assert foreground.count('map->mTransX') == 1 and foreground.count('map->mTransZ') == 1
assert 'std::clamp(map->mAlphaRate, 0.0f, 1.0f)' in foreground
assert 'bounds.x + part.x * sx, bounds.y + part.y * sy' in foreground
for forbidden in ('mpMapArea->resize', 'mpMapArea->scale', 'mpMapArea->translate',
                  'mpMapArea->getPanePtr()', 'fMapBackWide()'):
    assert forbidden not in source
for metric in ('2DWidth', '2DHeight', '2DPosH', '2DPosV'):
    assert f'dMeter2Info_set{metric}' not in source
assert 'getFmapPoeCount(' not in source  # counts/visibility stay in native draw
assert 'styled_l_button_texture()' in source  # L/LB/L1 portal binding
assert 'ADD_POST(FmapMoveHook, after_fmap_move' in hooks
portal_input = hooks.split('HookAction before_fmap_move(', 1)[1].split('HookAction before_fmap_draw(', 1)[0]
assert 'PAD_TRIGGER_R' not in portal_input
assert 's_mapLeftPressed' in portal_input
assert 'restore_menu_shortcut_buttons' in portal_input
assert 'mpBackTex->changeTexture(background' not in hooks
assert 'ADD_POST(FmapTopDrawHook, after_fmap_top_draw' in hooks
assert 's_fmapTopDrawing = nullptr' in source
assert 'mSelectRegionNo =' not in source
hint = source.split('void add_fmap_back_hint(', 1)[1].split('void style_fmap_cursor(', 1)[0]
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
print('PASS: native map-layer geometry untouched; custom styling and controls retained')
