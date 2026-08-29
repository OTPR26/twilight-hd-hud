"""Keep oil/oxygen centered on their artwork, not the oversized null panes."""
from pathlib import Path
import struct

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
draw = source.split('HookAction before_gauge_screen_draw(', 1)[1].split(
    'void after_meter_gauge_screen(', 1)[0]
for tag in ('mw_ll', 'mw_lu', 'mw_rl', 'mw_ru', 'mm_base'):
    assert f"MULTI_CHAR('{tag}')" in draw
for tag in ('mm_00', 'm_w_l_n', 'm_w_r_n', 'mw_ll_s', 'mw_rl_s'):
    assert f"MULTI_CHAR('{tag}')" not in draw
assert 'if (completeFrame && hasBounds)' in draw
assert draw.index('pane->scale(') < draw.index('add_pane_current_global_bounds(')
setup = source.split('HookAction before_meter_gauge_screen(', 1)[1].split(
    'HookAction before_gauge_screen_draw(', 1)[0]
assert 'type == 1 || type == 2' in setup
restore = source.split('void after_meter_gauge_screen(', 1)[1].split(
    'void hide_other_howl_pictures(', 1)[0]
assert 's_gaugeDraw.pane->scale(s_gaugeDraw.scaleX, s_gaugeDraw.scaleY)' in restore
assert 's_gaugeDraw.pane->translate(s_gaugeDraw.x, s_gaugeDraw.y)' in restore

# Verify the authored geometry that reproduced the 20-pixel error at 1080p.
data = (root / 'overlay/res/Layout/main2D.arc').read_bytes()
assert data[:4] == b'RARC'
count = struct.unpack_from('>I', data, 0x28)[0]
entries = 0x20 + struct.unpack_from('>I', data, 0x2c)[0]
strings = 0x20 + struct.unpack_from('>I', data, 0x34)[0]
files = 0x20 + struct.unpack_from('>I', data, 0x0c)[0]
layout = None
for i in range(count):
    entry = entries + i * 20
    name = strings + struct.unpack_from('>H', data, entry + 6)[0]
    if data[name:data.index(b'\0', name)] == b'zelda_game_image_kantera.blo':
        offset, size = struct.unpack_from('>II', data, entry + 8)
        layout = data[files + offset:files + offset + size]
        break
assert layout is not None and layout[:8] == b'SCRNblo2'
panes = {}
offset = 32
while offset < len(layout):
    kind, size = struct.unpack_from('>4sI', layout, offset)
    assert size >= 8
    if kind in (b'PAN2', b'PIC2'):
        base = offset + (8 if kind == b'PIC2' else 0)
        tag = layout[base + 16:base + 24].lstrip(b'\0').decode('ascii')
        width = struct.unpack_from('>f', layout, base + 32)[0]
        x = struct.unpack_from('>f', layout, base + 60)[0]
        panes[tag] = (width, x)
    offset += size
assert panes['m_w_l_n'] == (115.0, 24.0)
assert panes['m_w_r_n'] == (115.0, 153.0)
for tag in ('mw_ll', 'mw_lu', 'mw_rl', 'mw_ru'):
    assert panes[tag] == (26.0, -25.0), (tag, panes[tag])
print('PASS: oil/oxygen use actual frame artwork, native fill/restore preserved, archive geometry verified')
