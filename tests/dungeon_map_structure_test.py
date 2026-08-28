"""Prevent late child insertion into native fixed-size pane animation caches."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/item_slot_hooks.cpp').read_text()
floors = source.split('void style_dmap_floor_rows(', 1)[1].split('void style_dmap_frame(', 1)[0]
frame = source.split('void style_dmap_frame(', 1)[1].split('void add_dmap_edge_rules(', 1)[0]
rules = source.split('void add_dmap_edge_rules(', 1)[1].split('void position_dmap_native_groups(', 1)[0]
for section in (floors, frame):
    assert 'JKR_NEW' not in section
    assert 'appendChild(' not in section
    assert 'insertChild(' not in section
assert "MULTI_CHAR('f_base_0')" in floors
assert "MULTI_CHAR('bs_00_0')" in frame
assert 'auto* root = map->mBaseScreen;' in rules
assert "search(MULTI_CHAR('ROOT'))" not in rules
assert 'field_0xd9c' in rules
banner = source.split('if (titleGroup == nullptr && titleSource != nullptr)', 1)[1].split(
    'if (promptGroup == nullptr && titleSource != nullptr)', 1)[0]
assert 'dComIfGp_getDmapResArchive()' in banner
assert '"tt_kazari_kani_00.bti"' in banner
assert "search(MULTI_CHAR('moyou'))" not in banner
assert 'collect_archive_texture(' not in banner
assert 'banner_motif(tile)' in banner
assert 'titleGroup->appendChild(decoration)' in banner
hint = source.split('void add_dmap_back_hint(', 1)[1].split('void apply_dmap_hd_layout(', 1)[0]
assert 'auto* root = map->mBaseScreen;' in hint
assert 'field_0xd9c' in hint and 'back_hint(' in hint
assert 'getGlobalVtx(frame, nullptr' not in hint
assert 'getGlobalVtx(frame, &matrix' in hint
execute = source.split('HookAction before_menu_window_execute(', 1)[1].split(
    'void after_menu_window_execute(', 1)[0]
assert execute.index('dungeon_map_navigation_buttons(') < execute.index('menu_shortcuts_active(')
assert 'window->mMenuProc == dMw_c::DMAP_MOVE' in execute
assert 'pad.mPressedButtonFlags, PAD_BUTTON_UP' in execute
assert 'DmapNextStatusHook, before_dmap_next_status' in source
print('PASS: native dungeon-map animation hierarchies remain unchanged')
