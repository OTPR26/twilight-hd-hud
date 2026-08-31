"""Action text scaling stays local to contextual label draws."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
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
assert 'dMeterButton_c::BUTTON_R_e' in layout
assert 'dMeterButton_c::BUTTON_BIN_e' in layout
assert "MULTI_CHAR('r_btn_b')" in layout
assert 'buttons->mpButtonR' in layout
assert 'buttons->mpButtonBin' in layout
assert 'row * 5' in layout
assert 'scalePane->scale(state.scaleX * scaleX' in layout
assert 'action_prompt_layout::kFaceCoverage' in layout
assert 'shift_action_pane(textGroup, layout.textLeft - scaledLeft, 0)' in layout
assert 'mpTextScreen' not in layout
assert 'restore_action_prompt_draw();' in source
assert 'for (auto& state : s_actionPromptDrawStates)' in source
assert 'state.scalePane->scale(state.scaleX, state.scaleY)' in source
assert 'state.buttonGroup->translate(state.buttonX, state.buttonY)' in source
assert 'state.textGroup->translate(state.textX, state.textY)' in source
context = source.split('void apply_context_button_layout(', 1)[1].split(
    'void hide_ring_stock_z_prompt(', 1)[0]
assert 'contextualRAction' in context
assert 'styled_zr_button_texture()' in context
assert 'styled_r_button_texture()' not in context
item_get = source.split('void apply_item_get_assignment_buttons(', 1)[1].split(
    'struct ItemHelpIconPosition', 1)[0]
assert 'dMsgScrnItem_c*' in item_get
assert 'item_assignment_button_texture(true)' in item_get
assert 'item_assignment_button_texture(false)' in item_get
assert 'styled_r_button_texture()' in item_get
assert 'itemGetScreen != nullptr' in source
message_draw = source.split('HookAction before_message_screen_draw(', 1)[1].split(
    'void after_meter_midna_alpha(', 1)[0]
assert 'dynamic_cast<dMsgScrnItem_c*>' in message_draw
assert 'style_item_get_text(itemScreen)' in message_draw
assert 'begin_item_prompt_font()' in message_draw
assert 'apply_item_get_assignment_buttons' in message_draw
item_get_font = source.split('void style_item_get_text(', 1)[1].split(
    'void after_item_help_message(', 1)[0]
assert 'mDoExt_getSubFont()' in item_get_font
assert 'kItemHelpBodyFontSize' in item_get_font
assert 'kItemHelpBodyLineSpace' in item_get_font
assert 'itemScreen->mpTm_c[index]' in item_get_font
assert 'itemScreen->mpTmr_c[index]' in item_get_font
assert 'reference->setFontSizeX' in item_get_font
assert 'itemGetHelp && button != ItemHelpButton::ItemX' in source
assert 'action_prompt_layout::kBottleScale' in source
assert 'scale * canvasHeight / canvasWidth' in layout
message_object_draw = source.split('HookAction before_message_object_draw(', 1)[1].split(
    'HookAction before_message_screen_draw(', 1)[0]
assert 'style_item_get_text(itemScreen, messageObject->mpRefer)' in message_object_draw
dialogue = (root / 'src/dialogue_text_screen.inc').read_text()
assert 'end_item_prompt_font()' in dialogue
assert 'apply_item_get_assignment_buttons(itemScreen)' in message_object_draw
assert 'rewrite_soup_item_get_message(messageObject->mpCtrl)' not in message_object_draw
assert 'lift_item_get_assignment_icons(itemScreen)' not in message_object_draw
assert 'ADD_PRE(MessageObjectDrawHook, before_message_object_draw' in source
assert 'three_button_soup_item_help' in source
assert 'Set it to and drink it with' in (
    root / 'src' / 'item_help_text.hpp').read_text()
assert 'ADD_POST(ItemGetMessageIndexHook, after_item_get_message_index' in source
assert 'ADD_POST(ItemGetMessageIndexDemoHook, after_item_get_message_index_demo' in source
assert 'control->pSequenceProcessor_->reset()' in source
assert 'control->pMessageText_current_ = s_itemGetHelpText.c_str()' in source
assert 's_itemGetHelpText.c_str()' in source
assert 'texture->width' in source and 'texture->height' in source
assert 'saved.icon->mSizeX = saved.width' in source
assert 'resource->getMessageText_messageEntry(control->pEntry_)' in source
assert 'lift_item_get_assignment_icons(itemScreen)' in source
assert 'restore_item_get_assignment_icons();' in (
    root / 'src' / 'dialogue_text_screen.inc').read_text()
print('PASS: contextual labels only, all text/outline layers, native size restored after draw')
