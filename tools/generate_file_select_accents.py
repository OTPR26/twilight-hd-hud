from pathlib import Path
import base64
import io
import math
import struct

from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parent.parent
MENU_DIR = ROOT / "res" / "menu"
SOURCE_DIR = ROOT / "assets" / "source" / "file-select"


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
                    alpha_red[index * 2 + 1],
                    green_blue[index * 2],
                    green_blue[index * 2 + 1],
                    alpha_red[index * 2],
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


def make_row_shadow() -> Image.Image:
    width, height = 256, 28
    alpha = Image.new("L", (width, height), 0)
    pixels = alpha.load()
    for y in range(height):
        vertical = max(0.0, 1.0 - y / 24.0)
        for x in range(width):
            edge = min(1.0, x / 15.0, (width - 1 - x) / 15.0)
            pixels[x, y] = round(108 * vertical * max(0.0, edge))
    alpha = alpha.filter(ImageFilter.GaussianBlur(radius=3.2))
    shadow = Image.new("RGBA", (width, height), (8, 5, 1, 0))
    shadow.putalpha(alpha)
    return shadow


def make_midna_choice_row(selected: bool) -> Image.Image:
    """Make the clean TPHD three-choice panel (no quest-log rules)."""
    width, height = 256, 64
    image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    polygon = [
        (9, 1), (width - 10, 1), (width - 2, 9),
        (width - 2, height - 10), (width - 10, height - 2),
        (9, height - 2), (1, height - 10), (1, 9),
    ]
    mask = Image.new("L", (width, height), 0)
    ImageDraw.Draw(mask).polygon(polygon, fill=255)

    if selected:
        # TPHD's active choice is gently lit from above, then falls through a
        # neutral gray into a soft charcoal shadow at the lower edge.
        gradient = Image.new("RGBA", (width, height))
        pixels = gradient.load()
        for y in range(height):
            t = y / (height - 1)
            if t < 0.68:
                blend = t / 0.68
                top = (137, 135, 128)
                middle = (103, 101, 95)
                rgb = tuple(round(a + (b - a) * blend)
                            for a, b in zip(top, middle))
            else:
                blend = (t - 0.68) / 0.32
                middle = (103, 101, 95)
                bottom = (54, 52, 48)
                rgb = tuple(round(a + (b - a) * blend)
                            for a, b in zip(middle, bottom))
            for x in range(width):
                pixels[x, y] = (*rgb, 255)
        gradient.putalpha(mask)
        image = Image.alpha_composite(image, gradient)
    else:
        fill = Image.new("RGBA", (width, height), (13, 13, 10, 190))
        fill.putalpha(mask.point(lambda value: round(value * 0.75)))
        image = Image.alpha_composite(image, fill)

    draw = ImageDraw.Draw(image)
    # The panel is rendered at roughly 60% of this texture's native width.
    # A three-pixel source stroke therefore lands at the slightly heavier
    # two-pixel visual weight used by TPHD instead of collapsing to a hairline.
    draw.line(polygon + [polygon[0]], fill=(176, 171, 94, 255), width=3,
              joint="curve")
    return image


def add_number_shadow(source: Image.Image) -> Image.Image:
    source = source.convert("RGBA")
    alpha = source.getchannel("A").filter(ImageFilter.GaussianBlur(radius=1.1))
    shifted = Image.new("L", source.size, 0)
    shifted.paste(alpha, (2, 2))
    shadow = Image.new("RGBA", source.size, (18, 9, 0, 0))
    shadow.putalpha(shifted.point(lambda value: round(value * 0.72)))
    return Image.alpha_composite(shadow, source)


