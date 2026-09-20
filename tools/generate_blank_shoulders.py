"""Blank TPHD shoulder caps shared by every controller layout."""
from PIL import Image, ImageOps
from generate_playstation_buttons import ROOT, OUTPUT_DIR, encode_rgba8_bti, make_wii_u_shoulder_button, make_shoulder_button

source = ROOT / 'assets/source/buttons'
source.mkdir(parents=True, exist_ok=True)
for name, image in (
    ('silver', ImageOps.mirror(make_wii_u_shoulder_button('', False))),
    ('black-pro', ImageOps.mirror(make_wii_u_shoulder_button('', True))),
    ('transparent', Image.new('RGBA', (64, 64))),
    ('rounded-silver', make_shoulder_button('', False)),
    ('rounded-black-pro', make_shoulder_button('', True)),
):
    stem = f'shoulder-button-blank-{name}'
    image.save(source / f'{stem}.png')
    (OUTPUT_DIR / f'{stem}.bti').write_bytes(encode_rgba8_bti(image))
