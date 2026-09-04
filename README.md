# Twilight HD

Twilight HD brings a *Twilight Princess HD*-inspired HUD and controller layout to [Dusklight](https://github.com/TwilitRealm/dusklight).

## Compatibility

Dusklight Nightly **v1.4.1-268 or newer** is required. The mod is built and tested against the official Dusklight repository.

Twilight HD is designed to coexist with **Dawnlight**. Twilight HD owns
the TPHD artwork, menu presentation, prompts, and control behavior; Dawnlight's
HUD Layout Editor may apply the user's final gameplay-HUD positions. Shared HUD
hooks must remain at normal priority so Dawnlight can restore native state first
and apply saved layout overrides last. Compatibility is reviewed during each
release; see [COMPATIBILITY.md](COMPATIBILITY.md) for the maintenance checklist.

## Features

- Wii U-inspired buttons, D-Pad, minimap, rupee placement
- Three item slots using X, Y, and R
- Independent D-Pad and minimap behavior
- Human and wolf action artwork
- ABXY, BAYX, BAYX Flipped, PlayStation-symbol, and blank button-label options
- Four text fonts: Original, Zen Kaku Gothic New, M PLUS 2, and Dusklight's Fira Sans family.
- Editable HUD sizes from 50% to 125% in 1% steps, with reset-to-100% buttons.
  Overall HUD Size overrides visual HUD groups when it is not 100%, without
  changing text. Action Text and Dialogue Text remain independent. At 100%,
  saved Controller Diamond, D-Pad, Hearts, Rupee, and Minimap percentages apply
  independently, without multiplication. Changes apply live.
- *Twilight Princess HD*-style panels for shared three-choice dialogue screens
- TPHD-style Collection screen with parchment descriptions, decorated equipment
  frames, and a forward-facing Link model that retains Dusklight's rotation controls
- TPHD-inspired Fish Journal, Golden Bugs, Hidden Skills, and Letters submenus,
  with touch, D-Pad, and analog-stick navigation
- TPHD-style Items bank with fixed inventory cells, dark patterned backings,
  soft shadows, and Ooccoo's circular bottom-right slot; the original wheel
  remains selectable in settings
- Collection/Save on D-Pad Down and Items on Start / +, with aligned HUD labels
- Collection wallet descriptions automatically reflect Dusklight's Bigger Wallets
  setting; no additional toggle or game-image patch is needed
- Refined dungeon and overworld maps with responsive narrow-window layouts,
  patterned banners, and D-Pad Up to go back
- TPHD-sized action and dialogue text, with the bottom-center A prompt on the left
- Centered oil, oxygen, and lantern meters across supported display sizes
- Native modules for Windows x64, Linux x64, macOS Apple Silicon, Android ARM64, iOS ARM64, and tvOS ARM64

Planned improvements are tracked in [ROADMAP.md](ROADMAP.md).
Release history is recorded in [CHANGELOG.md](CHANGELOG.md).

## Downloads

The release provides two `.dusk` packages, also available in ZIPs with installation notes:

- **Desktop and Android:** Windows, Linux, Steam Deck, macOS Apple Silicon, and Android ARM64
- **iOS and tvOS:** iPhone, iPad, and Apple TV

Download the package for the device running Dusklight. The packages contain the same mod assets and settings; only their native modules differ.

## Installation: Windows, Linux, macOS, and Android

1. Download the latest `.dusk` file from [Releases](../../releases/latest).
2. Place it in the `mods` folder inside the active Dusklight data folder.
3. Restart Dusklight or reload mods from its mod manager.
4. Open the Twilight HD settings panel to choose a button-label layout and HUD size.

On macOS, the active data folder is normally under `~/Library/Application Support/TwilitRealm/Dusklight`. On Android, use the `mods` folder inside the Dusklight data folder selected by the app.

If you download a ZIP, extract it first and install the `.dusk` file inside.
When updating, remove the previous Twilight HD package and keep only one
version in `mods`. Do not remove your saves, settings, or controller profiles.

## Items screen

The new **TPHD Bank** is the default. Select **Items Screen > Original Wheel**
in Twilight HD Settings to use the original layout instead; close and reopen
Items to apply the change. X, Y, and R item assignment and supported combinations
retain the game's inventory behavior.

By default, **Start / +** opens Items and **D-Pad Down** opens Collection/Save.
Turn **TPHD Items / Collection Buttons** off in Twilight HD Settings to
restore **D-Pad Down → Items** and **Start / + → Collection/Save** while keeping
the TPHD Bank layout. The preference is saved and the HUD label updates when
you return to gameplay; no restart is needed.

Inside Items and Collection, D-Pad directions remain navigation controls;
**Start / +** or **B** closes Items. On dungeon and overworld maps,
**D-Pad Up** goes back and the analog stick handles navigation.
In Follow mode, if Call Midna is assigned to Down, the
D-Pad menu shortcut moves to Right and Minimap to Left in either setting.
No controller profile is rewritten.

## Installation: iPhone and iPad

1. Download the **iOS and tvOS** `.dusk` file from [Releases](../../releases/latest).
2. Launch Dusklight once so its data folders are created.
3. Open the Files app and browse to **On My iPhone** or **On My iPad > Dusklight > mods**.
4. Copy the downloaded `.dusk` file into `mods`.
5. Return to Dusklight, then restart the app or reload mods from the mod manager.
6. Enable Twilight HD and open its settings panel to choose a button-label layout and HUD size.

If the Dusklight folder is not visible in Files, use the app's file-transfer page and upload the `.dusk` file to **Mods** instead.

## Installation: Apple TV

1. Download the **iOS and tvOS** `.dusk` file on a phone or computer connected to the same network as the Apple TV.
2. In Dusklight on Apple TV, open the file-transfer page and turn on local-network transfer.
3. Enter the complete address shown on the television in a web browser, including the colon before the port number.
4. Under **Mods**, choose the downloaded `.dusk` file and upload it.
5. On Apple TV, rescan or reload mods, then enable Twilight HD.
6. Restart Dusklight if the mod does not appear immediately.

The transfer page is available only while Dusklight is open, and both devices must remain on the same local network.

The package does not contain game files. A compatible, legally obtained disc image is required by Dusklight.

## Fonts and HUD size

Open **Twilight HD Settings** to choose a text font. **Original** keeps the
game's font; the other three options use open-source fonts. Fully quit and
restart Dusklight after changing fonts. Western-language message text is
supported; decorative fonts and unsupported characters keep their original
artwork. Text spacing and line wrapping are unchanged.

The **HUD Sizing** tab accepts whole percentages from **50% to 125%**. Set
**Overall HUD Size** to 100% to adjust the controller diamond, D-Pad, hearts,
rupees, and minimap separately. Any other overall value overrides those visual
controls without multiplying their saved percentages. D-Pad and controller
diamond sizing changes icons, not their labels; Action Text and Dialogue Text
remain independent of Overall. Each control has a reset-to-100% button. Use
Dusklight's **Minimal HUD** setting to hide the HUD.

## Controller layouts

Both **BAYX** layouts use Xbox shoulder/trigger prompts: LB, RB, LT, and RT,
in either Silver or Black Pro style.

The optional **BAYX Flipped** button layout retains Nintendo-style face-button
prompts in menus, dialogue, and item assignment. The gameplay diamond changes:
East B/Attack, South A/Action, North Y, West X. The complete Attack/Action
groups move, including labels and sword/Wolf artwork. This is a presentation
preset, not a controller-binding rewrite; standard Xbox A/B bindings already
put Attack on East and Action on South. Custom bindings remain unchanged.

- X, Y, and R: equipped items
- L: Midna in TPHD Fixed Bindings
- D-Pad Down: Collection/Save (Items with the menu swap off)
- Start / +: Items (Collection/Save with the menu swap off)
- D-Pad Left/Right: minimap toggle
- ZL: enemy lock-on/defend and paused item combinations
- ZR: Gale Boomerang multi-target lock
- A: context action
- B: attack
- Right stick: free camera

Physical controller bindings still depend on the active Dusklight controller profile.

The **Button Layout** setting includes a PlayStation option that displays Circle for the east
action, Cross for the south attack, Triangle and Square for the two face-button items, R1 for the
third item, L2 for enemy lock-on, and R2 for Gale Boomerang multi-target lock.

The **Button Style** setting switches between the standard silver TPHD prompts and dark graphite
**Black Pro** prompts. Button Style is independent of the ABXY, BAYX, Universal, and PlayStation
layouts, including BAYX Flipped.

The default **Follow Dusklight Bindings** controller mode leaves that profile unchanged and honors
the configured **Call Midna** action without also triggering the control's normal game action. Its
HUD prompt follows that assignment, including the default R shoulder and either vertical D-Pad
direction. The optional **TPHD Fixed Bindings** mode ignores the separate Call Midna binding and
keeps Midna exclusively on physical L. The third item follows logical GameCube Z (normally the controller's right
shoulder), while ZR follows logical GameCube R for Gale Boomerang targeting and Dusklight's R+X/R+Y
Sun's Song and Quick Transform combinations.

## Building

The project builds against the Dusklight mod SDK. Supported build targets are:

| Download | Native targets |
| --- | --- |
| Desktop and Android | Windows x64, Linux x64 / Steam Deck, macOS Apple Silicon, Android ARM64 |
| iOS and tvOS | iPhone and iPad ARM64, Apple TV ARM64 |

The included workflow builds and verifies Windows x64, Linux x64, macOS Apple Silicon, Android ARM64, iOS ARM64, and tvOS ARM64 against the compatible Dusklight revision recorded in the workflow file.

### Host-platform build

This command builds the native module for the current host or configured toolchain:

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DDUSK_DIR=/path/to/dusklight
cmake --build build
```

The resulting package is written to `build/mods/twilight_hd_hud.dusk`.

To run the native regression tests, configure with
`-DTWILIGHT_HUD_BUILD_TESTS=ON`, build, then run:

```sh
ctest --test-dir build --output-on-failure
python3 tests/font_manifest_test.py
```

### iOS and tvOS builds

Apple mobile targets require the iOS or tvOS CMake toolchain and the matching Dusklight link stub. The complete, reproducible commands are in the `Build iOS and tvOS` job in [the build workflow](.github/workflows/build-platforms.yml). That job builds both ARM64 modules, verifies them, and combines them into the separate `Twilight-HD-HUD-iOS-tvOS.dusk` package.

## Credits and licensing

Project code is available under the MIT License.

The optional fonts are distributed under the SIL Open Font License 1.1.
Their copyright notices, licenses, and conversion details are included in
[res/fonts](res/fonts/README.md). They are open-font conversions, not TPHD font dumps.

Twilight Princess, its characters, and its original assets are properties of Nintendo. This is an unofficial fan project and is not affiliated with or endorsed by Nintendo or the Dusklight developers.
