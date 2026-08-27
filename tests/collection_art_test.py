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
print('PASS: Collection banner/frame dimensions, tiled RGBA8, transparency, and palette')
