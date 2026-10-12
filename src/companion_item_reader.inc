int s_companionItemInfoDepth = 0;

HookAction before_companion_item_info(ModContext*, void*, void*, void*) {
    if (s_dualScreenHost.active()) ++s_companionItemInfoDepth;
    return HOOK_CONTINUE;
}

void after_companion_item_info(ModContext*, void*, void*, void*) {
    if (s_companionItemInfoDepth > 0) --s_companionItemInfoDepth;
}

void after_companion_item_text(ModContext*, void* args, void*, void*) {
    if (s_companionItemInfoDepth == 0) return;
    const u32 id = mods::arg<u32>(args, 1);
    auto* text = mods::arg<char*>(args, 2);
    const int capacity = mods::arg<int>(args, 3);
    if (id < 0x265 || id > 0x364 || text == nullptr || capacity <= 0) return;
    const auto* end = static_cast<const char*>(std::memchr(text, '\0', capacity));
    if (end == nullptr) return;
    const auto replacement = companion_item_help({text, static_cast<std::size_t>(end - text)});
    if (replacement.empty() || replacement.size() >= static_cast<std::size_t>(capacity)) return;
    std::memcpy(text, replacement.c_str(), replacement.size() + 1);
}
