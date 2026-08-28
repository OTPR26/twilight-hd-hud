"""Narrow-window fitting must restore native map state after each draw."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
compact = (root / 'src/map_responsive_screen.inc').read_text()
world = (root / 'src/overworld_map_screen.inc').read_text()
assert 'size >= 1' in compact  # Approved widescreen path stays untouched.
assert 'JKR_NEW' not in compact and 'appendChild' not in compact
assert 'map->mMapScreen[0], map->mMapScreen[1], map->mFloorScreen' in compact
assert 'state.backgroundBounds = state.background->getBounds()' in compact
assert 'bounds.getWidth() * size, bounds.getHeight() * size' in compact
assert 'state.backgroundBounds.getWidth(), state.backgroundBounds.getHeight()' in compact
for field in ('pos_x', 'pos_y', 'scale'):
    assert f'map->mIconInfo[i].{field} *= size' in compact
    assert f's_dmapDrawing->mIconInfo[i].{field} = state.icons[i].{field}' in compact
assert 's_dmapDrawing->mCenterPosX = state.mirrorCenter' in compact
assert 'saved.pane->scale(saved.scaleX, saved.scaleY)' in compact
assert 'saved.pane->translate(saved.x, saved.y)' in compact
assert 'state.poeText->setFontSize(state.poeFont)' in compact
assert 'ADD_POST(DmapWideHook, after_dmap_wide' in source
assert 'begin_compact_dmap(s_dmapDrawing)' in source
assert 'restore_compact_dmap();' in source
assert 'add_dmap_back_hint(map, s_compactDmap.scale)' in source
assert 'group->scale(size, size)' in world
assert '(i == 0 ? 44 : 28) * size' in world
assert 'text->setFontSize(13 * size, 13 * size)' in world
assert 'setFontSize(s_fmapPoeFont)' in world
print('PASS: uniform narrow-window layout, matching map/marker transforms, and native state restoration')
