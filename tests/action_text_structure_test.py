"""Action text scaling stays local to contextual label draws."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/item_slot_hooks.cpp').read_text()
scaling = source.split('void scale_action_text_for_draw(', 1)[1].split(
    'void after_meter_button_draw(', 1)[0]
assert 'buttons->mpTextBox[i]' in scaling
assert 'action_text_multiplier(hud_scales().actionText)' in scaling
assert 'text->getFontSize(state.font)' in scaling
assert 'text->setCharSpace(state.charSpace * factor)' in scaling
assert 'mpTextScreen' not in scaling and 'mpButtonA' not in scaling
restoration = source.split('void restore_action_text_draw(', 1)[1].split(
    'void scale_action_text_for_draw(', 1)[0]
assert 'state.text->setFontSize(state.font)' in restoration
assert 'state.text->setCharSpace(state.charSpace)' in restoration
assert 'state = {}' in restoration
assert 'ADD_POST(MeterButtonDrawHook, after_meter_button_draw' in source
layout = source.split('void arrange_action_prompt_for_draw(', 1)[1].split(
    'void restore_action_text_draw(', 1)[0]
assert 'dMeterButton_c::BUTTON_A_e' in layout
assert 'row * 5' in layout
assert 'picture->scale(state.scaleX * action_prompt_layout::kButtonScale' in layout
assert 'shift_action_pane(textGroup, layout.textLeft - scaledLeft, 0)' in layout
assert 'mpTextScreen' not in layout
assert 'restore_action_prompt_draw();' in source
assert 'state.picture->scale(state.scaleX, state.scaleY)' in source
assert 'state.buttonGroup->translate(state.buttonX, state.buttonY)' in source
assert 'state.textGroup->translate(state.textX, state.textY)' in source
print('PASS: contextual labels only, all text/outline layers, native size restored after draw')
