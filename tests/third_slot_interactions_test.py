"""Guard the scope and ownership of Z-slot trade and bait compatibility."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / "src/item_slot_hooks.cpp").read_text()
def section(start, end):
    return source.split(start, 1)[1].split(end, 1)[0]

lookup = section("HookAction before_get_select_item(", "void after_set_select_item(")
assert "s_baitLookupScope && index == kBaitRodAlias" in lookup
assert "s_thirdSlotTalkRead && index == SELECT_ITEM_X" in lookup
talk = section("HookAction before_order_talk(", "void after_player_execute(")
assert "link->checkWolf()" in talk and "link->notTalk()" in talk
assert "link->checkRequestTalkActor" in talk
assert "order == s_thirdSlotTalk.order" in talk
assert "order->mpRequestActor == s_thirdSlotTalk.player" in talk
assert "order->mpTargetActor == s_thirdSlotTalk.target" in talk
assert "void after_talk_queue_entry" in talk
assert "setSelectItemIndex" not in talk
bait = section("HookAction before_fishing_food_init(", "HookAction before_order_talk(")
assert "find_select_button(link, 0x108) != kZItemSlot" in bait
assert "link->mProcVar3.field_0x300e = link->mSelectItemId" in bait
assert bait.count("link->mSelectItemId = kZItemSlot") == 2
assert "kBaitRodAlias" not in bait
overlay = section("void apply_context_button_layout(", "void hide_ring_stock_z_prompt(")
assert "MULTI_CHAR('b_btn')" in overlay
assert "menu_face_button_texture(false)" in overlay
registration = source.split('if (feature_enabled(Feature::ThirdItemSlot)) {')[-1]
for hook in ("CheckNewItemChangeHook", "FishingFoodInitHook", "OrderTalkHook",
             "TalkItemCheckHook", "TalkQueueEntryHook"):
    assert hook in registration
print("PASS: scoped third-slot trade/bait handling and styled contextual B prompt")
