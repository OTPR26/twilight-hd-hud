"""Ensure gameplay input and minimap restoration stay scoped to their owners."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/item_slot_hooks.cpp').read_text()
pad = source.split('void after_pad_read(', 1)[1].split('HookAction before_meter_map_ctrl_show(', 1)[0]
assert 'daAlink_getAlinkActorClass() != nullptr, s_fileSelectScreenActive' in pad
assert 'use_tphd_dpad_map_bindings() && gameplayShortcuts' in pad
assert 'use_tphd_midna_binding() && gameplayShortcuts' in pad
assert pad.index('gameplay_shortcuts_active(') < pad.index('pad.mButtonFlags &= ~consumedMapMask')
before = source.split('HookAction before_menu_window_execute(', 1)[1].split('void after_menu_window_execute(', 1)[0]
after = source.split('void after_menu_window_execute(', 1)[1].split('HookAction before_check_item_button_change(', 1)[0]
assert 'preserve_map_minimap_preference(window)' in before
assert 'preserve_map_minimap_preference(mods::arg<dMw_c*>(args, 0))' in after
preserve = source.split('void preserve_map_minimap_preference(', 1)[1].split('HookAction before_menu_window_execute(', 1)[0]
assert 'opening && dMeter2Info_getMapStatus() == 2' in preserve
assert 'window != s_minimapReturnWindow' in preserve
assert 'meter != s_minimapReturnMeter' in preserve
assert 's_minimapReturnState.begin(dComIfGp_checkMapShow())' in preserve
for kind in ('FMAP', 'DMAP'):
    for phase in ('OPEN', 'CLOSE'):
        assert f'dMw_c::{kind}_{phase}' in preserve
assert 'dMeter2Info_setMapStatus(restore->visible ? 1 : 0)' in preserve
assert 'if (restore->preference) dComIfGp_mapShow();' in preserve
assert 'else dComIfGp_mapHide();' in preserve
assert 'setInOutDir' not in preserve and 'seStart' not in preserve
print('PASS: file-select input isolation and one-shot minimap restore for both maps')
