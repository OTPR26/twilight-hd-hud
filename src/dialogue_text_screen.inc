// Draw-only scaling keeps the native parser, line breaks, reveal timing and
// choice layout unchanged. Text, ruby, shadows, character halos and embedded
// symbols all use the same transform around the native text box center.
struct DialoguePaneState {
    J2DPane* pane = nullptr;
    float x = 0, y = 0, scaleX = 1, scaleY = 1;
};
struct DialogueGeometry {
    float x = 0, y = 0, width = 0, height = 0;
};
struct DialogueSymbolState {
    COutFontSet_c* symbol = nullptr;
    DialogueGeometry geometry{};
};
struct DialogueDrawState {
    dMsgScrnTalk_c* talk = nullptr;
    std::array<DialoguePaneState, 14> panes{};
    std::array<DialogueGeometry, 150> characters{};
    std::array<DialogueSymbolState, 35> symbols{};
    int characterCount = 0;
};
DialogueDrawState s_dialogueDrawState{};

void scale_dialogue_for_draw(dMsgScrnBase_c* screen) {
    auto* talk = dynamic_cast<dMsgScrnTalk_c*>(screen);
    if (talk == nullptr || s_dialogueDrawState.talk != nullptr ||
        talk->mpTm_c[0] == nullptr) return;
    auto* mainText = talk->mpTm_c[0]->getPanePtr();
    if (mainText == nullptr) return;
    int characterCount = 0;
    float sourceMessageScale = 1.0f;
    if (talk->mCharInfoPtr != nullptr) {
        // The first rendered character carries the message's initial/top-level
        // scale. This is 1.0 for ordinary dialogue and larger for a handful of
        // GC-authored character lines. Later inline emphasis is intentionally
        // not used to resize the whole message.
        characterCount = std::clamp<int>(
            *reinterpret_cast<s16*>(talk->mCharInfoPtr + 150), 0, 150);
        if (characterCount > 0 && talk->mCharInfoPtr[0].field_0x08 > 0.0f) {
            sourceMessageScale = talk->mCharInfoPtr[0].field_0x08;
        }
    }
    CPaneMgr manager;
    const Vec anchor = manager.getGlobalVtxCenter(mainText, false, 0);
    const dialogue_text_layout::Transform transform{anchor.x, anchor.y,
        dialogue_text_multiplier(hud_scales().dialogueText, sourceMessageScale)};
    auto& state = s_dialogueDrawState;
    state.talk = talk;
    for (int i = 0; i < 14; ++i) {
        auto* paneManager = i < 7 ? talk->mpTm_c[i] : talk->mpTmr_c[i - 7];
        if (paneManager == nullptr) continue;
        auto* pane = paneManager->getPanePtr();
        if (pane == nullptr) continue;
        const Vec center = manager.getGlobalVtxCenter(pane, false, 0);
        auto& saved = state.panes[i];
        saved = {pane, pane->getTranslateX(), pane->getTranslateY(),
            pane->getScaleX(), pane->getScaleY()};
        pane->scale(transform.size(saved.scaleX), transform.size(saved.scaleY));
        const Vec resized = manager.getGlobalVtxCenter(pane, false, 0);
        shift_action_pane(pane, transform.position_x(center.x) - resized.x,
            transform.position_y(center.y) - resized.y);
    }
    if (talk->mCharInfoPtr != nullptr) {
        state.characterCount = characterCount;
        for (int i = 0; i < state.characterCount; ++i) {
            auto& character = talk->mCharInfoPtr[i];
            state.characters[i] = {character.field_0x00, character.field_0x04,
                character.field_0x08, character.field_0x0c};
            character.field_0x00 = transform.position_x(character.field_0x00);
            character.field_0x04 = transform.position_y(character.field_0x04);
            character.field_0x08 = transform.size(character.field_0x08);
            character.field_0x0c = transform.size(character.field_0x0c);
        }
    }
    if (talk->mpOutFont != nullptr) {
        for (std::size_t i = 0; i < state.symbols.size(); ++i) {
            auto* symbol = talk->mpOutFont->mpOfs[i];
            // Dialogue's symbols are absolute draws (null textbox). Do not
            // touch textbox-owned menu or choice symbols sharing this renderer.
            if (symbol == nullptr || symbol->getType() == 0x47 ||
                symbol->getTextBoxPtr() != nullptr) continue;
            state.symbols[i] = {symbol,
                {symbol->mPosX, symbol->mPosY, symbol->mSizeX, symbol->mSizeY}};
            symbol->mPosX = transform.position_x(symbol->mPosX);
            symbol->mPosY = transform.position_y(symbol->mPosY);
            symbol->mSizeX = transform.size(symbol->mSizeX);
            symbol->mSizeY = transform.size(symbol->mSizeY);
        }
    }
}

void after_message_screen_draw(ModContext*, void* args, void*, void*) {
    if (dynamic_cast<dMsgScrnItem_c*>(mods::arg<dMsgScrnBase_c*>(args, 0)) != nullptr) {
        end_item_prompt_font();
    }
    restore_item_get_assignment_icons();
    auto& state = s_dialogueDrawState;
    if (state.talk == nullptr || state.talk != mods::arg<dMsgScrnBase_c*>(args, 0)) return;
    for (const auto& saved : state.panes) {
        if (saved.pane == nullptr) continue;
        saved.pane->scale(saved.scaleX, saved.scaleY);
        saved.pane->translate(saved.x, saved.y);
    }
    for (int i = 0; i < state.characterCount; ++i) {
        auto& character = state.talk->mCharInfoPtr[i];
        const auto& saved = state.characters[i];
        character.field_0x00 = saved.x;
        character.field_0x04 = saved.y;
        character.field_0x08 = saved.width;
        character.field_0x0c = saved.height;
        // Preserve animation timers/colours advanced by the native draw.
    }
    for (const auto& saved : state.symbols) {
        if (saved.symbol == nullptr) continue;
        saved.symbol->mPosX = saved.geometry.x;
        saved.symbol->mPosY = saved.geometry.y;
        saved.symbol->mSizeX = saved.geometry.width;
        saved.symbol->mSizeY = saved.geometry.height;
    }
    state = {};
}
