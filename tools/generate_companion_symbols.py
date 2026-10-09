"""Standalone PlayStation labels for the companion item buttons."""
from PIL import Image, ImageDraw
from generate_playstation_buttons import ROOT, OUTPUT_DIR, SIZE, SCALE, draw_symbol, encode_rgba8_bti

colors = {"circle": (240, 90, 110, 255), "cross": (125, 145, 245, 255),
          "triangle": (80, 210, 160, 255), "square": (225, 140, 195, 255)}
source = ROOT / "assets/source/buttons"
for colored in (False, True):
    for symbol, color in colors.items():
        image = Image.new("RGBA", (SIZE*SCALE, SIZE*SCALE))
        draw_symbol(ImageDraw.Draw(image), symbol,
                    color if colored else (242, 242, 239, 255), (0, 0, 0, 210))
        image = image.crop((18*SCALE, 18*SCALE, 46*SCALE, 46*SCALE))
        image = image.resize((32, 32), Image.Resampling.LANCZOS)
        name = f"companion-symbol-{symbol}" + ("-colors" if colored else "")
        image.save(source / f"{name}.png")
        (OUTPUT_DIR / f"{name}.bti").write_bytes(encode_rgba8_bti(image))
