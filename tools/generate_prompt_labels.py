from pathlib import Path
import struct

from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parent.parent
FONT = ROOT / "tools/font_sources/FiraSans-Bold.ttf"
OUTPUT = ROOT / "res/menu"
PREVIEW_OUTPUT = ROOT / "assets/source/prompt-labels"


def decode_rgba8_bti(path: Path) -> Image.Image:
    data = path.read_bytes()
    width, height = struct.unpack_from(">HH", data, 2)
    image = Image.new("RGBA", (width, height))
    pixels = image.load()
    offset = 32
    for tile_y in range(0, height, 4):
        for tile_x in range(0, width, 4):
            alpha_red = data[offset:offset + 32]
            green_blue = data[offset + 32:offset + 64]
            offset += 64
            for index in range(16):
                x = min(tile_x + index % 4, width - 1)
                y = min(tile_y + index // 4, height - 1)
                pixels[x, y] = (
                    alpha_red[index * 2 + 1], green_blue[index * 2],
                    green_blue[index * 2 + 1], alpha_red[index * 2],
                )
    return image


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


def prompt_label(text: str, width: int) -> Image.Image:
    scale = 4
    height = 48
    size = (width * scale, height * scale)
    # TPHD's prompt labels are roughly two-thirds the height of the adjacent
    # button disc. Keep both words on the exact same 28 px treatment.
    font = ImageFont.truetype(FONT, 28 * scale)
    probe = ImageDraw.Draw(Image.new("L", size))
    bounds = probe.textbbox((0, 0), text, font=font, stroke_width=0)
    text_width = bounds[2] - bounds[0]
    text_height = bounds[3] - bounds[1]
    # The BTI canvases are power-of-two and are drawn immediately left of the
    # button discs. Bias the visible lettering right within that transparent
    # canvas so the word, rather than its texture bounds, sits beside A/B.
    # The longer Confirm canvas needs less bias than Back; using one offset for
    # both caused Confirm's final letter to enter the A-button disc.
    right_bias = 2 if text == "Confirm" else 12
    x = (size[0] - text_width) // 2 - bounds[0] + right_bias * scale
    y = (size[1] - text_height) // 2 - bounds[1]

    fill_mask = Image.new("L", size, 0)
    stroke_mask = Image.new("L", size, 0)
    ImageDraw.Draw(fill_mask).text((x, y), text, font=font, fill=255)
    ImageDraw.Draw(stroke_mask).text(
        (x, y), text, font=font, fill=255, stroke_width=2 * scale,
        stroke_fill=255)

    result = Image.new("RGBA", size, (0, 0, 0, 0))
    result.paste((0, 0, 0, 205), mask=stroke_mask)
    result.paste((235, 235, 230, 255), mask=fill_mask)
    return result.resize((width, height), Image.Resampling.LANCZOS)


OUTPUT.mkdir(parents=True, exist_ok=True)
PREVIEW_OUTPUT.mkdir(parents=True, exist_ok=True)
decode_rgba8_bti(OUTPUT / "file-select-back-label.bti").save(
    PREVIEW_OUTPUT / "existing-back-reference.png")
for name, text, width in (
    ("prompt-label-back.bti", "Back", 128),
    ("prompt-label-confirm.bti", "Confirm", 192),
):
    # Generate both words in one pass so their face, weight, outline, baseline,
    # and antialiasing are identical rather than merely similar.
    label = prompt_label(text, width)
    (OUTPUT / name).write_bytes(encode_rgba8_bti(label))
    label.save(PREVIEW_OUTPUT / name.replace(".bti", ".png"))
    print(OUTPUT / name)
