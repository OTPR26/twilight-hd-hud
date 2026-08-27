"""Verify the bundled font atlases and pinned sources without regenerating them."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
fonts = root / "res/fonts"
manifest = json.loads((fonts / "font-atlases.json").read_text())
for entry in manifest.values():
    atlas = fonts / entry["atlas_file"]
    source = root / "tools/font_sources" / entry["source_file"]
    assert hashlib.sha256(atlas.read_bytes()).hexdigest() == entry["atlas_sha256"], atlas.name
    assert hashlib.sha256(source.read_bytes()).hexdigest() == entry["source_sha256"], source.name
for name in ("Fira-Sans", "M-PLUS-2", "Zen-Kaku-Gothic-New"):
    assert "SIL OPEN FONT LICENSE" in (fonts / f"OFL-{name}.txt").read_text()
print("PASS: font source and atlas checksums; all three font licenses present")
