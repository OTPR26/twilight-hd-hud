"""Keep Options confirmation centered and its prompt artwork registered."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
fit = source.split('bool fit_option_warning_frame(', 1)[1].split(
    'void style_option_confirmation(', 1)[0]
assert 'print.parse(' in fit  # Retain native localized measurement/wrapping.
assert 'option_warning_size(' in fit
assert 'width / frameScaleX, height / frameScaleY' in fit
assert 'position_dmap_global_center(frame, centerX, centerY)' in fit
assert 'position_dmap_global_center(text, centerX, centerY)' in fit
assert '(HBIND_CENTER << 2) | VBIND_CENTER' in fit
assert 'text->field_0x110 = 0.0f' in fit
assert 'setString(' not in fit
style = source.split('void style_option_confirmation(', 1)[1].split(
    '// Match the compact gold-edged choices', 1)[0]
assert 'titleGroup->hide()' in style and 'else titleGroup->show()' in style
assert 'warningReady && option_confirmation_active(menu)' in style
active = source.split('bool option_confirmation_active(', 1)[1].split(
    'bool fit_option_warning_frame(', 1)[0]
for state in ('OPEN', 'MOVE', 'SELECT', 'CLOSE'):
    assert f'PROC_CONFIRM_{state}_MOVE_e' in active

# Compare actual authored rectangles, not approximate button placement.
# Options must use the same whole flourish/label/disc geometry as Quest Log
# and Save. The flourish must be drawn before the discs that cover its loops.
def bounds(tag):
    match = re.search(r"MULTI_CHAR\('" + tag + r"'\),\s*"
                      r"JGeometry::TBox2<f32>\(([^)]*)\)", source)
    assert match, tag
    return re.sub(r'\s+', '', match.group(1))

for tags in [('hd_oflr', 'hd_fflr', 'hd_sflr'),
             ('hd_ocfm', 'hd_fcon', 'hd_sconf'),
             ('hd_oapi', 'hd_fapi', 'hd_sapi'),
             ('hd_obck', 'hd_fbck', 'hd_sbck'),
             ('hd_obpi', 'hd_fbpi', 'hd_sbpi')]:
    assert len({bounds(tag) for tag in tags}) == 1, tags
prompts = source.split('void add_option_prompts(', 1)[1].split(
    'bool option_confirmation_active(', 1)[0]
assert prompts.index("addPicture(MULTI_CHAR('hd_oflr')") < prompts.index(
    "addPicture(MULTI_CHAR('hd_oapi')")
assert 's_fileSelectPromptFlourishResource' in prompts
assert 'update_menu_face_button' in prompts
print('PASS: Options confirmation layout, title lifecycle and shared A/B geometry')
