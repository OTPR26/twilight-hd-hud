"""The silver L keeps the original curved cap and repairs only its inset face."""
from pathlib import Path
import struct

root = Path(__file__).resolve().parents[1]
script = (root / 'tools/generate_playstation_buttons.py').read_text()
main = script.split('if __name__ == "__main__":', 1)[1]
assert 'make_wii_u_l_from_archive_r()' in main
assert 'make_wii_u_shoulder_button(label, False)' not in main
assert 'image.getchannel("A").tobytes() == original.getchannel("A").tobytes()' in script
data = (root / 'res/hud/shoulder-button-l.bti').read_bytes()
assert data[0] == 6
assert struct.unpack_from('>HH', data, 2) == (83, 83)
assert len(data) == 32 + 84 * 84 * 4
print('PASS: original L silhouette, inset-only repair and padded RGBA8 payload')
