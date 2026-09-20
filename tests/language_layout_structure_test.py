"""Regression coverage for localized prompts and letter-matched Xbox HUDs."""
from source_helpers import read_hook_source

source = read_hook_source()
prompt = source.split('J2DPane* make_hd_prompt_label(', 1)[1].split('// Shared three-choice prompt', 1)[0]
assert 'MenuLabel::Confirm : MenuLabel::Back' in prompt
assert 'JKR_NEW J2DTextBox' in prompt and 'fit_localized_label(label' in prompt
assert 's_promptConfirmLabelResource' in prompt  # English artwork stays available.
assert 'single_line_label(item_bank_source_text' in source
assert 'fit_localized_label(s_itemBank.assignment' in source
assert 'hide_other_text(footer, label)' in source
assert 'saveLabel->setString(0x20, "Save Game")' not in source
assert 'collection_shortcut_label(localized_label(MenuLabel::Collection)' in source
assert 'localized_label(MenuLabel::Minimap)' in source

flipped = source.split('if (layout == ButtonLayout::BayxFlipped)', 1)[1].split('} else if', 1)[0]
assert "MULTI_CHAR('x_btn'), styled_face_button_texture('X')" in flipped
assert "MULTI_CHAR('y_btn'), styled_face_button_texture('Y')" in flipped
positions = source.split('void apply_flipped_diamond_positions(', 1)[1].split('void apply_button_layout_preference(', 1)[0]
for slot in (0, 1):
    for part in ('mpButtonXY', 'mpTextXY', 'mpItemXY', 'mpLightXY', 'mpBTextXY'):
        assert f'pane_ptr(meter->{part}[{slot}])' in positions
assert 'y.x - x.x, y.y - x.y' in positions
assert 'x.x - y.x, x.y - y.y' in positions
assert 'restore_archive_pane(meter->mpBTextXY[i])' in source
print('PASS: native localized labels, single-row assignment, footer layers, and full X/Y HUD groups')
