#!/usr/bin/env python3
"""Build redistributable, single-page I4 BFN atlases from the pinned OFL fonts.

Requires Pillow with FreeType support. No game files are read or embedded.
The game still owns text metrics; these resources provide only glyph drawings.
"""

import argparse
import hashlib
import json
from pathlib import Path
import struct

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[1]
CELL, ASCENT, DESCENT, PAD = 128, 104, 24, 4
# Fira needs extra room below its baseline. Each face is scaled to the same
# visible capital height by the draw hook; padded raster sizes can differ.
CAP_HEIGHTS = {"zen": 65, "mplus": 65, "fira": 57}
SIDE = CELL * 16
SOURCES = {
    "zen": ("ZenKakuGothicNew-Bold.ttf",
            "0081cedabc4921982fcd061f845a005664ac7fb642af2dd34b4007bc63ccd235"),
    "mplus": ("MPLUS2-Variable.ttf",
              "2e4f45c2391355fb03195da4854ffbe85fea49bfdff5cc51020238083af6b75c"),
    "fira": ("FiraSans-Bold.ttf",
             "a4d8e149ecdd4874a0726eb0af894488b3b31c423d6b0017c8f415ed1b795b45"),
}


def supported(code):
    return 0x20 <= code <= 0x7E or 0xA0 <= code <= 0xFF


def make_font(path, size, variable):
    font = ImageFont.truetype(str(path), size, layout_engine=ImageFont.Layout.BASIC)
    if variable:
        font.set_variation_by_axes([700])
    return font


def fit_font(path, variable, cap_height):
    for size in range(160, 40, -1):
        font = make_font(path, size, variable)
        box = font.getbbox("H", anchor="ls")
        if -box[1] != cap_height:
            continue
        bounds = [font.getbbox(bytes([c]).decode("cp1252"), anchor="ls")
                  for c in range(256) if supported(c)]
        if all(max(PAD, 2 - b[0]) + b[2] < CELL - 2 and
               ASCENT + b[1] >= 2 and ASCENT + b[3] < CELL - 2 for b in bounds):
            return font, size
    raise ValueError(f"Cannot fit {path.name} in a padded glyph cell")


