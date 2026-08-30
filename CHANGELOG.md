# Changelog

## v1.5.6 - 2026-08-30

- Fix a Windows startup failure caused by touch-only virtual-input observation
  hooks being registered as mandatory desktop hooks.
- Keep the v1.5.5 HUD layout, artwork, and settings unchanged.

## v1.5.5 - 2026-08-30

- Refine D-pad and Midna/L-button sizing, layering, and vertical placement.
- Make HUD sizing controls resize icons without changing their associated text.
- Correct overworld and dungeon map Poe-count alignment.
- Introduce TPHD-inspired item-description cards.
- Include other minor bug fixes and improvements.

## v1.5.1 - 2026-08-29

- Correct touch-control routing for Collection, Midna, targeting, and the third item slot.
- Hide redundant native HUD controls while the touch overlay is active and restore B/X/Y item artwork after touch controls are disabled.
- Fix the third-item assignment animation so the correct item travels to R/RB.
- Align the overworld Poe Soul counter with its icon.
- Preserve Dawnlight HUD Layout Editor overrides through an explicit shared-HUD compatibility contract.

## v1.5.0 - 2026-08-29

- Add TPHD-inspired Fish Journal, Golden Bugs, Hidden Skills, and Letters collection submenus.
- Support touch, D-Pad, and analog-stick navigation across Collection submenus.
- Center oil, oxygen, and lantern meters consistently across display sizes.
- Fix File Selection D-Pad navigation and preserve the minimap's hidden state after closing a map.
- Include additional input, layout, cursor, and cross-platform stability fixes.

## v1.4.3 - 2026-08-28

- Refine dungeon and overworld maps, including narrow-window scaling and D-Pad Up to go back.
- Add action text, dialogue text, rupee, and minimap scaling controls.
- Match TPHD action/dialogue sizing and place the bottom-center A prompt before its text.
- Fix Collection vertical navigation and Items button/background sizing.

## v1.4.2 - 2026-08-27

- Add the Items / Collection button-swap toggle while keeping the TPHD Bank layout available.

## v1.4.1 - 2026-08-27

minor fixes and adjustments

## v1.4.0 - 2026-08-27

- Wallet description hotfix: Collection now reads the live wallet capacity,
  automatically matching Dusklight's Bigger Wallets setting without an extra
  toggle or ISO edits. Existing v1.4.0 downloads were refreshed in place.
- Add the TPHD-style Items bank with fixed item positions, a patterned title
  banner, darker cells, soft shadows, and Ooccoo's circular bottom-right slot.
- Keep the original item wheel available through the new Items Screen setting.
- Move Collection/Save to D-Pad Down and Items to Start / +; Start or B closes
  Items, while D-Pad navigation inside menus remains unchanged.
- Center Collection/Save beneath the D-Pad and refine Minimap's vertical alignment.
- Preserve custom Call Midna bindings and existing controller profiles.
- Add regression coverage for bank layout, navigation, settings, menu shortcuts,
  artwork, and D-Pad label spacing.

## v1.3.6 - 2026-08-27

- TPHD-style Collection screen with parchment descriptions.
- Updated Collection banner, equipped-item frames, and decorative details.
- Link starts facing forward while keeping Dusklight's model rotation.
- Corrected and aligned controller icons in Collection descriptions.

## v1.3.5 - 2026-08-27

- Top-centered oil/oxygen meters.
- Howl button matching the selected layout/style.
- Updated item-help icons, including ZL/LT/L2 for bomb-arrow combinations.
- Assignment icons hidden while item descriptions are open.

## v1.3.1 - 2026-08-27

- Fix L, R, ZL, and ZR gameplay actions leaking through while Dusklight's
  menu is open. Held shoulders and triggers must be released before they
  can activate gameplay again after closing the menu.
- No other gameplay, layout, font, or settings changes.

## v1.3.0 - 2026-08-26

