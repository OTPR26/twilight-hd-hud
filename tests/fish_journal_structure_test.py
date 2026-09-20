from source_helpers import read_hook_source
"""Guard the first TPHD-style Fish Journal pass and save-backed records."""
from pathlib import Path

source = read_hook_source()
body = source.split('struct FishJournalState', 1)[1].split(
    'void after_skill_create(', 1)[0]
move = source.split('void after_fishing_move(', 1)[1].split(
    'void after_skill_create(', 1)[0]
assert 'DEFINE_HOOK(&dMenu_Fishing_c::_move, FishingMoveHook);' in source
assert 'ADD_POST(FishingMoveHook, after_fishing_move' in source
create = source.split('void after_fishing_create(', 1)[1].split(
    'struct FishJournalState', 1)[0]
assert 'ensure_fish_journal_overlay(menu);' in create
for direction in ('getTrigUp', 'getTrigDown', 'getTrigLeft', 'getTrigRight'):
    assert direction in body
for direction in ('checkUpTrigger', 'checkDownTrigger', 'checkLeftTrigger',
                  'checkRightTrigger'):
    assert f'menu->mpStick->{direction}()' in body
assert 'caught_fish_in_direction' in body
assert 'if (up)' in body and '} else if (down)' in body
assert '} else if (left)' in body and '} else if (right)' in body
assert 'dComIfGs_getFishNum(index) != 0' in body
assert 'dComIfGs_getFishSize(selected)' in body
assert 'dComIfGs_addFishNum' not in body and 'dComIfGs_setFishSize' not in body
assert 'No. Caught' in body and 'Largest' in body
assert "MULTI_CHAR('hd_fjcv')" in body and "MULTI_CHAR('hd_fjsv')" in body
assert 'constexpr int kFishBookSlots[MAX_FINDABLE_FISHES] = {1, 5, 4, 2, 3, 0};' in body
assert 'const int column = slot & 1;' in body
assert 'const int row = slot >> 1;' in body
assert 'fit_fish_artwork(listPicture, listLeft, listTop, 85.0f, 38.0f)' in body
assert 'fit_fish_artwork(detail, 430.0f, 103.0f, 110.0f, 60.0f)' in body
assert 'resource_texture(s_fishArtworkResources[index])' in body
assert 'for (auto& resource : s_fishArtworkResources) free_resource(resource)' in source
assert "MULTI_CHAR('hd_fjbk')" in body
assert "MULTI_CHAR('hd_fjsm')" in body
assert "MULTI_CHAR('hd_fjsc')" in body
assert 's_fishJournalSelectionResource' in source
assert "MULTI_CHAR('hd_fjbn')" in body and 'localized_label(MenuLabel::FishJournal)' in body
assert 'nativeTitle->hide();' in body
assert 'menu->mpIconScreen->hide();' in body
assert "MULTI_CHAR('hd_fjbb')" in body and "MULTI_CHAR('hd_fjbt')" in body
assert "MULTI_CHAR('hd_fjab')" in body
assert "update_menu_face_button(menu->mpScreen, MULTI_CHAR('hd_fjab'), true)" in body
assert "update_menu_face_button(menu->mpScreen, MULTI_CHAR('hd_fjbb'), false)" in body
assert "MULTI_CHAR('hd_fjbf')" in body and "MULTI_CHAR('hd_fjnf')" in body
assert "MULTI_CHAR('hd_fjdv')" in body
assert "MULTI_CHAR('hd_fjv0')" in body
assert "MULTI_CHAR('hd_fjbg')" in body
assert "MULTI_CHAR('hd_fjru')" in body and "MULTI_CHAR('hd_fjrl')" in body
assert 's_menuPointerBeginContext(dusk::menu_pointer::Context::Collection)' in move
assert 'pointer->x < bounds.i.x - padding' in move
assert 's_menuPointerSetHoverTarget' in move
assert 's_menuPointerConsumeClick()' in move
assert 'resolve_fish_journal_pointer_functions();' in source
assert 'fish\\nLargest' not in body
assert 'menu->mpFishNameString[selected]' in body
assert 'source->getTexture(0)' in body and 'texture->getTexInfo()' in body
assert 'index == s_fishJournal.selected' in body
assert 'missingTextureWarningMask' in body
assert 'Fish Journal texture unavailable for species %d; retrying' in body
assert body.index("if (group == nullptr) {") < body.index(
    "const u64 detailTag = MULTI_CHAR('hd_fd00') + index;")
assert 'group->search(detailTag) != nullptr' in body
assert 'group->search(listTag) != nullptr' in body
assert 'group->search(nameTag) != nullptr' in body
assert 'ensure_fish_journal_overlay(menu);' in source.split(
    'HookAction before_fishing_draw(', 1)[1].split(
    'HookAction before_skill_draw(', 1)[0]
assert 'ensure_fish_journal_overlay(menu);' in move
assert "fit_collection_submenu_overlay(group, MULTI_CHAR('hd_fjbg')" in body
assert 'collection_submenu_global_bounds(fish)' in move
print('PASS: caught-only selection, HD artwork with native fallback, and read-only records')

import struct
root = Path(__file__).resolve().parents[1]
for name in ('hyrule-bass', 'hylian-loach', 'hylian-pike', 'ordon-catfish', 'reekfish', 'greengill'):
    data = (root / 'res/menu/fish' / (name + '.bti')).read_bytes()
    width, height = struct.unpack_from('>HH', data, 2)
    assert data[0] == 6 and width == 512 and height % 4 == 0
    assert len(data) == 32 + width * height * 4
    alpha = [data[tile + i * 2] for tile in range(32, len(data), 64) for i in range(16)]
    assert min(alpha) == 0 and max(alpha) >= 250
print('PASS: all six RGBA8 fish assets have high-resolution dimensions and alpha')
