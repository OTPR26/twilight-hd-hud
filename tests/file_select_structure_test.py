"""Guard the shared play-time anchor and its authored texture coordinates."""
from pathlib import Path
import struct

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
align = source.split('void align_file_select_play_time(', 1)[1].split(
    'void style_file_select_metadata(', 1)[0]
assert 'file_select_layout::play_time_center(top.y, bottom.y)' in align
assert 'manager.getGlobalVtx(row, &matrix, 0, false, 0)' in align
assert 'manager.getGlobalVtx(row, &matrix, 3, false, 0)' in align
assert 'manager.getGlobalVtxCenter(text, false, 0)' in align
assert 'VBIND_CENTER' in align and 'std::isfinite(determinant)' in align
assert 'totalTimeSpacing' not in source
for start, end in (
    ('void style_file_select_metadata(', 'void scale_file_select_button('),
    ('void style_save_menu_metadata(', 'void style_save_select_title('),
):
    body = source.split(start, 1)[1].split(end, 1)[0]
    assert body.count('align_file_select_play_time(row, text)') == 2

for start, end, resize, style in (
    ('HookAction before_file_select_draw(', 'HookAction before_file_select_main_draw(',
     'update_file_select_row_selection(menu)', 'style_file_select_metadata(menu)'),
    ('void after_save_menu_wide(', 'HookAction before_save_dlst_draw(',
     'update_save_menu_row_selection(menu)', 'style_save_menu_metadata(menu)'),
    ('HookAction before_save_dlst_draw(', 'void after_meter_move_button_cross(',
     'update_save_menu_row_selection(s_activeSaveMenu)', 'style_save_menu_metadata(s_activeSaveMenu)'),
):
    body = source.split(start, 1)[1].split(end, 1)[0]
    assert body.index(resize) < body.index(style)

for name in ('file-select-row.bti', 'file-select-row-selected.bti'):
    data = (root / 'res/menu' / name).read_bytes()
    width, height = struct.unpack_from('>HH', data, 2)
    assert data[0] == 6 and (width, height) == (512, 80)

    def pixel(y):
        x = width // 2
        offset = 32 + ((y // 4) * (width // 4) + x // 4) * 64
        index = (y % 4) * 4 + x % 4
        alpha, red = data[offset + 2 * index:offset + 2 * index + 2]
        green, blue = data[offset + 32 + 2 * index:offset + 34 + 2 * index]
        return red, green, blue, alpha

    assert pixel(57)[0] > 200 and pixel(57)[3] > 0  # divider
    assert all(pixel(y)[0] < 140 for y in range(58, 76))  # clear band
    assert pixel(76)[0] > 170 and pixel(76)[3] > 200  # bottom border

print('PASS: shared live play-time alignment, post-resize ordering, and texture anchors')

heading = source.split('void style_save_select_title(', 1)[1].split(
    'void simplify_save_menu_rows(', 1)[0]
assert 'active ? file_select_layout::kSaveQuestionBannerTop : 24.0f' in heading
assert 'active ? file_select_layout::kSaveQuestionBannerBottom : 80.0f' in heading
assert 'constexpr f32 size = file_select_layout::kSaveQuestionFontSize' in heading
assert '(608.0f - totalWidth) * 0.5f' in heading
assert 'left, textTop + opticalDrop' in heading
assert 'textBottom + opticalDrop' in heading
assert 'if (!active)' in heading and 'mHeaderAnmComplete' in heading
assert 'std::strstr(nativeString, "Save to which log?")' in heading
assert 'std::strstr(nativeString, "Overwrite this log?")' in heading
assert 'const bool active = saveQuestion || overwriteQuestion;' in heading
assert 'copy_title_text_width(font, question, size, spacing)' in heading
assert 'overwriteQuestion ? "Overwrite this lo" : "Save to which lo", "g", "?"' in heading
# Reuse the same text panes, so switching/canceling cannot leave duplicate
# headings, different descender offsets or differently sized banners.
assert heading.count('copy_metadata_text(group, tags[index],') == 1
assert source.index('add_save_menu_title_rules(menu);') < source.index(
    'style_save_select_title(menu);', source.index('add_save_menu_title_rules(menu);'))
print('PASS: shared compact Save/Overwrite heading, measured centering and other-message restoration')
