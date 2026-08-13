# Twilight HD HUD

Twilight HD HUD brings a *Twilight Princess HD*-inspired HUD and controller layout to [Dusklight](https://github.com/TwilitRealm/dusklight).

## Compatibility

Dusklight **v1.4.1-129 or newer** is required. The mod is built and tested against the official Dusklight repository.

## Features

- Wii U-inspired buttons, D-Pad, minimap, rupee placement
- Three item slots using X, Y, and R
- Independent D-Pad and minimap behavior
- Human and wolf action artwork
- ABXY, BAYX, and letter-free button-label options
- HUD sizes of 75%, 100%, and 125%
- Native modules for Windows x64, Linux x64, macOS Apple Silicon, Android ARM64, iOS ARM64, and tvOS ARM64

Planned improvements are tracked in [ROADMAP.md](ROADMAP.md).
Release history is recorded in [CHANGELOG.md](CHANGELOG.md).

## Downloads

The release provides two `.dusk` packages:

- **Desktop and Android:** Windows, Linux, Steam Deck, macOS Apple Silicon, and Android ARM64
- **iOS and tvOS:** iPhone, iPad, and Apple TV

Download the package for the device running Dusklight. The packages contain the same mod assets and settings; only their native modules differ.

## Installation: Windows, Linux, macOS, and Android

1. Download the latest `.dusk` file from [Releases](../../releases/latest).
2. Place it in the `mods` folder inside the active Dusklight data folder.
3. Restart Dusklight or reload mods from its mod manager.
4. Open the Twilight HD HUD settings panel to choose a button-label layout and HUD size.

On macOS, the active data folder is normally under `~/Library/Application Support/Dusklight`. On Android, use the `mods` folder inside the Dusklight data folder selected by the app.

## Installation: iPhone and iPad

1. Download the **iOS and tvOS** `.dusk` file from [Releases](../../releases/latest).
2. Launch Dusklight once so its data folders are created.
3. Open the Files app and browse to **On My iPhone** or **On My iPad > Dusklight > mods**.
4. Copy the downloaded `.dusk` file into `mods`.
5. Return to Dusklight, then restart the app or reload mods from the mod manager.
6. Enable Twilight HD HUD and open its settings panel to choose a button-label layout and HUD size.

If the Dusklight folder is not visible in Files, use the app's file-transfer page and upload the `.dusk` file to **Mods** instead.

## Installation: Apple TV

1. Download the **iOS and tvOS** `.dusk` file on a phone or computer connected to the same network as the Apple TV.
2. In Dusklight on Apple TV, open the file-transfer page and turn on local-network transfer.
3. Enter the complete address shown on the television in a web browser, including the colon before the port number.
4. Under **Mods**, choose the downloaded `.dusk` file and upload it.
5. On Apple TV, rescan or reload mods, then enable Twilight HD HUD.
6. Restart Dusklight if the mod does not appear immediately.

The transfer page is available only while Dusklight is open, and both devices must remain on the same local network.

The package does not contain game files. A compatible, legally obtained disc image is required by Dusklight.

## Controls

- X, Y, and R: equipped items
- D-Pad Up: Midna
- D-Pad Down: Items
- D-Pad Left/Right: minimap toggle
- ZL: targeting and defending
- A: context action
- B: attack
- Right stick: free camera

Physical controller bindings still depend on the active Dusklight controller profile.

The default **Follow Dusklight Bindings** controller mode leaves that profile unchanged and honors
the configured **Call Midna** action without also triggering the control's normal game action. The
optional **TPHD Fixed Bindings** mode ignores the separate Call Midna binding and keeps Midna
exclusively on D-Pad Up. In both modes, the control mapped to logical R operates the third item and
related menu actions without rewriting the saved controller profile.

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

### iOS and tvOS builds

Apple mobile targets require the iOS or tvOS CMake toolchain and the matching Dusklight link stub. The complete, reproducible commands are in the `Build iOS and tvOS` job in [the build workflow](.github/workflows/build-platforms.yml). That job builds both ARM64 modules, verifies them, and combines them into the separate `Twilight-HD-HUD-iOS-tvOS.dusk` package.

## Credits and licensing

Project code is available under the MIT License.

Twilight Princess, its characters, and its original assets are properties of Nintendo. This is an unofficial fan project and is not affiliated with or endorsed by Nintendo or the Dusklight developers.