def make_prompt_flourish() -> Image.Image:
    """Return a cleaned trace of TPHD's compact A/B prompt ornament.

    The source silhouette was traced from the Cemu reference, stripped to its
    gold alpha, and non-uniformly fitted to this mod's established A/B centers.
    This preserves TPHD's asymmetric interlocking curls and the intentional
    gaps hidden beneath the two button discs without procedural curve drift.
    """
    traced_png = (
        "iVBORw0KGgoAAAANSUhEUgAAAGAAAABYCAYAAAAKsfL4AAAEoklEQVR42u1cq3Lj"
        "ShA9m2uyUIaBEgwUdZWJ6IX+Be0nyEhAaPUL+gXRhSIBhqJhEr1w6MIL0rPbmZJs"
        "2c6uZ+xzqlLlWBpVMt19+jkCCIIgCIIgCIIgCIIgCIIgCIIgCIIgCIL4I/gntD+4LJJ4"
        "u1mn283avB7MzxP3RlP3yDNwav3fwFNgmx8ByACgqgezYEl04TUKYAYpAFPVQ7fk"
        "5qoexrJIYgrg87Q/BdCfudSc+T0FMIPYavU5i6p6MCI8V/spgAvop79wbWSFIJQU"
        "LfQhfxyrgOgnAjBe8ZisLBL7ufflf1sFov3Rlbxt1NrxXBp7eJRFkpZFUnyCFXmHp"
        "4As4CrO9oXzQxYAaAEEBUABBExDFMDlMLSAwMEo6HoLiHyNZB5BAKP4gOieQtBgB"
        "CD0MeK9IEcLwA1KEXgvR6f3xP9BCEBqQN+tAI50uIKjH8DzpnxZJDsAOYADgB/iA"
        "55fD+aw9Bk+NN5DtoAdgLaqh31VDy2ABkBcFkl2D9rvtQAU1bSKy3sAHYDdKSqyn"
        "S8KYKGmzvRtp7LgTiKiQpzzrPBCaLx88SjCiVTM38t3OYC9G8WIsKxjbp1WZSRd"
        "rx7gZNwSmnmRDXzD7+mHZ/n8AmB8PZj/XMe63ax/ynUD4Ktq2rxV9fAWSh6wunGC"
        "NTraO4pgMhFErKchLE2JRfSq19v6HOsHF4a+Hsy43ax/WcN2s37ZbtbPAGJLLWIF"
        "XwH8C+CH7+FmcFGQbLTm92hinKQTC0hPOXhfQ1Lfx1KsM56kGJl6G/E+sNtNbH4m"
        "wrFDWT2Azie6egqgCNefqAH1kpxFjtbnEkXpfCGT8DWmAHBeKfoIhYwTLctcNruX"
        "7LmWn1buy32hpFUIpWgZKZybDTIqfB1F8zMAjZQvPvgMoaxCyhwNLQDLO2JHaMp2"
        "zFLR/tZuflkkH+hJQt/2ksrqI8+GGtHw/sT1nTjZRoSxE/8xlkXSKYvoxUrSKwd+"
        "ORWhrtsKaS2a/d0Zac+FnqzVdD502O5FAJFYgA0xC1mzVw64kxH1VDnv+NY0FAoF"
        "LTlwZwC00sSJpYhntd+URdKIVWROGSO+JQ3dy1xQpJK1DEDvVkM17ZRFEmnnTQrC"
        "1RXVSELMX1Q0c3unz5v5MPIYkgDMicN7vdNTODYdwUwYn3t4r5s5jnRWTkEBnHlC"
        "Rp0dNop+zpmgi2+dB6wCj4BspVNXPWeb8Soj7pXvMLSACyxANnM3w+fxiefoMUdm"
        "wgsF4G5UpmL6vfw0NsE68hybrKUSrtICFoSZRm+UfLcTbW6qehirerAv8Wgl450S"
        "wljVQyeNmuxIuEoBzDlKoZ5cja+4ltGKYPKZ0nYspYpx6VtXHlYA6hUFRv1eHKuM"
        "iqU0YgW50ylLpRwRSX0ILEcfRyaaajXXthhr0fzMjXhkw21DPxdBjE7UVPsyuOXz"
        "bGgm3N/LZ6u5+6oetPNMnc2PlRXUqlRtHfa3iU4ZHnI00Rm0cl9LFqtw0VJO49yb"
        "2Q6YarJ84HZFY3CdOQXwO5OdSq506aCdG7SVAxyxiumbkKbkvnjiaOeyUrNkwlm9"
        "hKkHQRAEQRAEQRAEQRAEQXiK/wGJrg3m9POxawAAAABJRU5ErkJggg=="
    )
    traced_png = "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"
    return Image.open(io.BytesIO(base64.b64decode(traced_png))).convert("RGBA")


def extend_selected_fill(source: Image.Image) -> Image.Image:
    source = source.convert("RGBA")
    width, height = source.size
    result = Image.new("RGBA", source.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(result)
    # Match the clipped octagonal frame on every corner.  The previous mask
    # clipped only the lower pair, leaving a square grey fill behind the two
    # angled upper corners when the row was highlighted.
    draw.polygon(
        [(10, 2), (width - 11, 2), (width - 3, 10),
         (width - 3, height - 11), (width - 11, height - 3),
         (10, height - 3), (2, height - 11), (2, 10)],
        # Opaque charcoal-grey prevents the full-screen ornamental rings from
        # showing through the selected card.  Keeping the value darker than
        # the prior translucent fill avoids the washed-out selected state.
        fill=(92, 90, 84, 255),
    )
    # Rebuild the interior instead of layering over the archive's translucent
    # fill. This blocks the large background spirals at the clipped corners
    # while keeping the selected grey substantially darker than the previous
    # washed-out opaque pass. Restore only the gold outline and fine rules.
    source_pixels = source.load()
    result_pixels = result.load()
    for y in range(height):
        for x in range(width):
            red, green, blue, alpha = source_pixels[x, y]
            # Never restore source pixels beyond the new mask: some of the
            # archive border texture's translucent corner field is rectangular
            # and was the remaining source of corner bleed.
            if result_pixels[x, y][3] > 0 and (
                    alpha >= 150 or
                    (alpha > 0 and red >= 225 and green >= 225)):
                result_pixels[x, y] = (red, green, blue, alpha)
    return result


(MENU_DIR / "file-select-row-shadow.bti").write_bytes(
    encode_rgba8_bti(make_row_shadow())
)
(MENU_DIR / "midna-choice-row.bti").write_bytes(
    encode_rgba8_bti(make_midna_choice_row(False))
)
(MENU_DIR / "midna-choice-row-selected.bti").write_bytes(
    encode_rgba8_bti(make_midna_choice_row(True))
)
(MENU_DIR / "file-select-prompt-flourish.bti").write_bytes(
    encode_rgba8_bti(make_prompt_flourish())
)

SOURCE_DIR.mkdir(parents=True, exist_ok=True)
selected_source = SOURCE_DIR / "file-select-row-selected.png"
if not selected_source.exists():
    decode_rgba8_bti(MENU_DIR / "file-select-row-selected.bti").save(
        selected_source
    )
(MENU_DIR / "file-select-row-selected.bti").write_bytes(
    encode_rgba8_bti(extend_selected_fill(Image.open(selected_source)))
)

for number in range(1, 4):
    source_path = SOURCE_DIR / f"file-select-number-{number}.png"
    output_path = MENU_DIR / f"file-select-number-{number}.bti"
    if not source_path.exists():
        decode_rgba8_bti(output_path).save(source_path)
    output_path.write_bytes(
        encode_rgba8_bti(add_number_shadow(Image.open(source_path)))
    )
