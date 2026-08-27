"""Validate Collection vector assets using an independent tiled-pixel reader."""
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[1]

def load(name, expected_size):
    data = (ROOT / 'res/menu' / (name + '.bti')).read_bytes()
    width, height = struct.unpack_from('>HH', data, 2)
    assert (width, height) == expected_size
    assert data[:2] == bytes((6, 1))
    assert struct.unpack_from('>I', data, 28)[0] == 32
    assert len(data) == 32 + width * height * 4
    def pixel(x, y):
        tile = (y // 4 * (width // 4) + x // 4) * 64 + 32
        index = (y % 4 * 4 + x % 4) * 2
        return (data[tile+index+1], data[tile+index+32],
                data[tile+index+33], data[tile+index])
    return pixel

banner = load('collection-banner', (940, 152))
assert banner(470, 75) == (30, 34, 13, 255)
assert banner(0, 151)[3] == 0
assert banner(470, 20)[3] == 255
assert sum(banner(470, 20)[:3]) > sum(banner(470, 75)[:3])
frame = load('collection-equipment-frame', (256, 256))
assert frame(128, 128)[3] == 0
assert frame(0, 0)[3] == 0
assert frame(128, 8) == (255, 255, 255, 255)
assert frame(8, 128) == (255, 255, 255, 255)
cell = load('item-bank-cell', (256, 256))
assert cell(0, 0)[3] == 0
assert cell(12, 12)[3] == 0
assert cell(128, 128)[3] == 255
assert cell(128, 32)[3] == 255
assert sum(cell(128, 128)[:3]) > sum(cell(128, 12)[:3])
assert 30 < cell(128, 128)[0] < 70 # Preview 4's darker charcoal center.
assert cell(128, 128)[0] >= cell(128, 128)[2] # Warm charcoal, not blue-gray.
assert len({cell(x, y)[:3] for x in range(100, 120) for y in range(100, 120)}) > 10
circle = load('item-bank-circle', (256, 256))
assert circle(128, 128)[3] == 255
assert circle(32, 32)[3] == 0 # A round slot, not a square with a circle over it.
assert circle(0, 0)[3] == 0
assert sum(circle(128, 11)[:3]) > sum(circle(128, 128)[:3])
assert 20 < circle(128, 128)[0] < 70
shadow = load('item-bank-shadow', (256, 256))
assert shadow(0, 0)[3] == 0
assert shadow(128, 128)[:3] == (0, 0, 0)
assert 0 < shadow(128, 8)[3] < shadow(128, 18)[3] < shadow(128, 30)[3] < 160
assert len({shadow(128, y)[3] for y in range(4, 35)}) > 15
print('PASS: menu art dimensions, tiled RGBA8, transparency, texture, and palette')
