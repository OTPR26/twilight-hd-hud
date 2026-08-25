from pathlib import Path
import struct

from PIL import Image, ImageDraw, ImageFont


SIZE = 64
SCALE = 4
ROOT = Path(__file__).resolve().parent.parent
SOURCE_DIR = ROOT / "assets" / "source" / "playstation"
OUTPUT_DIR = ROOT / "res" / "hud"
FONT_PATH = "/System/Library/Fonts/HelveticaNeue.ttc"
MAIN_2D_ARCHIVE = ROOT / "overlay" / "res" / "Layout" / "main2D.arc"


def extract_rarc_file(archive_path: Path, filename: str) -> bytes:
    data = archive_path.read_bytes()
    if data[:4] != b"RARC":
        raise ValueError(f"{archive_path} is not a RARC archive")

    entry_count = struct.unpack_from(">I", data, 0x28)[0]
    entry_table = 0x20 + struct.unpack_from(">I", data, 0x2C)[0]
    string_table = 0x20 + struct.unpack_from(">I", data, 0x34)[0]
    file_data = 0x20 + struct.unpack_from(">I", data, 0x0C)[0]
    for index in range(entry_count):
        entry = entry_table + index * 20
        name_offset = struct.unpack_from(">H", data, entry + 6)[0]
        name_start = string_table + name_offset
        name_end = data.index(b"\0", name_start)
        if data[name_start:name_end] != filename.encode("ascii"):
            continue
        offset, size = struct.unpack_from(">II", data, entry + 8)
        return data[file_data + offset:file_data + offset + size]
    raise FileNotFoundError(filename)


def decode_ia4_bti(data: bytes) -> Image.Image:
    if data[0] != 2:
        raise ValueError(f"Expected IA4 BTI, found format {data[0]}")
    width, height = struct.unpack_from(">HH", data, 2)
    image_offset = struct.unpack_from(">I", data, 28)[0]
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    pixels = image.load()
    cursor = image_offset
    for block_y in range(0, height, 4):
        for block_x in range(0, width, 8):
            for y in range(4):
                for x in range(8):
                    value = data[cursor]
                    cursor += 1
                    px = block_x + x
                    py = block_y + y
                    if px < width and py < height:
                        intensity = (value & 0x0F) * 17
                        alpha = (value >> 4) * 17
                        pixels[px, py] = (intensity, intensity, intensity, alpha)
    return image


def make_wii_u_l_from_archive_r() -> Image.Image:
    """Mirror TPHD's exact R cap and replace only its embedded glyph."""
    r_button = decode_ia4_bti(extract_rarc_file(MAIN_2D_ARCHIVE, "wiiu_r.bti"))
    image = r_button.transpose(Image.Transpose.FLIP_LEFT_RIGHT)
    pixels = image.load()
    width, height = image.size

    # Remove the mirrored R from the cap by interpolating the surrounding cap
    # shading on each scanline. This retains the exact outline, rim and gloss.
    mask_left = int(width * 0.36)
    mask_right = int(width * 0.64)
    mask_top = int(height * 0.28)
    mask_bottom = int(height * 0.70)
    for y in range(mask_top, mask_bottom):
        left = pixels[mask_left - 2, y]
        right = pixels[mask_right + 2, y]
        span = max(1, mask_right - mask_left)
        for x in range(mask_left, mask_right + 1):
            t = (x - mask_left) / span
            pixels[x, y] = tuple(
                round(left[channel] * (1.0 - t) + right[channel] * t)
                for channel in range(4)
            )

    draw = ImageDraw.Draw(image)
    font = ImageFont.truetype(FONT_PATH, 22, index=1)
    box = draw.textbbox((0, 0), "L", font=font, stroke_width=1)
    x = width / 2 - (box[2] - box[0]) / 2 - box[0]
    y = height / 2 - (box[3] - box[1]) / 2 - box[1]
    draw.text((x, y), "L", font=font, fill=(92, 94, 96, 255),
              stroke_width=1, stroke_fill=(245, 245, 245, 255))
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


def make_wii_u_shoulder_button(label: str, black: bool) -> Image.Image:
    """Draw the flatter, tapered shoulder cap used by the TPHD HUD."""
    canvas = Image.new("RGBA", (SIZE * SCALE, SIZE * SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    outer = (5, 6, 7, 220) if black else (120, 123, 126, 235)
    rim = (93, 97, 102, 255) if black else (238, 239, 239, 255)
    cap = (24, 26, 29, 255) if black else (205, 207, 208, 255)
    highlight = (137, 141, 146, 160) if black else (255, 255, 255, 190)
    glyph = (242, 242, 239, 255) if black else (92, 94, 96, 255)
    glyph_stroke = (73, 75, 78, 255) if black else (245, 245, 245, 255)

    def points(values):
        return [(x * SCALE, y * SCALE) for x, y in values]

    # TPHD's R cap has a square inner edge and a rounded/tapered outer edge.
    # This is its horizontal mirror for L: taper on the left, straight edge on
    # the right.  The shallow lower lip matches the photographed Wii U art.
    outer_shape = points(((16, 20), (57, 20), (60, 23), (60, 42),
                          (57, 45), (9, 45), (4, 40), (8, 26)))
    rim_shape = points(((17, 22), (56, 22), (58, 24), (58, 40),
                        (56, 43), (10, 43), (7, 39), (11, 27)))
    cap_shape = points(((18, 24), (54, 24), (56, 26), (56, 38),
                        (54, 41), (12, 41), (10, 38), (13, 28)))
    draw.polygon(outer_shape, fill=outer)
    draw.line(outer_shape + [outer_shape[0]], fill=outer,
              width=3 * SCALE, joint="curve")
    draw.polygon(rim_shape, fill=rim)
    draw.line(rim_shape + [rim_shape[0]], fill=rim,
              width=2 * SCALE, joint="curve")
    draw.polygon(cap_shape, fill=cap)
    draw.line(cap_shape[:4], fill=highlight, width=SCALE, joint="curve")

    font = ImageFont.truetype(FONT_PATH, 15 * SCALE, index=1)
    box = draw.textbbox((0, 0), label, font=font)
    x = SIZE * SCALE / 2 - (box[2] - box[0]) / 2
    y = 31.5 * SCALE - (box[3] - box[1]) / 2 - box[1]
    draw.text((x, y), label, font=font, fill=glyph,
              stroke_width=SCALE, stroke_fill=glyph_stroke)
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
    for label in ("L1", "L2", "R1", "R2"):
        save_asset(
            f"shoulder-button-ps-{label.lower()}{suffix}",
            make_shoulder_button(label, black),
        )

for label in ("L",):
    save_asset(
        f"shoulder-button-{label.lower()}",
        make_wii_u_l_from_archive_r(),
    )
    save_asset(
        f"shoulder-button-{label.lower()}-black-pro",
        make_wii_u_shoulder_button(label, True),
    )
