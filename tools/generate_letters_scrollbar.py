#!/usr/bin/env python3
"""Generate the small TPHD-style pieces used by the functional letter scrollbar."""

from pathlib import Path

from PIL import Image, ImageDraw

from encode_rgba8_raw import encode


ROOT = Path(__file__).resolve().parents[1]
OUTPUT = ROOT / "res/menu"
SCALE = 4


def downsample(image: Image.Image, size: tuple[int, int]) -> Image.Image:
    return image.resize(size, Image.Resampling.LANCZOS)


def arrow() -> Image.Image:
    size = (24, 24)
    image = Image.new("RGBA", (size[0] * SCALE, size[1] * SCALE))
    draw = ImageDraw.Draw(image)
    points = [(3 * SCALE, 20 * SCALE), (12 * SCALE, 3 * SCALE),
              (21 * SCALE, 20 * SCALE)]
    draw.polygon(points, fill=(23, 24, 14, 190))
    draw.line(points + [points[0]], fill=(222, 221, 177, 255),
              width=3 * SCALE, joint="curve")
    return downsample(image, size)


def rail(thumb_stop: int | None = None) -> Image.Image:
    size = (16, 256)
    image = Image.new("RGBA", (size[0] * SCALE, size[1] * SCALE))
    draw = ImageDraw.Draw(image)
    draw.rounded_rectangle((2 * SCALE, 1 * SCALE, 14 * SCALE, 255 * SCALE),
                           radius=6 * SCALE, fill=(23, 24, 14, 190),
                           outline=(222, 221, 177, 255), width=2 * SCALE)
    if thumb_stop is not None:
        thumb_height = 144
        travel = 252 - thumb_height
        top = 2 + round(thumb_stop * travel / 10)
        draw.rounded_rectangle((4 * SCALE, top * SCALE,
                                12 * SCALE, (top + thumb_height) * SCALE),
                               radius=4 * SCALE, fill=(235, 233, 195, 255))
    return downsample(image, size)


def write(name: str, image: Image.Image) -> None:
    width, height = image.size
    (OUTPUT / name).write_bytes(encode(image.tobytes(), width, height))


if __name__ == "__main__":
    OUTPUT.mkdir(parents=True, exist_ok=True)
    write("letters-scroll-arrow.bti", arrow())
    for stop in range(11):
        write(f"letters-scroll-state-{stop}.bti", rail(stop))
