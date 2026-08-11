#!/usr/bin/env python3
"""Hide the legacy Wii Z prompt panes in main2D.arc.

The third item-assignment slot remains intact for the mod's R-button artwork;
only the original Z glow, Z glyph, and action-text parent start hidden.
"""

from pathlib import Path


ARCHIVE = Path(__file__).resolve().parents[1] / "overlay/res/Layout/main2D.arc"

# Absolute RARC offsets. Each entry validates its surrounding pane signature
# before changing the J2D visibility byte from 0x04 to 0x00.
TARGETS = (
    (0x14FAD, b"z_btnl", "legacy Z glow"),
    (0x1502D, b"z_btn_t", "legacy Z glyph"),
    (0x15FF5, b"z_text_n", "legacy Z action-text parent"),
)


def main() -> None:
    data = bytearray(ARCHIVE.read_bytes())
    if data[:4] != b"RARC":
        raise SystemExit(f"{ARCHIVE} is not a RARC archive")

    changed = False
    for visibility_offset, pane_name, description in TARGETS:
        window = data[visibility_offset - 24 : visibility_offset + 40]
        if pane_name not in window:
            raise SystemExit(
                f"refusing to patch {description}: {pane_name!r} was not near "
                f"archive offset 0x{visibility_offset:x}"
            )
        current = data[visibility_offset]
        if current == 0x00:
            print(f"already hidden: {description}")
            continue
        if current != 0x04:
            raise SystemExit(
                f"refusing to patch {description}: expected visibility 0x04 at "
                f"0x{visibility_offset:x}, found 0x{current:02x}"
            )
        data[visibility_offset] = 0x00
        changed = True
        print(f"hidden: {description}")

    if changed:
        ARCHIVE.write_bytes(data)


if __name__ == "__main__":
    main()
