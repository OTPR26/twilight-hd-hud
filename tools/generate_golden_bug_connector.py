from pathlib import Path
import struct

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets/source/golden-bug-connector-generated.png"
OUTPUT = ROOT / "res/menu/golden-bug-connector.bti"


def encode_rgba8_bti(image: Image.Image) -> bytes:
    image = image.convert("RGBA")
    width, height = image.size
    header = bytearray(32)
    header[0] = 6
    header[1] = 1
    struct.pack_into(">HH", header, 2, width, height)
    header[20] = 1
    header[21] = 1
    header[24] = 1
    struct.pack_into(">I", header, 28, 32)

    pixels = image.load()
    payload = bytearray()
    for tile_y in range(0, height, 4):
        for tile_x in range(0, width, 4):
            tile = []
            for y in range(4):
                for x in range(4):
                    tile.append(pixels[min(tile_x + x, width - 1),
                        min(tile_y + y, height - 1)])
            for red, _green, _blue, alpha in tile:
                payload.extend((alpha, red))
            for _red, green, blue, _alpha in tile:
                payload.extend((green, blue))
    return bytes(header + payload)


source = Image.open(SOURCE).convert("RGBA")
width, height = source.size
# Keep the generated woven knot and only short lengths of its two rails. This
# compact crop preserves the TPHD reference's near-square center motif when it
# is drawn in the narrow space between the two bug frames.
connector = source.crop((
    round(width * 0.379), round(height * 0.138),
    round(width * 0.621), round(height * 0.862),
))
connector = connector.resize((64, 48), Image.Resampling.LANCZOS)
OUTPUT.parent.mkdir(parents=True, exist_ok=True)
OUTPUT.write_bytes(encode_rgba8_bti(connector))
print(OUTPUT)
