from source_helpers import read_hook_source
"""Keep Options confirmation centered and its prompt artwork registered."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
source = read_hook_source()
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
# Warning animation state is region-neutral; process indices differ when
# the Japanese-only Ruby option is omitted.
assert 'menu->mpWarning != nullptr' in active
assert 'menu->mpWarning->field_0x28 > 2849.0f' in active
assert 'menu->mpWarning->field_0x2c > 2849' in active
assert 'PROC_CONFIRM_' not in active

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
assert prompts.count("position_menu_prompt_group(menu->mpScreenIcon, MULTI_CHAR('hd_oprm'))") == 2

def rectangle(tag):
    return tuple(608.0 - float(v[6:].removesuffix('f')) if v.startswith('right-')
                 else float(v.removesuffix('f')) for v in bounds(tag).split(','))

for label, button in [('hd_ocfm', 'hd_oapi'), ('hd_obck', 'hd_obpi'),
                      ('hd_fcon', 'hd_fapi'), ('hd_fbck', 'hd_fbpi'),
                      ('hd_sconf', 'hd_sapi'), ('hd_sbck', 'hd_sbpi'),
                      ('hd_gbat', 'hd_gbab'), ('hd_gbbt', 'hd_gbbb'),
                      ('hd_fjbt', 'hd_fjbb'), ('hd_ltbt', 'hd_ltbb'),
                      ('hd_hsbt', 'hd_hsbb')]:
    text_rect, button_rect = rectangle(label), rectangle(button)
    assert button_rect[0] - text_rect[2] == 4, label
    assert text_rect[1] + text_rect[3] == button_rect[1] + button_rect[3], label

save_position = source.split('void position_save_menu_prompts(', 1)[1].split(
    'void add_save_menu_fixed_prompts(', 1)[0]
assert "MULTI_CHAR('hd_sprm')" in save_position
assert 'position_menu_prompt_group(' in save_position
group_position = source.split('void position_menu_prompt_group(', 1)[1].split(
    '// File Selection and copy menus', 1)[0]
assert 'prompt_group_layout(' in group_position
assert 'group->scale(layout.scaleX, 1.0f)' in group_position
assert 'position_dmap_global_center(group, layout.centerX, layout.centerY)' in group_position
file_prompts = source.split('void add_file_select_fixed_prompts(', 1)[1].split(
    'void ', 1)[0]
assert "position_menu_prompt_group(screen, MULTI_CHAR('hd_fprm'))" in file_prompts
assert 'mpABtnIcon' not in save_position and 'mpBBtnIcon' not in save_position
save_wide = source.split('void after_save_menu_wide(', 1)[1].split(
    'HookAction ', 1)[0]
assert 'position_save_menu_prompts(menu)' in save_wide
journal_fit = source.split('void fit_collection_submenu_overlay(', 1)[1].split(
    'JGeometry::TBox2<f32> collection_submenu_global_bounds', 1)[0]
assert 'group->scale(viewport.scale / parentScaleX,' in journal_fit
assert 'viewport.scale / parentScaleY)' in journal_fit
print('PASS: Options confirmation layout, title lifecycle and shared A/B geometry')
