"""Guard the dialogue-only draw scope and native layout restoration."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
source = (root / 'src/dialogue_text_screen.inc').read_text()
hooks = (root / 'src/item_slot_hooks.cpp').read_text()
assert 'dynamic_cast<dMsgScrnTalk_c*>(screen)' in source
assert 'talk->mpTm_c[i] : talk->mpTmr_c[i - 7]' in source
assert 'dialogue_text_multiplier(hud_scales().dialogueText, sourceMessageScale)' in source
assert 'talk->mCharInfoPtr[0].field_0x08' in source
assert 'symbol->getTextBoxPtr() != nullptr' in source
assert 'characterCount = std::clamp<int>' in source
assert 'state.characterCount = characterCount' in source
assert 'saved.pane->scale(saved.scaleX, saved.scaleY)' in source
assert 'saved.pane->translate(saved.x, saved.y)' in source
for field in ('field_0x00', 'field_0x04', 'field_0x08', 'field_0x0c'):
    assert f'character.{field} = ' in source
assert 'character.field_0x14 =' not in source  # never roll back halo animation
assert 'setFontSize(' not in source and 'setString(' not in source
assert 'mpSelect_c' not in source and 'mpPmP_c' not in source
assert 'ADD_POST(MessageScreenDrawHook, after_message_screen_draw' in hooks
assert 'scale_dialogue_for_draw(messageScreen)' in hooks
print('PASS: dialogue-only text/ruby/glow/symbol scaling, geometry restored, choices/parser unchanged')
