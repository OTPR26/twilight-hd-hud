"""Colored face textures need initialized service buffers and packaged assets."""
from pathlib import Path
import struct

root = Path(__file__).resolve().parents[1]
source = (root / "src/item_slot_hooks.cpp").read_text()
buffers = source.split("ResourceBuffer s_playStationFaceButtonResources[3][4] = {", 1)[1].split("};", 1)[0]
assert buffers.count("RESOURCE_BUFFER_INIT") == 12
for symbol in ("circle", "cross", "triangle", "square"):
    path = f"hud/face-button-ps-{symbol}-colors.bti"
    assert path in source
    data = (root / "res" / path).read_bytes()
    assert data[0] == 6
    assert struct.unpack_from(">HH", data, 2) == (64, 64)
    assert len(data) == 32 + 64 * 64 * 4
print("PASS: all PlayStation texture buffers initialized and colored assets present")
