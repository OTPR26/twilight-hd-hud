# Twilight HD

Twilight HD brings a *Twilight Princess HD*-inspired HUD and controller layout to [Dusklight](https://github.com/TwilitRealm/dusklight).

## Compatibility

Dusklight 2.0 or newer is required. The mod is built and tested against the official Dusklight repository.

Twilight HD can coexist with **Dawnlight**. Twilight HD owns
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
- TPHD-inspired world-map colors, portal icons, and location labels; toggle
  Portals with L / Xbox LB / PlayStation L1. Minimap visibility is retained after warping.
- TPHD-sized action and dialogue text, with the bottom-center A prompt on the left
- Centered oil, oxygen, and lantern meters across supported display sizes
- Native modules for Windows x64 and ARM64, Linux x64 and ARM64, macOS Intel and Apple Silicon, Android ARM64, iOS ARM64, and tvOS ARM64

Planned improvements are tracked in [ROADMAP.md](ROADMAP.md).
Release history is recorded in [CHANGELOG.md](CHANGELOG.md).

## Downloads

Download from [Releases](../../releases/latest):

- **Twilight-HD-HUD.dusk:** Windows x64/ARM64, Linux x64/ARM64, macOS Intel/Apple Silicon, Android ARM64, and iOS ARM64. Use this eight-platform package for the mod site.
- **Twilight-HD-HUD-tvOS.dusk:** Apple TV only.

The older Desktop-Android filename is a copy of the eight-platform package. The iOS-tvOS filename is a combined Apple-mobile compatibility package. Both are retained for existing update checks, not mod-site uploads. Install only one package. Apple mobile devices still require the bundling and signing steps below.

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

### Optional features

Twilight HD Settings includes three independent switches, all on by default:

- **Third Item Slot (Z/R):** turn off the extra item slot, its assignment prompts,
  and Twilight HD's shoulder/Midna input handling.
- **TPHD Collection Screen:** turn off the main Collection layout, model placement,
  and navigation changes. Journal improvements remain enabled.
- **D-Pad Shortcuts:** turn off Twilight HD's map/minimap and Items/Collection
  shortcuts, D-Pad HUD adjustments, and D-Pad map-back hints. The Items / Collection
  Buttons setting has no effect while this switch is off.

Restart Dusklight after changing these switches. Turning a feature off leaves its
controls and runtime hooks to Dusklight or other mods; it does not disable the
rest of Twilight HD's shared artwork and styling. Compatibility with another
mod still depends on which remaining features it changes.

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

## Installation: iPhone, iPad, and Apple TV

Twilight HD includes native executable code. On standard iOS and tvOS devices,
it must be bundled and signed with Dusklight. Use `Twilight-HD-HUD.dusk` for iPhone/iPad or `Twilight-HD-HUD-tvOS.dusk` for Apple TV. The files
in [Releases](../../releases/latest) is for that packaging workflow; copying an
unsigned package into the writable `mods` folder is not sufficient.

Install a signed Dusklight app build containing the desired Twilight HD version,
then enable the mod and choose your button-label layout and HUD size.

Updates are managed through Dusklight's built-in Online mods interface.
Twilight HD no longer performs its own GitHub checks or displays separate update controls.
On iOS and tvOS, native-code updates still require an updated signed Dusklight app
containing the newer mod; the catalog cannot install native code on those platforms.

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

All **BAYX** layouts use Xbox shoulder/trigger prompts: LB, RB, LT, and RT,
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

**ABXY (BOTW Style)**, **BAYX (BOTW Style)**, and **Universal (BOTW Style)**
place Attack on West, Action on South, and the two face-button items on North
and East. These are visual presets, not input remaps. Configure the matching
bindings in Dusklight: native B/Attack to West, native A/Action to South,
native X/item to North, and native Y/item to East. L, R, triggers, and D-Pad
bindings are unchanged. Menu, dialogue, and item-assignment prompts show the
same selected face-button labels; actual menu behavior follows Dusklight's
bindings. The mod does not detect or rewrite arbitrary custom bindings.

The **Button Layout** setting includes a PlayStation option that displays Circle for the east
action, Cross for the south attack, Triangle and Square for the two face-button items, R1 for the
third item, L2 for enemy lock-on, and R2 for Gale Boomerang multi-target lock.

The **Button Style** setting switches between the standard silver TPHD prompts and dark graphite
**Black Pro** prompts. Button Style is independent of the ABXY, BAYX, Universal, and PlayStation
layouts, including BAYX Flipped and the BOTW Style presets. Both Universal
layouts additionally offer **Transparent**, preserving their former blank
background appearance. Universal **Silver** now uses a letterless version of
the standard silver button artwork. Transparent is unavailable for lettered
layouts; switching to one returns that style to Silver.

The **Shoulder & D-Pad Behavior** setting controls Midna, shoulder/trigger handling,
and related D-Pad shortcuts. Face-button bindings are always configured in Dusklight.
The default **Follow Dusklight Bindings** mode leaves that profile unchanged and honors
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

The included workflow builds and verifies Windows x64 and ARM64, Linux x64 and ARM64, macOS Intel and Apple Silicon, Android ARM64, iOS ARM64, and tvOS ARM64 against the compatible Dusklight revision recorded in the workflow file. Eight native modules are combined into `Twilight-HD-HUD.dusk` for the mod site. Apple TV is packaged separately as `Twilight-HD-HUD-tvOS.dusk` because the site does not accept tvOS.

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

Apple mobile targets require the iOS or tvOS CMake toolchain and the matching Dusklight link stub. The complete, reproducible commands are in the `Build iOS and tvOS` job in [the build workflow](.github/workflows/build-platforms.yml). That job builds and verifies both ARM64 modules before the eight-platform and separate tvOS packages are assembled.

## Credits and licensing

Project code is available under the MIT License.

The optional fonts are distributed under the SIL Open Font License 1.1.
Their copyright notices, licenses, and conversion details are included in
[res/fonts](res/fonts/README.md). They are open-font conversions, not TPHD font dumps.

Twilight Princess, its characters, and its original assets are properties of Nintendo. This is an unofficial fan project and is not affiliated with or endorsed by Nintendo or the Dusklight developers.