- Add four text-font choices: Original, Zen Kaku Gothic New, M PLUS 2, and
  Dusklight's Fira Sans family. Font changes require a restart and preserve
  the game's text spacing and line wrapping.
- Add separate controller-diamond, D-Pad, and heart sizes, with whole-percent
  values from 50% to 125% and reset-to-100% buttons. Overall sizing overrides
  individual values instead of multiplying them.
- Add **BAYX Flipped**, with Attack/B on the east and Action/A on the south
  of the diamond. Existing controller bindings are unchanged.
- Use **LB, RB, LT, and RT** prompts on both Xbox layouts, in Silver and
  Black Pro styles.
- Restyle the Options save/discard confirmation and fit its question box to
  the text. Correct the Brightness / Screen Ratio trigger prompt.
- Replace Wolf Link's separate Dig prompt with the selected button artwork.
- Correct the item-ring Direct Select icon and Bow & Arrow Combo label.
- Simplify the settings descriptions and preserve existing saved selections.
- Add regression tests for fonts, HUD sizing, settings migration, and prompts.

## v1.2.0 - 2026-08-25

- Redesign the Quest Log screen with *Twilight Princess HD*-inspired panels.
- Move Midna to **L/L1** when using **TPHD Fixed Bindings** and add matching
  artwork for every supported button style and controller layout.
- Add a TPHD-style map icon and improve the D-Pad HUD presentation.
- Update map and portal prompts, including a clearer **ZR** prompt.
- Respect the custom Midna binding when using **Follow Dusklight Bindings**.
- Fix black prompt boxes appearing after resetting Dusklight.
- Improve HUD behavior when Midna is assigned to **R** or a D-Pad direction.
- Include additional bug fixes and stability improvements.

## v1.1.1 - 2026-08-23

- Center the localized **Yes** and **No** labels inside the HD map-warp
  confirmation panels by removing the original GameCube text-positioning
  commands before drawing the replacement labels.
- Keep two-choice labels anchored to their own HD frames without moving the
  shared choice containers, preserving shop option placement and eliminating
  the flashing introduced by earlier centering attempts.

## v1.1.0 - 2026-08-23

- Extend the *Twilight Princess HD*-style choice panels and selection treatment
  from Midna's prompt to every shared three-choice dialogue screen.
- Preserve each dialogue's native localized choice text instead of applying
  Midna's Transform, Warp, and Talk labels to unrelated conversations.
- Automatically reduce the label size when a longer choice needs additional
  room while retaining the same panel dimensions, padding, and centered layout.

## v1.0.0 - 2026-08-22

- Add a complete PlayStation-symbol button layout, including matching Silver
  and Black Pro face buttons, **R1**, **L2**, and **R2** prompts across the HUD,
  item wheel, menus, maps, and contextual actions.
- Keep physical **R/R1** dedicated to the third item while preserving **ZR** as
  logical GameCube R for Gale Boomerang targeting and Dusklight's **R+X/R+Y**
  Sun's Song and Quick Transform combinations.
- Redesign the Quest Log and in-game Save rows around one shared responsive
  layout with matching metadata, rules, selected-row treatment, cursor
  spacing, and heart-safe minimum and maximum widths at 4:3 and widescreen.
- Correct the file-selection flourish, number art, prompt placement, and
  reset-only footer residue.
- Restyle Midna's three-choice prompt and prevent its labels from wrapping or
  colliding.
- Preserve the completed Collection-screen layout and related submenu prompt
  styling while applying the selected controller artwork consistently.
- Validate the release package on Windows x64, macOS Apple Silicon, and Android
  ARM64 with Dusklight v1.4.1 builds in the supported 141-or-newer range.

## v0.9.1 - 2026-08-20

- Correct Gale Boomerang multi-target selection to use **ZR**, matching
  *Twilight Princess HD*, while leaving **ZL** available for ordinary enemy
  lock-on.
- Replace the boomerang targeting prompt with matching silver and Black Pro
  **ZR** artwork.

