from pathlib import Path

source = (Path(__file__).parents[1] / 'src/item_slot_hooks.cpp').read_text()
eligible = source.split('bool item_needs_z_valid_button(int itemNo) {', 1)[1].split('\n}', 1)[0]
for item in ('dItemNo_HVY_BOOTS_e', 'dItemNo_SPINNER_e', 'dItemNo_KANTERA_e'):
    assert f'itemNo == {item}' in eligible

hook = source.split('HookAction before_check_item_set_button(', 1)[1].split(
    'HookAction before_set_heavy_boots(', 1)[0]
assert 'link == nullptr || !item_needs_z_valid_button(itemNo)' in hook
assert '!link->checkGroupItem(itemNo, resolved_select_item(kZItemSlot))' in hook
assert hook.count('return HOOK_CONTINUE;') == 2
# 2 is the native not-found sentinel. Report a valid native button instead.
assert '*static_cast<int*>(retval) = SELECT_ITEM_X;' in hook
assert 'return HOOK_SKIP_ORIGINAL;' in hook
assert 'ADD_PRE(CheckItemSetButtonHook, before_check_item_set_button,' in source
# Only answer the assignment query; keep native put-away, water and oil rules.
for mutation in ('onNoResetFlg', 'offNoResetFlg', 'offKandelaarModel', 'itemEquip(',
                 'deleteEquipItem(', 'swordEquip('):
    assert mutation not in hook

print('PASS: R lantern reports assigned without overriding native extinguishing rules')
