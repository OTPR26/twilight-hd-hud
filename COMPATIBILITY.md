# Mod Compatibility

## Dawnlight

Twilight HD HUD and Dawnlight intentionally share several Dusklight HUD hooks.
They can coexist because their responsibilities and hook order are distinct:

The v1.5.1 compatibility review used Dawnlight **v2.3.3**.

- Twilight HD HUD supplies TPHD artwork, menu presentation, prompts, and control behavior.
- Dawnlight's HUD Layout Editor supplies the user's final gameplay-HUD positions.
- Dawnlight restores shared native HUD state at pre-draw priority `+100`, Twilight
  HD HUD styles at normal priority, and Dawnlight applies saved positions at
  pre-draw priority `-100`.

Do not move Twilight HD HUD's shared Meter Draw, Gauge Screen, or Minimap hooks
outside normal priority without testing both mods together. Do not permanently
reset shared pane positions after Dawnlight's final layout pass.

### Release checklist

For every release that changes gameplay HUD drawing, minimap layout, gauges,
button prompts, or item slots:

1. Review the shared hook priorities in the current Dawnlight release.
2. Test both mods enabled with Twilight HD HUD before Dawnlight in the mod list.
3. Repeat with Dawnlight before Twilight HD HUD.
4. Confirm Dawnlight position changes remain visible for hearts, buttons, rupees,
   minimap, oil/oxygen/lantern meters, and third-item HUD elements.
5. Confirm Twilight HD HUD menus, button artwork, touch controls, and item behavior
   remain functional.

The compatibility claim applies to released versions that pass this checklist;
future Dawnlight changes may require a new review.
