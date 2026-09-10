from pathlib import Path
root = Path(__file__).parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
world = (root / 'src/overworld_map_screen.inc').read_text()
assert 'overworld_map_layout::map_origin_x(g_fmapHIO.mMapScale)' in source
assert 'g_fmapHIO.mMapTopLeftPosX = 170.0f' not in source
for tag in ('f_name_1', 'f_name3', 'f_name2', 'i_name_s', 'i_name', 'i_name1'):
    assert f"MULTI_CHAR('{tag}')" in world
assert "MULTI_CHAR('hd_farea')" in world
assert 'fmap_text_source(screen, areaTags, std::size(areaTags))' not in world
assert 'candidate->isVisible()' in world
assert 'candidate->getFont() == mDoExt_getRubyFont()' in world
assert 'const char* areaName = text_box_string(areaSource)' in world
assert 'area->setFont(areaSource->getFont())' in world
assert 'area->setFont(mDoExt_getMesgFont())' not in world
assert 'area->setString(256, areaName != nullptr ? areaName : "")' in world
assert 'else area->hide();' in world
assert 'group->appendChild(area)' in world
assert 'areaNameFontSize * size' in world
assert 'areaNameX / 608.0f' in world
assert '220, 215, 137, 255' in world
print('PASS: localized warp names and shared map/marker centering')
