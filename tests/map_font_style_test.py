"""Map typography remains isolated and preserves native encoding/advances."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
font = (root / 'src/font_override.cpp').read_text()
screen = (root / 'src/overworld_map_screen.inc').read_text()
assert 's_mapDepth > 0 && source && source == mDoExt_getRubyFont()' in font
assert 'fonts/mplus-bold.bfn' in font
assert 'drawOriginal(source, x' not in font
assert 'mapStem ? 1.0f : itemStem ? 0.5f : 0.0f' in font
assert 'int(black.b) + 22' not in screen
assert 'fonts/fira-regular.bfn' in font
assert 'begin_map_font();' in screen and 'end_map_font();' in screen
assert 'title->setFont(mDoExt_getRubyFont())' in screen
assert 'area->setFont(areaSource->getFont())' in screen
assert 'picture == map->mPictures[ICON_LINK_e]' in screen
assert 'picture->scale(s_fmapPicture.scaleX, s_fmapPicture.scaleY)' in screen
assert 'picture->setBlackWhite(s_fmapPicture.black, s_fmapPicture.white)' in screen
assert 'title->setFontSize(22 * size, 22 * size)' in screen
assert 'set_text_binding(title, HBIND_CENTER, VBIND_CENTER)' in screen
assert 'bannerY + bannerTitleTop * size' in screen
assert 'bannerTitleHeight * size' in screen
assert 'text->setCharSpace(-0.6f * size)' in screen
assert 'cursor != map->mpDraw2DBack->mpPortalIcon' in screen
assert 'picture->setAlpha(cursor->mpPaneMgr->getPanePtr()->getAlpha())' in screen
assert 'picture->move(cursor->mPositionX - width * .5f' in screen
assert 'cursor->mpScreen->appendChild(picture)' in screen
portal = (root / 'res/menu/overworld-portal.bti').read_bytes()
assert portal[0] == 6 and len(portal) == 32 + 128 * 128 * 4
print('PASS: scoped map fonts, native encoding and field-map-only marker sizing')
