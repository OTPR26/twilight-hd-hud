"""Encode the high-resolution companion wolf portrait as RGBA8."""
from PIL import Image
from generate_playstation_buttons import ROOT, OUTPUT_DIR, encode_rgba8_bti

image = Image.open(ROOT / "assets/source/companion/wolf.png")
(OUTPUT_DIR / "companion-wolf.bti").write_bytes(encode_rgba8_bti(image))
