HookAction before_set_select_item_index(ModContext*, void* args, void*, void*) {
    const int index = mods::arg<int>(args, 0);
    const u8 slot = mods::arg<u8>(args, 1);
    if (index != kZItemSlot || !daPy_py_c::checkNowWolf() || !dComIfGp_event_runCheck())
        return HOOK_CONTINUE;
    const u8 previous = dComIfGs_getSelectItemIndex(index);
    // Wolf events reuse the third-slot field for scent, including an empty scent.
    // Keep the human assignment; the collected scent has its own save field.
    const bool scent = slot >= dItemNo_SMELL_YELIA_POUCH_e && slot <= dItemNo_SMELL_MEDICINE_e;
    if ((slot == dItemNo_NONE_e || scent) && previous < 24 &&
        dComIfGs_getItem(previous, false) != dItemNo_NONE_e)
        return HOOK_SKIP_ORIGINAL;
    return HOOK_CONTINUE;
}
