from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
collection = (root / 'src/collection_screen.inc').read_text()
navigation = source.split('HookAction before_option_vibration_move(', 1)[1].split(
    'HookAction before_option_screen_draw(', 1)[0]
assert 'STControl probe = *menu->mpStick' in navigation
assert '!probe.checkUpTrigger()' in navigation
assert '++menu->field_0x3ef' in navigation
assert 'HOOK_SKIP_ORIGINAL' in navigation
screen = source.split('HookAction before_option_screen_draw(', 1)[1].split(
    'HookAction before_option_draw(', 1)[0]
assert 's_drawingOptionMenu->mpBackScreen' in screen
assert 'apply_option_hd_style(s_drawingOptionMenu)' in screen
draw = source.split('HookAction before_option_draw(', 1)[1].split(
    'void after_option_draw(', 1)[0]
assert 'apply_option_hd_style(' not in draw
active = source.split('bool option_confirmation_active(', 1)[1].split(
    'bool fit_option_warning_frame(', 1)[0]
assert 'PROC_CONFIRM_' not in active
assert 'cursor->setPos(centerX, centerY, target, false)' in collection
assert 'cursor->mpPaneMgr->translate(centerX, centerY)' in collection
print('PASS: Options navigation, post-animation layout, and live Collection cursor anchoring')
