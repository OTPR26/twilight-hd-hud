"""HUD sizing keeps currency visibility, key geometry, and map state intact."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/item_slot_hooks.cpp').read_text()
icon = source.split('void scale_rupee_icon_for_draw(', 1)[1].split(
    'void restore_rupee_icon_draw(', 1)[0]
assert "MULTI_CHAR('rupi')" in icon
assert '!icon->isVisible() || icon->getAlpha() == 0' in icon
assert 'rupee_icon_multiplier(scales.overall, scales.rupees)' in icon
assert 'rupee_icon_right(originalBR.x, digits, scales.rupees)' in icon
assert 'mpRupeeKeyParent' not in icon and 'mpKeyParent' not in icon
assert 'setAlpha' not in icon and 'show()' not in icon
restore = source.split('void restore_rupee_icon_draw(', 1)[1].split(
    'void arrange_action_prompt_for_draw(', 1)[0]
assert 'state.icon->scale(state.scaleX, state.scaleY)' in restore
assert 'state.icon->translate(state.x, state.y)' in restore
assert 's_rupeeIconDrawState = {}' in restore
before = source.split('HookAction before_meter_draw(', 1)[1].split(
    'void after_meter_draw(', 1)[0]
assert before.index('stabilize_wii_u_rupee_counter') < before.index('scale_rupee_icon_for_draw')
after = source.split('void after_meter_draw(', 1)[1].split(
    'void after_meter_draw_button_z(', 1)[0]
assert after.index('draw_uniform_rupee_digits') < after.index('restore_rupee_icon_draw')
assert '!menu_overlay_hides_rupees()' in after
digits = source.split('void draw_uniform_rupee_digits(', 1)[1].split('void draw_wolf_icon(', 1)[0]
for helper in ('rupee_digit_size(scale)', 'rupee_digit_step(scale)', 'rupee_digit_gap(scale)'):
    assert helper in digits
assert 'digit->setAlpha(icon->getAlpha())' in digits
minimap = source.split('void apply_wii_u_minimap_layout(', 1)[1].split(
    '// Item wheel and R-slot HUD', 1)[0]
assert 'minimap_multiplier(hud_scales().minimap)' in minimap
assert 'originalHeight - map->mSizeH + 12.0f' in minimap
for name in ('drawPosX', 'drawPosY', 'sizeW', 'sizeH'):
    assert f's_wiiUMinimapTransform.{name}' in minimap
assert 'ADD_POST(MeterMapDrawHook, after_meter_map_draw' in source
print('PASS: rupee icon/digits scale together; visibility/keys and minimap restoration preserved')
