"""Check that map touch input is scoped and restored around native processing."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/item_slot_hooks.cpp').read_text()
read = source.split('void after_pad_read(', 1)[1].split('HookAction before_meter_map_ctrl_show(', 1)[0]
assert read.index('s_mapTouchPortals.observe(') < read.index('if (fixedTphdBindings) {')
assert 's_touchInput.l_held()' in read
assert 'dMeter2Info_getWindowStatus() == 4' in read
assert '!s_inputGate.blocked()' in read

move = source.split('HookAction before_fmap_move(', 1)[1].split('HookAction before_fmap_draw(', 1)[0]
assert 's_mapLeftHeld || s_mapTouchPortals.held()' in move
assert 's_mapLeftPressed || touchPressed' in move
assert 's_mapTouchPortals.take_pressed()' in move
assert 'restore_menu_shortcut_buttons' in move

assert 'status == 4 || status == 5' in source
assert 'ADD_PRE(TouchSyncVisualStateHook, before_touch_sync_visual_state' in source
assert 'ADD_PRE(TouchSetControlOverrideHook, before_touch_set_control_override' in source
assert 's_mapTouchPortals.clear();' in source
assert 'TouchSetControlOverrideHook::g_orig(dusk::ui::Control::L, s_hostTouchLOverride)' in source
print('Map touch portal hooks, map scope, and shutdown restoration passed')
