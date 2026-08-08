# Twilight HD HUD

A focused Twilight Princess HD-style HUD and controller-layout package for clean upstream
Dusklight builds.

## Included

- Upper-left life, Midna and independent D-Pad grouping based on the Wii U presentation.
- Smaller lower-left minimap placement that respects the active texture set.
- Three item slots corresponding to X, Y and R.
- Midna assigned to D-Pad Up for reliable portable and controller support.
- Stationary D-Pad behavior when the minimap is shown or hidden.
- Wii U-style upper-right action and item-icon arrangement.
- Round X/Y button presentation.
- Support for the existing lantern, oxygen, rupee and context-sensitive HUD states.

## Controller profile

Twilight Princess HD uses this physical layout:

- X, Y and R: equipped items
- D-Pad Up: Midna
- D-Pad Down: Items
- ZL: targeting and defending
- A: context action
- B: attack
- D-Pad Left/Right: minimap toggle
- Right stick: free camera

L and ZL must be assigned separately in Dusklight's controller settings. Automatic installation
of a complete physical controller profile is not yet exposed by the current mod interface.

## Installation

Copy `twilight_hd_hud.dusk` into the `mods` directory inside the active Dusklight data folder,
then restart the game or reload mods from the mod manager.

## Texture-pack compatibility

This package does not include a generic texture pack. It contains the layout archive, custom HUD
resources, and two targeted replacements required for the silver D-Pad and softly shaded Universal
face buttons. All other external texture replacements remain authoritative.
