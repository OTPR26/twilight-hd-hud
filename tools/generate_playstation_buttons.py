from pathlib import Path
import struct

from PIL import Image, ImageDraw, ImageFont


SIZE = 64
SCALE = 4
ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIR = ROOT / "assets" / "source" / "playstation"
OUTPUT_DIR = ROOT / "res" / "hud"
FONT_PATH = "/System/Library/Fonts/HelveticaNeue.ttc"


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
    data = bytearray()
    for tile_y in range(0, height, 4):
        for tile_x in range(0, width, 4):
            tile = []
            for y in range(4):
                for x in range(4):
                    px = min(tile_x + x, width - 1)
                    py = min(tile_y + y, height - 1)
                    tile.append(pixels[px, py])
            for red, _green, _blue, alpha in tile:
                data.extend((alpha, red))
            for _red, green, blue, _alpha in tile:
                data.extend((green, blue))
    return bytes(header + data)


def draw_symbol(draw: ImageDraw.ImageDraw, symbol: str, color, shadow) -> None:
    cx = cy = SIZE * SCALE // 2
    width = 3 * SCALE

    def line(points, fill, line_width=width):
        draw.line(points, fill=fill, width=line_width, joint="curve")

    def symbol_paths(offset_x=0, offset_y=0):
        ox = offset_x * SCALE
        oy = offset_y * SCALE
        if symbol == "cross":
            return [
                [(cx - 9 * SCALE + ox, cy - 9 * SCALE + oy),
                 (cx + 9 * SCALE + ox, cy + 9 * SCALE + oy)],
                [(cx + 9 * SCALE + ox, cy - 9 * SCALE + oy),
                 (cx - 9 * SCALE + ox, cy + 9 * SCALE + oy)],
            ]
        if symbol == "triangle":
            return [[
                (cx + ox, cy - 11 * SCALE + oy),
                (cx - 10 * SCALE + ox, cy + 8 * SCALE + oy),
                (cx + 10 * SCALE + ox, cy + 8 * SCALE + oy),
                (cx + ox, cy - 11 * SCALE + oy),
            ]]
        if symbol == "square":
            return [[
                (cx - 9 * SCALE + ox, cy - 9 * SCALE + oy),
                (cx + 9 * SCALE + ox, cy - 9 * SCALE + oy),
                (cx + 9 * SCALE + ox, cy + 9 * SCALE + oy),
                (cx - 9 * SCALE + ox, cy + 9 * SCALE + oy),
                (cx - 9 * SCALE + ox, cy - 9 * SCALE + oy),
            ]]
        return []

    if symbol == "circle":
        bounds = (
            cx - 10 * SCALE,
            cy - 10 * SCALE,
            cx + 10 * SCALE,
            cy + 10 * SCALE,
        )
        shadow_bounds = tuple(value + SCALE for value in bounds)
        draw.ellipse(shadow_bounds, outline=shadow, width=width)
        draw.ellipse(bounds, outline=color, width=width)
        return

    for path in symbol_paths(1, 1):
        line(path, shadow)
    for path in symbol_paths():
        line(path, color)


def make_face_button(symbol: str, black: bool) -> Image.Image:
    canvas = Image.new("RGBA", (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    cx = cy = SIZE * SCALE // 2

    if black:
        rings = [
            (29, (4, 5, 6, 215)),
            (27, (105, 109, 114, 255)),
            (25, (18, 20, 23, 255)),
            (23, (55, 58, 62, 255)),
            (21, (25, 27, 30, 255)),
        ]
        symbol_color = (242, 242, 239, 255)
        shadow = (0, 0, 0, 210)
    else:
        rings = [
            (29, (50, 53, 57, 190)),
            (27, (246, 247, 247, 255)),
            (25, (148, 152, 156, 255)),
            (23, (229, 231, 232, 255)),
            (21, (205, 208, 210, 255)),
        ]
        symbol_color = (70, 73, 77, 255)
        shadow = (255, 255, 255, 200)

    for radius, color in rings:
        r = radius * SCALE
        draw.ellipse((cx - r, cy - r, cx + r, cy + r), fill=color)
    draw.arc(
        (cx - 19 * SCALE, cy - 18 * SCALE, cx + 19 * SCALE, cy + 20 * SCALE),
        204,
        332,
        fill=(145, 149, 154, 150) if black else (255, 255, 255, 180),
        width=SCALE,
    )
    draw_symbol(draw, symbol, symbol_color, shadow)
    return canvas.resize((SIZE, SIZE), Image.Resampling.LANCZOS)


def make_shoulder_button(label: str, black: bool) -> Image.Image:
    canvas = Image.new("RGBA", (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    bounds = (5 * SCALE, 17 * SCALE, 59 * SCALE, 47 * SCALE)
    outer = (5, 6, 7, 220) if black else (120, 123, 126, 235)
    rim = (93, 97, 102, 255) if black else (238, 239, 239, 255)
    cap = (24, 26, 29, 255) if black else (205, 207, 208, 255)
    highlight = (137, 141, 146, 160) if black else (255, 255, 255, 190)
    glyph = (242, 242, 239, 255) if black else (92, 94, 96, 255)
    glyph_stroke = (73, 75, 78, 255) if black else (245, 245, 245, 255)
    draw.rounded_rectangle(bounds, radius=12 * SCALE, fill=outer)
    inset = tuple(
        value + (2 * SCALE if index < 2 else -2 * SCALE)
        for index, value in enumerate(bounds)
    )
    draw.rounded_rectangle(inset, radius=10 * SCALE, fill=rim)
    inner = tuple(
        value + (2 * SCALE if index < 2 else -2 * SCALE)
        for index, value in enumerate(inset)
    )
    draw.rounded_rectangle(inner, radius=8 * SCALE, fill=cap)
    draw.arc(inner, 190, 350, fill=highlight, width=SCALE)
    font = ImageFont.truetype(FONT_PATH, 17 * SCALE, index=1)
    box = draw.textbbox((0, 0), label, font=font)
    x = SIZE * SCALE / 2 - (box[2] - box[0]) / 2
    y = SIZE * SCALE / 2 - (box[3] - box[1]) / 2 - box[1]
    draw.text(
        (x, y),
        label,
        font=font,
        fill=glyph,
        stroke_width=SCALE,
        stroke_fill=glyph_stroke,
    )
    return canvas.resize((SIZE, SIZE), Image.Resampling.LANCZOS)


def save_asset(name: str, image: Image.Image) -> None:
    SOURCE_DIR.mkdir(parents=True, exist_ok=True)
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    image.save(SOURCE_DIR / f"{name}.png")
    (OUTPUT_DIR / f"{name}.bti").write_bytes(encode_rgba8_bti(image))


for black, suffix in ((False, ""), (True, "-black-pro")):
    for symbol in ("cross", "circle", "square", "triangle"):
        save_asset(
            f"face-button-ps-{symbol}{suffix}",
            make_face_button(symbol, black),
        )
    for label in ("L2", "R1", "R2"):
        save_asset(
            f"shoulder-button-ps-{label.lower()}{suffix}",
            make_shoulder_button(label, black),
        )
