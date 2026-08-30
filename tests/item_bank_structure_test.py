"""Keep Items independent of transient system-bar insets and prompt distortion."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_bank_screen.inc').read_text()
hooks = (root / 'src/item_slot_hooks.cpp').read_text()
assert 'getSafe' not in source
assert 'current_collection_viewport()' not in source
assert source.count('auto viewport = item_bank_content_viewport();') == 2
for edge in ('MinXF', 'MinYF', 'MaxXF', 'MaxYF'):
    assert f'get{edge}()' in source
viewport = source.split('CollectionViewport item_bank_viewport(', 1)[1].split('int item_bank_cell(', 1)[0]
assert 'ring->mCenterPosX' in viewport and 'ring->mCenterPosY' in viewport
dimmer = source.split('ring->mpBlackTex->draw(', 1)[1].split('auto* root', 1)[0]
assert 'ring->mCenterPos' not in dimmer
assert 'bank_assignment_prompt(i, textures[i]->width, textures[i]->height)' in source
assert 'button->resize(bounds.width, bounds.height)' in source
assert 'button->move(bounds.x, bounds.y)' in source
assert 'void refresh_ring_z_assignment_texture(dMenu_Ring_c* ring)' in hooks
assert 'ring->setSelectItem(kZItemSlot, item)' in hooks
assert hooks.count('refresh_ring_z_assignment_texture(ring);') >= 3
print('PASS: Items full-surface viewport/dimmer, native transitions, and uniform button fitting')
