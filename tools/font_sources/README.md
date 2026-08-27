# Open-font sources

Downloaded from the official Google Fonts repository on 2026-08-26:

- https://raw.githubusercontent.com/google/fonts/main/ofl/zenkakugothicnew/ZenKakuGothicNew-Bold.ttf
- https://raw.githubusercontent.com/google/fonts/main/ofl/mplus2/MPLUS2%5Bwght%5D.ttf
- https://raw.githubusercontent.com/google/fonts/main/ofl/firasans/FiraSans-Bold.ttf

The generation script pins each file's SHA-256 and refuses different input.
All fonts are SIL OFL 1.1; the original copyright/license notices are included in
`res/fonts/OFL-Zen-Kaku-Gothic-New.txt`, `res/fonts/OFL-M-PLUS-2.txt`, and
`res/fonts/OFL-Fira-Sans.txt`. The Fira license was downloaded from
https://raw.githubusercontent.com/google/fonts/main/ofl/firasans/OFL.txt
(SHA-256 `8f24842e9174beda18a556c2ae7d54f5dc444340c19a3a9ef77e23bca366adbd`).
Fira uses the same family as Dusklight's general UI text; this pinned Google Fonts
distribution is not byte-identical to the TTF bundled in the tested Dusklight build.
These source TTFs are not included in the runtime mod bundle.

Run `python3 tools/generate_font_atlases.py --preview build/font-preview` using
Python with Pillow/FreeType. It generates and validates the BFN resources and a
manifest recording the source/derived hashes, weight, and raster dimensions.

No proprietary glyphs, original font tables, or game files are used by the generator.
