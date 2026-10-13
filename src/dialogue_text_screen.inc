// Draw-only scaling keeps the native parser, line breaks, reveal timing and
// choice layout unchanged. Text, ruby, shadows, character halos and embedded
// symbols share one transform; item cards keep their left text edge fixed.
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
    dMsgScrnBase_c* screen = nullptr;
    std::array<DialoguePaneState, 14> panes{};
    std::array<DialogueGeometry, 150> characters{};
    std::array<DialogueSymbolState, 35> symbols{};
    int characterCount = 0;
};
DialogueDrawState s_dialogueDrawState{};

void scale_dialogue_for_draw(dMsgScrnBase_c* screen) {
    const bool itemCard = dynamic_cast<dMsgScrnItem_c*>(screen) != nullptr;
    if ((!itemCard && dynamic_cast<dMsgScrnTalk_c*>(screen) == nullptr) ||
        screen == nullptr || s_dialogueDrawState.screen != nullptr ||
        screen->mpTm_c[0] == nullptr) return;
    auto* mainText = screen->mpTm_c[0]->getPanePtr();
    if (mainText == nullptr) return;
    int characterCount = 0;
    float sourceMessageScale = 1.0f;
    if (screen->mCharInfoPtr != nullptr) {
        // The first rendered character carries the message's initial/top-level
        // scale. This is 1.0 for ordinary dialogue and larger for a handful of
        // GC-authored character lines. Later inline emphasis is intentionally
        // not used to resize the whole message.
        characterCount = std::clamp<int>(
            *reinterpret_cast<s16*>(screen->mCharInfoPtr + 150), 0, 150);
        if (characterCount > 0 && screen->mCharInfoPtr[0].field_0x08 > 0.0f) {
            sourceMessageScale = screen->mCharInfoPtr[0].field_0x08;
        }
    }
    CPaneMgr manager;
    Vec anchor = manager.getGlobalVtxCenter(mainText, false, 0);
    if (itemCard) {
        Mtx matrix;
        anchor.x = manager.getGlobalVtx(mainText, &matrix, 0, false, 0).x;
    }
    const dialogue_text_layout::Transform transform{anchor.x, anchor.y,
        itemCard ? hud_scales().dialogueText :
            dialogue_text_multiplier(hud_scales().dialogueText, sourceMessageScale)};
    auto& state = s_dialogueDrawState;
    state.screen = screen;
    for (int i = 0; i < 14; ++i) {
        auto* paneManager = i < 7 ? screen->mpTm_c[i] : screen->mpTmr_c[i - 7];
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
    if (screen->mCharInfoPtr != nullptr) {
        state.characterCount = characterCount;
        for (int i = 0; i < state.characterCount; ++i) {
            auto& character = screen->mCharInfoPtr[i];
            state.characters[i] = {character.field_0x00, character.field_0x04,
                character.field_0x08, character.field_0x0c};
            character.field_0x00 = transform.position_x(character.field_0x00);
            character.field_0x04 = transform.position_y(character.field_0x04);
            character.field_0x08 = transform.size(character.field_0x08);
            character.field_0x0c = transform.size(character.field_0x0c);
        }
    }
    const float symbolOffsetX = itemCard ? g_MsgObject_HIO_c.mBoxItemTextPosX - 2.0f : 0.0f;
    const float symbolOffsetY = itemCard ? g_MsgObject_HIO_c.mBoxItemTextPosY : 0.0f;
    if (screen->mpOutFont != nullptr) {
        for (std::size_t i = 0; i < state.symbols.size(); ++i) {
            auto* symbol = screen->mpOutFont->mpOfs[i];
            // Dialogue's symbols are absolute draws (null textbox). Do not
            // touch textbox-owned menu or choice symbols sharing this renderer.
            if (symbol == nullptr || symbol->getType() == 0x47 ||
                symbol->getTextBoxPtr() != nullptr) continue;
            state.symbols[i] = {symbol,
                {symbol->mPosX, symbol->mPosY, symbol->mSizeX, symbol->mSizeY}};
            symbol->mPosX = transform.position_x(symbol->mPosX + symbolOffsetX) - symbolOffsetX;
            symbol->mPosY = transform.position_y(symbol->mPosY + symbolOffsetY) - symbolOffsetY;
            symbol->mSizeX = transform.size(symbol->mSizeX);
            symbol->mSizeY = transform.size(symbol->mSizeY);
        }
    }
}

void after_message_screen_draw(ModContext*, void* args, void*, void*) {
    if (dynamic_cast<dMsgScrnItem_c*>(mods::arg<dMsgScrnBase_c*>(args, 0)) != nullptr) {
        end_item_prompt_font();
    }
    auto& state = s_dialogueDrawState;
    if (state.screen == nullptr || state.screen != mods::arg<dMsgScrnBase_c*>(args, 0)) {
        restore_item_get_assignment_icons();
        return;
    }
    for (const auto& saved : state.panes) {
        if (saved.pane == nullptr) continue;
        saved.pane->scale(saved.scaleX, saved.scaleY);
        saved.pane->translate(saved.x, saved.y);
    }
    for (int i = 0; i < state.characterCount; ++i) {
        auto& character = state.screen->mCharInfoPtr[i];
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
    restore_item_get_assignment_icons();
}
