"""Convert the HUD's RGBA8 Wolf icons to padded PNGs for touch buttons."""
from pathlib import Path
import struct
import zlib

ROOT = Path(__file__).resolve().parents[1]


def chunk(kind, data):
    return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind + data))


def convert(name):
    data = (ROOT / 'res/hud/wolf_actions' / f'{name}.bti').read_bytes()
    assert data[0] == 6
    width, height = struct.unpack_from('>HH', data, 2)
    offset = struct.unpack_from('>I', data, 28)[0]
    padding = 12
    size = max(width, height) + padding * 2
    pixels = bytearray(size * size * 4)
    for tile_y in range(0, height, 4):
        for tile_x in range(0, width, 4):
            for index in range(16):
                x, y = tile_x + index % 4, tile_y + index // 4
                alpha, red = data[offset + index * 2:offset + index * 2 + 2]
                green, blue = data[offset + 32 + index * 2:offset + 34 + index * 2]
                if x < width and y < height:
                    target = ((y + padding) * size + x + padding) * 4
                    pixels[target:target + 4] = bytes((red, green, blue, alpha))
            offset += 64
    rows = b''.join(b'\0' + pixels[y * size * 4:(y + 1) * size * 4] for y in range(size))
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', size, size, 8, 6, 0, 0, 0))
    png += chunk(b'IDAT', zlib.compress(rows)) + chunk(b'IEND', b'')
    target = ROOT / 'res/ui/wolf' / f'{name}.png'
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_bytes(png)


if __name__ == '__main__':
    for name in ('sense', 'dig-left', 'attack'):
        convert(name)
