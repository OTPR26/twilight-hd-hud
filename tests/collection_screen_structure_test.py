"""Guard Collection's full-screen backdrop against Android safe-area clipping."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'src/collection_screen.inc').read_text()
helper = source.split('void collection_fill_physical_viewport(', 1)[1].split(
    'void collection_text(', 1)[0]
assert 'mDoGph_gInf_c::getWidthF()' in helper
assert 'mDoGph_gInf_c::getHeightF()' in helper
assert 'mDoGph_gInf_c::getMinXF()' in helper
assert 'mDoGph_gInf_c::getMinYF()' in helper
apply = source.split('void apply_collection_screen(', 1)[1].split(
    'void apply_collection_prompts(', 1)[0]
assert 'collection_fill_physical_viewport(menu, s_collectionScreen.background);' in apply
assert 'getSafeMaxYF() - mDoGph_gInf_c::getSafeMinYF()' not in apply
print('PASS: Collection backdrop fills the physical viewport outside HUD-safe bounds')
