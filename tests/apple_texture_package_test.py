import struct
from pathlib import Path


root = Path(__file__).parents[1]
cmake = (root / "CMakeLists.txt").read_text()

assert "set(TWILIGHT_HUD_TEXTURES_DIR textures-universal)" in cmake
assert "TEXTURES_DIR ${TWILIGHT_HUD_TEXTURES_DIR}" in cmake

textures = (
    "TPHD/Menus/Buttons/tex1_40x40_0bcb924b474aa8a7_2.dds",
    "TPHD/Menus/Buttons/tex1_16x16_9593036f62f0cdee_2.dds",
)

for relative_path in textures:
    desktop = root / "textures" / relative_path
    apple = root / "textures-universal" / relative_path
    assert desktop.is_file(), desktop
    assert apple.is_file(), apple

    # Both are DDS DX10 files. The DXGI format is the first uint32 in the
    # 20-byte DX10 extension header, which starts at byte 128.
    desktop_format = struct.unpack_from("<I", desktop.read_bytes(), 128)[0]
    apple_format = struct.unpack_from("<I", apple.read_bytes(), 128)[0]
    assert desktop_format == 98, (desktop, desktop_format)  # BC7_UNORM
    assert apple_format == 28, (apple, apple_format)  # RGBA8_UNORM
    data = apple.read_bytes()
    h, w = struct.unpack_from('<II', data, 12)
    mips = max(1, struct.unpack_from('<I', data, 28)[0])
    assert len(data) == 148 + sum(max(1, w >> i) * max(1, h >> i) * 4 for i in range(mips))

print("PASS: all platforms select complete RGBA8 D-pad textures")
