#!/usr/bin/env python3
"""Extract the TPHD HUD map icon using the silhouette visible in the reference."""

from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter

from generate_playstation_buttons import encode_rgba8_bti


ROOT = Path(__file__).resolve().parents[1]
REFERENCE = ROOT / "assets/source/hud/tphd-map-outline-reference.png"
OUTPUT = ROOT / "assets/source/hud/tphd-map-icon-traced.png"
BTI_OUTPUT = ROOT / "res/hud/tphd-map-icon.bti"


def catmull_rom(points, samples_per_segment=16):
    """Return a smooth closed contour passing through every traced edge point."""
    points = list(points)
    result = []
    count = len(points)
    for index in range(count):
        p0 = points[(index - 1) % count]
        p1 = points[index]
        p2 = points[(index + 1) % count]
        p3 = points[(index + 2) % count]
        for sample in range(samples_per_segment):
            t = sample / samples_per_segment
            t2 = t * t
            t3 = t2 * t
            x = 0.5 * (
                2 * p1[0]
                + (-p0[0] + p2[0]) * t
                + (2 * p0[0] - 5 * p1[0] + 4 * p2[0] - p3[0]) * t2
                + (-p0[0] + 3 * p1[0] - 3 * p2[0] + p3[0]) * t3
            )
            y = 0.5 * (
                2 * p1[1]
                + (-p0[1] + p2[1]) * t
                + (2 * p0[1] - 5 * p1[1] + 4 * p2[1] - p3[1]) * t2
                + (-p0[1] + 3 * p1[1] - 3 * p2[1] + p3[1]) * t3
            )
            result.append((round(x), round(y)))
    return result


def main():
    reference = Image.open(REFERENCE).convert("RGBA")

    # Points lie on the outside of the dark TPHD perimeter, clockwise from the
    # upper-left corner. They are traced from the supplied 518x614 close-up;
    # the shallow top/bottom splines and unequal side bows are retained rather
    # than replaced with a synthetic symmetric arc.
    contour = catmull_rom([
        # Keep the mask on the dark outside stroke itself. The first trace ran
        # along the stroke's outer fringe, where the blurred reference carries
        # a few orange scene pixels. This inset retains the complete black rim
        # while preventing those background colors from entering the texture.
        (111, 91), (141, 85), (188, 82), (235, 80),
        (284, 80), (332, 83), (382, 85), (416, 90),
        (419, 127), (421, 176), (421, 226), (419, 277), (416, 322),
        (378, 326), (330, 328), (280, 326), (230, 325),
        (181, 327), (141, 326), (115, 322),
        (113, 285), (114, 239), (115, 190), (114, 142),
    ])

    mask = Image.new("L", reference.size, 0)
    ImageDraw.Draw(mask).polygon(contour, fill=255)
    # Pull the alpha boundary through the dark outline rather than leaving it
    # on the blurred source's colored exterior fringe. The erosion follows the
    # already traced spline, so it preserves the TPHD silhouette.
    mask = mask.filter(ImageFilter.MinFilter(9))
    reference.putalpha(mask)

    crop = reference.crop((98, 68, 434, 343))
    crop.thumbnail((256, 208), Image.Resampling.LANCZOS)
    # Lanczos creates partially transparent edge texels. Give those texels the
    # dark rim's RGB instead of the captured scene's orange RGB; this prevents
    # GPU bilinear filtering from producing a colored halo outside the border.
    pixels = crop.load()
    for y in range(crop.height):
        for x in range(crop.width):
            red, green, blue, alpha = pixels[x, y]
            if alpha < 255:
                pixels[x, y] = (28, 24, 21, alpha)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    crop.save(OUTPUT)
    runtime_icon = crop.resize((128, 104), Image.Resampling.LANCZOS)
    BTI_OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    BTI_OUTPUT.write_bytes(encode_rgba8_bti(runtime_icon))
    print(OUTPUT)
    print(BTI_OUTPUT)


if __name__ == "__main__":
    main()
