from source_helpers import read_hook_source
"""Third-item presentation must not change native Midna/transform availability."""
from pathlib import Path

source = read_hook_source()

assert "dMeter2Info_onUseButton(METER2_USEBUTTON_Z)" not in source
assert "dMeter2Info_offUseButton(METER2_USEBUTTON_Z)" not in source

player_update = source.split("void after_player_execute(", 1)[1].split(
    "template <class Hook>", 1
)[0]
assert "s_zHudItemUsable = false;" in player_update
assert "s_zHudItemUsable = third_slot_item_usable(" in player_update

print("PASS: third-item HUD leaves native Midna and Quick Transform availability untouched")
