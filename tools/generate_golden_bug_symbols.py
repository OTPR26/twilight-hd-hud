from pathlib import Path
import struct

from PIL import Image, ImageDraw


ROOT = Path(__file__).resolve().parent.parent
OUTPUT_DIR = ROOT / "res" / "menu"


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


def symbol(female: bool) -> Image.Image:
    scale = 4
    image = Image.new("RGBA", (32 * scale, 32 * scale), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    color = (114, 75, 24, 255)
    width = 3 * scale
    draw.ellipse((7 * scale, 4 * scale, 23 * scale, 20 * scale), outline=color, width=width)
    if female:
        draw.line((15 * scale, 20 * scale, 15 * scale, 29 * scale), fill=color, width=width)
        draw.line((10 * scale, 25 * scale, 20 * scale, 25 * scale), fill=color, width=width)
    else:
        draw.line((21 * scale, 7 * scale, 29 * scale, 1 * scale), fill=color, width=width)
        draw.line((22 * scale, 1 * scale, 29 * scale, 1 * scale), fill=color, width=width)
        draw.line((29 * scale, 1 * scale, 29 * scale, 8 * scale), fill=color, width=width)
    return image.resize((32, 32), Image.Resampling.LANCZOS)


OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
(OUTPUT_DIR / "golden-bug-male.bti").write_bytes(encode_rgba8_bti(symbol(False)))
(OUTPUT_DIR / "golden-bug-female.bti").write_bytes(encode_rgba8_bti(symbol(True)))
