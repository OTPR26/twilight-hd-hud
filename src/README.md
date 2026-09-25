# Hook and screen code

`item_slot_hooks.cpp` owns shared resources, hook callbacks, and registration.
Screen implementations are included in declaration and initialization order.

The screen files are implementation fragments, not separate translation units:

- `options_screen.inc`: Options, confirmation prompts, TV and brightness settings.
- `file_select_screen.inc`: file selection, copying, erase confirmation, and metadata.
- `save_screen.inc`: in-game save selection and confirmation styling.
- `dungeon_map_screen.inc`: dungeon-map layout, floor rows, and prompts.
- `overworld_map_screen.inc` and `map_responsive_screen.inc`: world-map styling and viewport adaptation.
- `collection_screen.inc`, `item_bank_screen.inc`, and `dialogue_text_screen.inc`: Collection, Items, and dialogue styling.

The fragments still depend on shared helpers and state in the main file. Moving
them into independent `.cpp` files requires explicit interfaces; the current
split does not remove that coupling or reduce the compiled code size.

Keep hook registration in the main file. Preserve native animation attachments
and distinguish styling from input/state changes when modifying screens.
Structural tests use `tests/source_helpers.py` to read the expanded implementation,
so extracting a screen does not bypass its existing regression checks.
