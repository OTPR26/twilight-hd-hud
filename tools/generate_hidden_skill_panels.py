from pathlib import Path
import struct

from PIL import Image


ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIR = ROOT / "assets" / "source" / "hidden-skills"
OUTPUT_DIR = ROOT / "res" / "menu" / "hidden-skills"
NAMES = (
    "ending-blow",
    "shield-attack",
    "back-slice",
    "helm-splitter",
    "mortal-draw",
    "jump-strike",
    "great-spin",
)


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
            alpha_red = bytearray()
            green_blue = bytearray()
            for index in range(16):
                x = min(tile_x + index % 4, width - 1)
                y = min(tile_y + index // 4, height - 1)
                red, green, blue, alpha = pixels[x, y]
                alpha_red.extend((alpha, red))
                green_blue.extend((green, blue))
            payload.extend(alpha_red)
            payload.extend(green_blue)
    return bytes(header + payload)


OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
for name in NAMES:
    source = Image.open(SOURCE_DIR / f"{name}.png").convert("RGBA")
    panel = source.resize((328, 200), Image.Resampling.LANCZOS)
    (OUTPUT_DIR / f"{name}.bti").write_bytes(encode_rgba8_bti(panel))
