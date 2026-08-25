# Twilight HD HUD

A focused Twilight Princess HD-style HUD and controller-layout package for clean upstream
Dusklight builds.

## Included

- Upper-left life, Midna and independent D-Pad grouping based on the Wii U presentation.
- Smaller lower-left minimap placement that respects the active texture set.
- Three item slots corresponding to X, Y and R.
- Midna assigned to L in TPHD Fixed Bindings, or to the configured Dusklight control in Follow mode.
- Stationary D-Pad behavior when the minimap is shown or hidden.
- Wii U-style upper-right action and item-icon arrangement.
- Round X/Y button presentation.
- Optional grey-and-black PlayStation Circle, Cross, Triangle, Square, L2, R1, and R2 prompts.
- Support for the existing lantern, oxygen, rupee and context-sensitive HUD states.

## Controller profile

Twilight Princess HD uses this physical layout:

- X, Y and R: equipped items
- L: Midna in TPHD Fixed Bindings
- D-Pad Down: Items
- ZL: targeting and defending
- ZR: Gale Boomerang multi-target lock
- A: context action
- B: attack
- D-Pad Left/Right: minimap toggle
- Right stick: free camera

Physical shoulder and trigger bindings depend on the active Dusklight controller profile.
**TPHD Fixed Bindings** distinguishes R from ZR for the third item and Gale Boomerang actions.

## Installation

Copy `twilight_hd_hud.dusk` into the `mods` directory inside the active Dusklight data folder,
then restart the game or reload mods from the mod manager.

## Texture-pack compatibility

This package does not include a generic texture pack. It contains the layout archive, custom HUD
resources, and two targeted replacements required for the silver D-Pad and softly shaded Universal
face buttons. All other external texture replacements remain authoritative.
