# Changelog

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
