"""Keep feature ownership separate when installing hooks and drawing shared HUDs."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
install = source.split('ModResult install_item_slot_hooks(', 1)[1]
owned = {
    'ThirdItemSlot': '''before_get_select_item after_set_select_item
        before_item_action_trigger after_set_stick_data after_item_help_message
        after_item_get_message_index after_item_get_message_index_demo
        before_meter_button_execute before_meter_draw_button_z after_meter_draw_button_z
        before_ring_set_active_cursor after_ring_set_active_cursor after_ring_set_mix_message
        before_ring_is_mix_item_on before_ring_is_mix_item_off before_midna_talk_trigger
        before_check_item_button_change before_check_item_change_from_button
        before_check_set_item_trigger before_check_item_set_button before_set_heavy_boots
        after_player_execute before_item_help_out_font_draw after_item_help_out_font_draw
        after_meter_midna_alpha''',
    'CollectionScreen': '''after_collect_create before_collection_cursor
        before_collection_model_move after_collection_model_move before_collection_link_pose
        before_collection_projection before_collection_position before_collection_icon_measure
        after_collection_icon_measure before_collection_icon_render after_collection_icon_render
        before_collection_text_parse after_collection_text_parse before_collection_text_escape
        after_collection_text_escape before_collection_out_font_draw after_collection_wallet_message
        after_collect_move before_collect_draw before_collect_delete after_collect_wide''',
    'DpadShortcuts': '''before_meter_draw_button_cross after_meter_draw_button_cross
        after_meter_move_button_cross before_meter_map_ctrl_show before_fmap_next_status
        after_fmap_next_status before_dmap_next_status''',
}
for feature, callbacks in owned.items():
    blocks = re.findall(r'if \(feature_enabled\(Feature::' + feature + r'\)\) \{([^{}]*)\}', install)
    for callback in callbacks.split():
        assert sum(', ' + callback + ',' in block for block in blocks) == 1, callback

guards = {
    'ThirdItemSlot': '''apply_wii_u_r_button_art create_ring_z_prompt draw_ring_z_prompt
        style_ring_direct_select_prompt style_ring_combo_prompt draw_z_hud_item_meters
        update_z_hud_item position_midna_hud update_midna_shoulder_badge
        hide_ring_stock_z_prompt hide_legacy_overlay_z apply_item_explain_button_layout''',
    'DpadShortcuts': 'apply_wii_u_dpad_transform apply_wii_u_dpad_style draw_tphd_map_icon',
}
for feature, functions in guards.items():
    for function in functions.split():
        assert re.search(r'void ' + function + r'\([^)]*\) \{\s*'
                         r'if \(!feature_enabled\(Feature::' + feature + r'\)\) return;', source), function

pad = source.split('void after_pad_read(', 1)[1].split('HookAction before_meter_map_ctrl_show(', 1)[0]
assert pad.index('if (!feature_enabled(Feature::ThirdItemSlot)) return;') < pad.index('const bool leftShoulderHeld')
menu = source.split('HookAction before_menu_window_execute(', 1)[1].split('void after_menu_window_execute(', 1)[0]
assert menu.index('if (!feature_enabled(Feature::DpadShortcuts)) return HOOK_CONTINUE;') < menu.index('pad.mButtonFlags =')
for filename, function in [('overworld_map_screen.inc', 'add_fmap_back_hint'),
                           ('dungeon_map_screen.inc', 'add_dmap_back_hint')]:
    text = (root / 'src' / filename).read_text()
    assert re.search(r'void ' + function + r'\([^)]*\) \{\s*'
                     r'if \(!feature_enabled\(Feature::DpadShortcuts\)\) return;', text)
midna = source.split('void position_midna_hud(', 1)[1].split('void update_midna_shoulder_badge(', 1)[0]
native_midna = source.split('void align_native_midna_hud(', 1)[1].split('void position_midna_hud(', 1)[0]
assert 'if (feature_enabled(Feature::ThirdItemSlot)) return;' in native_midna
assert 'paneTrans(g_drawHIO.mMidnaIconPosX, g_drawHIO.mMidnaIconPosY)' in native_midna
assert 'shoulder.x - face.x, shoulder.y - face.y' in native_midna
assert 'appendChild' not in native_midna and 'setAlpha' not in native_midna
native_prompt = source.split('void style_native_midna_backing(', 1)[1].split('void align_native_midna_hud(', 1)[0]
assert 's_blankShoulderResources[style]' in native_prompt
assert 'resize_pane_around_center(button, 64.0f, 64.0f)' in native_prompt
assert 'hudParent->insertChild(parent, cap)' in native_prompt
assert 'parent->insertChild(' not in native_prompt
assert 'cap->setInfluencedAlpha(true, true)' in native_prompt
assert 'button->setAlpha(0)' in native_prompt
assert 'constexpr float portraitScale = 0.70f;' in native_midna
assert 'cap->move(0.0f, 0.0f)' in native_midna
assert 'cap->resize(button->getWidth(), button->getHeight())' in native_midna
assert 'cap->scale(midnaPane->getScaleX()' not in native_midna
assert 'uses_xbox_prompts(button_layout())' in native_prompt
assert 'hd_mdir' not in source
layout = source.split('void apply_wii_u_archive_layout_corrections(', 1)[1].split('void stabilize_wii_u_rupee_counter(', 1)[0]
assert 'constexpr f32 shoulderScale = 0.45f;' in layout
assert layout.count('31.0f, -4.0f, shoulderScale') == 2
assert 'if (feature_enabled(Feature::ThirdItemSlot))' not in layout
native_layout = midna.split('if (!feature_enabled(Feature::DpadShortcuts) &&', 1)[1].split('}', 1)[0]
assert 'nativeButton != kSdlRightShoulderButton' in native_layout
assert 'positionY = -65.0f;' in native_layout
assert 'anchorPane->getScaleY()' in native_layout
badge = source.split('void update_midna_shoulder_badge(', 1)[1].split('bool z_item_menu_or_pause_context()', 1)[0]
assert 'midnaPane->getBounds()' not in badge
assert 'midnaPane->getWidth() * 0.5f + 8.0f' in badge
assert 'midnaPane->getHeight() * 0.5f' in badge
assert '26.0f / effectiveMidnaScale - 30.0f' in badge
print('PASS: independent feature hook ownership, shared HUD guards, input bypasses, and native Midna spacing')
