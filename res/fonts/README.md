# Text font resources

These BFN atlases are rasterized format conversions of open fonts, not TPHD font dumps.
They remain licensed under the SIL Open Font License 1.1, separately from the mod's code.
The source copyright notices and full licenses accompany these files.

- `zen-bold.bfn`: derived from Zen Kaku Gothic New Bold.
- `mplus-bold.bfn`: derived from M PLUS 2 at weight 700.
- `fira-bold.bfn`: derived from Fira Sans Bold, the family used by Dusklight's general UI.

2026-08-26 conversion changes: Latin/Western CP1252 subset; 128-pixel padded cells;
cap-height normalization; 4-bit antialiasing; single 2048×2048 GX I4 texture;
GameCube BFN container and identity code mapping. See `font-atlases.json` for hashes.
These are modified font resources, not the canonical upstream font distributions.

The mod replaces standard message-font drawing only. Original glyph advances,
line-break calculations, and original font ownership are unchanged. Decorative
fonts and unsupported glyphs continue to use the original game assets.
