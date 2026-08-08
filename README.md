# Twilight HD HUD

Twilight HD HUD brings a *Twilight Princess HD*-inspired HUD and controller layout to [Dusklight](https://github.com/BeZide93/dusk).

## Features

- Wii U-inspired buttons, D-Pad, minimap, rupee placement
- Three item slots using X, Y, and R
- Independent D-Pad and minimap behavior
- Human and wolf action artwork
- ABXY, BAYX, and letter-free button-label options
- HUD sizes of 75%, 100%, and 125%
- Native modules for Windows x64, Linux x64, macOS Apple Silicon, and Android ARM64

## Installation

1. Download the latest `.dusk` file from [Releases](../../releases/latest).
2. Place it in the `mods` folder inside the active Dusklight data folder.
3. Restart Dusklight or reload mods from its mod manager.
4. Open the Twilight HD HUD settings panel to choose a button-label layout and HUD size.

The package does not contain game files. A compatible, legally obtained disc image is required by Dusklight.


Physical controller bindings still depend on the active Dusklight controller profile.

## Building

The project builds against the Dusklight mod SDK. The included workflow produces native Windows and Linux packages against the compatible Dusklight revision recorded in the workflow file.

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DDUSK_DIR=/path/to/dusklight
cmake --build build
```

The resulting package is written to `build/mods/twilight_hd_hud.dusk`.

## Credits and licensing

Project code is available under the MIT License.

Twilight Princess, its characters, and its original assets are properties of Nintendo. This is an unofficial fan project and is not affiliated with or endorsed by Nintendo or the Dusklight developers.
