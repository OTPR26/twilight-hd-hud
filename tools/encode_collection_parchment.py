"""Convert the Collection parchment source to the game's tiled RGBA8 format."""

from pathlib import Path
import struct

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets/source/collection-parchment.png"
OUTPUT = ROOT / "res/menu/collection-parchment.bti"


def encode_rgba8(image):
    width, height = image.size
    assert width % 4 == 0 and height % 4 == 0
    header = bytearray(32)
    header[0:2] = bytes((6, 1))
    struct.pack_into(">HH", header, 2, width, height)
    header[20:22] = bytes((1, 1))
    header[24] = 1
    struct.pack_into(">I", header, 28, 32)
    data = bytearray(header)
    pixels = image.load()
    for ty in range(0, height, 4):
        for tx in range(0, width, 4):
            tile = [pixels[tx + x, ty + y] for y in range(4) for x in range(4)]
            for red, green, blue, alpha in tile:
                data.extend((alpha, red))
            for red, green, blue, alpha in tile:
                data.extend((green, blue))
    return data


if __name__ == "__main__":
    # Fit the GPU's 1024-pixel texture limit and RGBA8's 4x4 tiles. Preserve
    # alpha at the worn edges; the game supplies all text and ornaments.
    with Image.open(SOURCE) as source:
        image = source.convert("RGBA").resize((1024, 428), Image.Resampling.LANCZOS)
    OUTPUT.write_bytes(encode_rgba8(image))
    print(f"Encoded {OUTPUT.name}: 1024x428 RGBA8, {OUTPUT.stat().st_size} bytes")
