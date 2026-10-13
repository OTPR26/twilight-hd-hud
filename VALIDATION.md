# Release validation

Run the native tests with `ctest --test-dir build --output-on-failure`.
The platform build workflow also checks resources, native hook behavior,
architecture, package metadata, texture formats, and archive integrity.

Gameplay checks:

- Assign bomb arrows and Hawkeye to X, Y and R. Verify both icon layers dim in Castle
  Town and return to full opacity outside town, including after changing rooms
  and reducing HUD opacity. Check an ordinary R-slot item as well.
- Compare R against X/Y with the same item while swimming, climbing/hanging,
  riding Epona, canoeing, riding the Spinner, and snowboarding. Check empty
  ammo and item-specific exceptions using the native slots as the reference.
  Verify readied items stay usable when no new equip action is needed.
- Repeat the combo and restriction checks in the dual-screen top HUD, both
  with and without the host's optional isItemUsable export.
- Present Renado's Letter from R with X empty or holding another item; compare
  with X/Y presentation and another trade-item NPC.
- Test maps with D-Pad Shortcuts on and off, including combined Up cycling,
  Midna bindings, touch input, and minimap state after warping.
- Check fonts, sizing controls, and controller prompts in gameplay and menus.
- Check Save and Collection in each Dusklight scaling mode.
- Test update checks and installation from an older version. iOS and tvOS must
  offer notices only; their native updates require a signed app build.
- Follow the Dawnlight checks in [COMPATIBILITY.md](COMPATIBILITY.md).

Native hook tests use mocked game state. They do not replace the quest-specific
in-game checks above.

R-slot fade change (2026-10-13): The first test build synchronized combo-layer
opacity, but the Odin still showed bright bomb arrows in Castle Town. The
revised build also checks checkCastleTownUseItem rather than trusting BTN_Z
alone. Android and macOS packages build; all 27 native tests and the third-slot
interaction/cutscene/talk and Quick Transform checks pass. The revised Android
package is copied to the Odin and its SHA-256 matches the local build. After
reloading, the user confirmed that the R-slot bomb-arrow fade now works in
Castle Town on the Odin. Hawkeye, permitted town items, reduced HUD opacity,
and restoration outside town still need the gameplay checks listed above.

Extended fade change: native item-action eligibility now supplements the
input bit for R, while a currently readied item retains its native input
availability. X/Y and R combo siblings share opacity. The dual-screen top
HUD applies stage/action restrictions and uses native availability if its
optional host query is absent. Automated native and mocked-hook tests pass.
The user reports the extended test build looks correct and all their checks
pass on the Odin. No separate dual-screen host/device test was observed here;
that path is covered by the mocked-hook tests.
The extended Android test package is installed on the Odin with matching
SHA-256 785a78fd3a286f90ac3414ebc7ce7d2355e07ac769afea8ae4b5a649c1f6f17b.
The previously confirmed Castle Town package is backed up locally.
