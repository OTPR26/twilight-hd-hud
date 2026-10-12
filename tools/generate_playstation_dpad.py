"""Four PlayStation directional buttons in grey and black."""
from PIL import Image, ImageDraw
from generate_playstation_buttons import ROOT, OUTPUT_DIR, encode_rgba8_bti

size, scale = 64, 4
source = ROOT / 'assets/source/buttons'
mask = Image.new('L', (size * scale, size * scale))
draw = ImageDraw.Draw(mask)
# Rounded outer cap and a pointed inner end.
draw.rounded_rectangle((24*scale, 4*scale, 40*scale, 22*scale), radius=4*scale, fill=255)
draw.polygon([(24*scale, 20*scale), (40*scale, 20*scale), (32*scale, 28*scale)], fill=255)

for dark in (False, True):
    image = Image.new('RGBA', (size*scale, size*scale))
    color = (67, 70, 77, 255) if dark else (192, 194, 199, 255)
    for direction in range(4):
        button = Image.new('RGBA', image.size, color)
        button.putalpha(mask.rotate(direction*90))
        image.alpha_composite(button)
    image = image.resize((size, size), Image.Resampling.LANCZOS)
    name = 'playstation-dpad-' + ('black' if dark else 'grey')
    image.save(source / f'{name}.png')
    (OUTPUT_DIR / f'{name}.bti').write_bytes(encode_rgba8_bti(image))
