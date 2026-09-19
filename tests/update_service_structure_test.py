"""Keep update ownership in Dusklight, not in the HUD mod."""
from pathlib import Path

root = Path(__file__).parents[1]
ui = (root / "src/ui.cpp").read_text()
assert '"Open Twilight HD Settings"' in ui
for obsolete in ("update_service.cpp", "update_service.hpp"):
    assert not (root / "src" / obsolete).exists()
source = "\n".join(p.read_text() for p in (root / "src").iterdir()
                   if p.suffix in {".cpp", ".hpp", ".inc"})
for obsolete in ("Auto Update Checks", "Check Now", "svc_http",
                 "check-for-updates", "initialize_update_service",
                 "api.github.com/repos/OTPR26"):
    assert obsolete not in source, obsolete
assert "src/update_service.cpp" not in (root / "CMakeLists.txt").read_text()
print("PASS: settings retained; custom updater and HTTP dependency removed")