## v0.9.0 - 2026-08-20

- Require Dusklight **v1.4.1-141 or newer** and build every native module
  against the compatible upstream revision.
- Redesign the Collection screen and its Save Game, Options, title, prompt,
  border, and selection treatments to more closely match *Twilight Princess
  HD* across arbitrary desktop aspect ratios and Android displays.
- Apply the selected Nintendo, Xbox, or Universal button artwork throughout
  Collection, Letters, Fish Journal, Hidden Skills, Golden Bugs, maps, save
  prompts, and gameplay HUD contexts.
- Keep the R-slot item and ammunition counter intact across room transitions,
  pit recoveries, transformations, and cutscenes.
- Prevent reusable R-slot text layers from retaining undersized buffers, which
  could crash Windows during room transitions such as Arbiter's Grounds.
- Correct Windows-only Collection prompt placement, selection-bracket sizing,
  and the transient title jump when opening the menu or returning from a
  Collection submenu.
- Keep the Android bow ammunition counter aligned with the desktop HUD.

## v0.8.5 - 2026-08-14

- Move boomerang lock-on to ZL when using **TPHD Fixed Bindings**.
- Restore D-Pad Up navigation across dungeon maps and menu screens.
- Keep L shield and ZL lock actions separate with fixed bindings.
- Apply the selected controller style to Collection menu prompts.
- Prevent save-confirmation titles from overlapping during transitions.

## v0.7.5 - 2026-08-13

- Add **Follow Dusklight Bindings** and **TPHD Fixed Bindings** controller
  compatibility modes without rewriting the user's controller profile.
- Prevent a configured **Call Midna** control from also performing its normal
  game action in Follow mode.
- Restore logical R for the third item and related menu actions in both
  controller compatibility modes.
- Restore reliable D-Pad Up navigation in File Selection and naming screens
  while keeping D-Pad Up assigned to Midna during gameplay.
- Honor Dusklight's separate **Call Midna** action and prevent a shared input
  from activating the third item at the same time.
- Apply Nintendo, Xbox, and Universal face-button artwork consistently across
  Options, save, File Selection, field-map, and dungeon-map prompts.
- Stabilize the Copy and Erase confirmation buttons so their positions no
  longer jump during selection changes.
- Correct the Quest Log title baseline and retain the v0.6 menu and map
  presentation across supported aspect ratios.

## v0.6.0 - 2026-08-12

- Restyle the Options and Device Settings screens with the cleaner
  *Twilight Princess HD*-inspired menu treatment.
- Replace legacy stone-wall menu backgrounds and update controller prompts
  with the matching A, B, and R button artwork.
- Redesign the field map header, background, and controls while keeping the
  map and prompt layout stable across aspect ratios and window resizing.
- Bring dungeon maps into the same visual system with updated titles,
  typography, prompts, and background treatment.
- Correct menu descenders and remove leftover native frame decorations.

## v0.5.0 - 2026-08-11

- Redesign the File Selection, file-copy, save, and Collection screens with a
  cleaner *Twilight Princess HD*-inspired presentation.
- Keep the selection brackets on **Start** while the selected save file begins
  loading.
- Complete the third item slot across the HUD and item wheel, using R for
  assignment and activation.
- Improve bow-combination readability by drawing the bow behind its attachment
  and reducing the attachment icon by 40%.
- Anchor the R-slot ammunition count to the equipped item artwork so bows and
  other counted items use the same visual placement.
- Refine face-button prompts, wolf actions, D-Pad behavior, minimap placement,
  counters, and HUD scaling.
- Add native packages for Windows x64, Linux x64, macOS Apple Silicon, Android
  ARM64, iOS ARM64, and tvOS ARM64.

## v0.1.3 - 2026-08-08

- Fix Windows package loading by avoiding an unsupported item-ring cleanup
  hook.
- Improve hook failure reporting.
- Refresh the R-slot item texture after wolf-to-human transformations.
