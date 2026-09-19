"""Decode BC7 replacements to portable RGBA8 DDS, preserving every mip.

Requires Pillow. Original compressed source files remain untouched.
"""
import io
import struct
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parents[1]
for source in sorted((root / 'textures').rglob('*.dds')):
    data = source.read_bytes()
    assert data[:4] == b'DDS ' and data[84:88] == b'DX10'
    assert struct.unpack_from('<I', data, 128)[0] == 98
    height, width = struct.unpack_from('<II', data, 12)
    mips = max(1, struct.unpack_from('<I', data, 28)[0])
    header = bytearray(data[:148])
    struct.pack_into('<I', header, 128, 28)
    flags = struct.unpack_from('<I', header, 8)[0]
    struct.pack_into('<I', header, 8, (flags & ~0x80000) | 8)
    struct.pack_into('<I', header, 20, width * 4)
    pixels = bytearray()
    offset = 148
    for level in range(mips):
        w, h = max(1, width >> level), max(1, height >> level)
        size = ((w + 3) // 4) * ((h + 3) // 4) * 16
        single = bytearray(data[:148])
        struct.pack_into('<II', single, 12, h, w)
        struct.pack_into('<I', single, 20, size)
        struct.pack_into('<I', single, 28, 1)
        image = Image.open(io.BytesIO(single + data[offset:offset + size])).convert('RGBA')
        pixels.extend(image.tobytes())
        offset += size
    assert offset == len(data), 'Unexpected DDS payload'
    output = root / 'textures-universal' / source.relative_to(root / 'textures')
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(header + pixels)
    assert Image.open(output).convert('RGBA').tobytes() == Image.open(source).convert('RGBA').tobytes()
    print(f'{output.relative_to(root)}: {width}x{height}, {mips} mip levels, pixel-identical')
