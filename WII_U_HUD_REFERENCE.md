# Twilight Princess HD HUD Reference

This document records the baseline used by the Wii U HUD preset. The primary reference is
Nintendo's North American electronic manual for *The Legend of Zelda: Twilight Princess HD*:

https://csassets.nintendo.com/noaext/image/private/t_KA_PDF/manual-WiiU-Legend_of_Zelda-Twilight_Princess_HD

## Main-screen layout

- Life is anchored at the upper left.
- The Midna prompt sits below the life gauge and flashes when she has something to say.
- The D-Pad display is independent of the minimap.
- The minimap is smaller than the GameCube presentation and uses a translucent dark backing.
- Action and equipped-item icons occupy the upper-right area.
- Rupees remain at the lower right.
- Context-sensitive prompts change with Link's current action and gameplay state.

## Controls represented by the HUD

- `X`, `Y`, and `R`: three equipped-item slots.
- `L`: talk to or interact with Midna.
- `ZL`: target and defend.
- `A`: context-sensitive action.
- `B`: sword attack in human form and bite/attack in wolf form.
- D-Pad Up: map screen.
- D-Pad Down: collection screen.
- D-Pad Left/Right: toggle the minimap display.
- Right stick: free camera.
- GamePad motion: optional item aiming.

## State-dependent behavior to verify

- Human versus wolf action icons.
- Midna unavailable, available, and flashing states.
- Minimap hidden, field minimap, and dungeon minimap states.
- Item drawn, aimed, combined, and put-away states.
- Horse spur display.
- Oxygen, lantern oil, and Vessel of Light gauges.
- First-person and third-person aiming.
- Cutscenes, conversations, shops, and paused screens.
- Standard television output versus off-TV play.

## Current implementation boundary

The package can reposition and scale the existing GameCube HUD panes, provide the third item slot,
keep the D-Pad independent of the minimap, and redirect Midna behavior. Physical L/ZL/R/ZR labels
and bindings still depend on the active Dusklight controller profile. Exact coordinates will be
refined against a live Cemu reference at the target display aspect ratio.