def encode_i4(im):
    pixels = im.load()
    out = bytearray()
    for ty in range(0, im.height, 8):
        for tx in range(0, im.width, 8):
            for y in range(ty, ty + 8):
                for x in range(tx, tx + 8, 2):
                    out.append(((pixels[x, y] + 8) // 17 << 4) |
                               ((pixels[x + 1, y] + 8) // 17))
    return bytes(out)


def decode_i4(data, side):
    im = Image.new("L", (side, side))
    pixels = im.load()
    cursor = 0
    for ty in range(0, side, 8):
        for tx in range(0, side, 8):
            for y in range(ty, ty + 8):
                for x in range(tx, tx + 8, 2):
                    val = data[cursor]
                    cursor += 1
                    pixels[x, y], pixels[x + 1, y] = (val >> 4) * 17, (val & 15) * 17
    return im


def block(magic, payload, minimum=0):
    length = max(minimum, (8 + len(payload) + 31) // 32 * 32)
    return magic + struct.pack(">I", length) + payload + bytes(length - 8 - len(payload))


def validate_bfn(data):
    assert data[:8] == b"FONTbfn1"
    assert struct.unpack_from(">II", data, 8) == (len(data), 4)
    offset, blocks = 32, {}
    while offset < len(data):
        magic, size = struct.unpack_from(">4sI", data, offset)
        assert size >= 32 and size % 32 == 0 and offset + size <= len(data)
        assert magic not in blocks
        blocks[magic] = (offset, data[offset:offset + size])
        offset += size
    assert offset == len(data)
    assert {k: v[0] for k, v in blocks.items()} == {
        b"INF1": 32, b"WID1": 64, b"MAP1": 608, b"GLY1": 640}
    assert struct.unpack_from(">6H", blocks[b"INF1"][1], 8) == (
        0, ASCENT, DESCENT, CELL, CELL, ord("?"))
    assert struct.unpack_from(">4H", blocks[b"MAP1"][1], 8) == (0, 0, 255, 0)
    assert struct.unpack_from(">4HI6H", blocks[b"GLY1"][1], 8) == (
        0, 255, CELL, CELL, SIDE * SIDE // 2, 0, 16, 16, SIDE, SIDE, 0)
    assert len(data) == 672 + SIDE * SIDE // 2
    return blocks


def generate(name, out_dir, preview_dir):
    filename, expected_hash = SOURCES[name]
    path = ROOT / "tools/font_sources" / filename
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest()
    if actual_hash != expected_hash:
        raise ValueError(f"Source checksum mismatch: {filename}")
    font, font_size = fit_font(path, name == "mplus", CAP_HEIGHTS[name])
    atlas = Image.new("L", (SIDE, SIDE))
    widths = bytearray()
    missing_reference = bytes(font.getmask("\u0378"))
    for code in range(256):
        if not supported(code):
            widths.extend((PAD, 0))
            continue
        char = bytes([code]).decode("cp1252")
        pad = max(PAD, 2 - font.getbbox(char, anchor="ls")[0])
        mask = font.getmask(char)
        if char not in (" ", "\u00a0", "\u00ad") and bytes(mask) == missing_reference:
            raise ValueError(f"Missing glyph {char!r} in {filename}")
        tile = Image.new("L", (CELL, CELL))
        ImageDraw.Draw(tile).text((pad, ASCENT), char, font=font, fill=255, anchor="ls")
        bounds = tile.getbbox()
        if bounds and (bounds[0] < 2 or bounds[1] < 2 or
                       bounds[2] >= CELL - 1 or bounds[3] >= CELL - 1):
            raise ValueError(f"Glyph clipping: {char!r}, {bounds}")
        atlas.paste(tile, ((code % 16) * CELL, (code // 16) * CELL))
        width = round(font.getlength(char))
        assert 0 <= width < 256
        widths.extend((pad, width))
    image_bytes = encode_i4(atlas)
    payload = b"".join([
        block(b"INF1", struct.pack(">6H", 0, ASCENT, DESCENT, CELL, CELL, ord("?"))),
        block(b"WID1", struct.pack(">HH", 0, 255) + widths),
        block(b"MAP1", struct.pack(">4H", 0, 0, 255, 0)),
        block(b"GLY1", struct.pack(">4HI6H", 0, 255, CELL, CELL, len(image_bytes),
                                    0, 16, 16, SIDE, SIDE, 0) + image_bytes),
    ])
    bfn = b"FONTbfn1" + struct.pack(">II", len(payload) + 32, 4) + bytes(16) + payload
    blocks = validate_bfn(bfn)
    decoded = decode_i4(blocks[b"GLY1"][1][32:], SIDE)
    quantized = atlas.point(lambda x: ((x + 8) // 17) * 17)
    assert decoded.tobytes() == quantized.tobytes(), "I4 tile round-trip mismatch"
    destination = out_dir / f"{name}-bold.bfn"
    destination.write_bytes(bfn)
    if preview_dir:
        decoded.save(preview_dir / f"{name}-atlas.png")
        preview = Image.new("RGB", (900, 230), "#1c2726")
        draw = ImageDraw.Draw(preview)
        display = make_font(path, 38, name == "mplus")
        draw.text((25, 15), "Collection     Copy     Start     Erase", font=display,
                  fill="#e3cd83", stroke_width=1, stroke_fill="black")
        draw.text((25, 85), "Items   Map   Put away   1000", font=display,
                  fill="#f3f0e4", stroke_width=1, stroke_fill="black")
        draw.text((25, 155), "Épée · Forêt · Corazón · agpqy Il1", font=display,
                  fill="#f3f0e4", stroke_width=1, stroke_fill="black")
        preview.save(preview_dir / f"{name}-specimen.png")
    return dict(source_file=filename, source_sha256=actual_hash, weight=700,
                raster_size=font_size, cap_height=-font.getbbox("H", anchor="ls")[1],
                glyph_count=sum(supported(c) for c in range(256)),
                atlas_file=destination.name, atlas_sha256=hashlib.sha256(bfn).hexdigest())


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "res/fonts")
    parser.add_argument("--preview", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    if args.preview:
        args.preview.mkdir(parents=True, exist_ok=True)
    report = {name: generate(name, args.output, args.preview) for name in SOURCES}
    (args.output / "font-atlases.json").write_text(json.dumps(report, indent=2) + "\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
