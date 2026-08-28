// Presentation and navigation only: native inventory indices, assignment,
// combinations, item explanations and assignment completion remain unchanged.
struct ItemBankScreen {
    dMenu_Ring_c* ring = nullptr;
    J2DScreen* screen = nullptr;
    J2DPane* root = nullptr;
    J2DTextBox* name = nullptr;
    J2DTextBox* assignment = nullptr;
    J2DTextBox* combo = nullptr;
    std::array<J2DPicture*, 3> buttons{};
    J2DPicture* comboButton = nullptr;
    std::array<J2DPicture*, 24> frames{};
    bool singleClawshot = false;
};
ItemBankScreen s_itemBank;

bool uses_item_bank(dMenu_Ring_c* ring) {
    return ring != nullptr && s_itemBank.ring == ring && s_itemBank.screen != nullptr;
}

void destroy_item_bank() {
    JKR_DELETE(s_itemBank.screen);
    s_itemBank = {};
}

CollectionViewport item_bank_content_viewport() {
    // Android transient system bars can change the safe-area inset while the
    // game surface stays full-size. Items is a full-screen overlay: fit the
    // rendered surface, not those temporary insets. Keep Collection unchanged.
    return collection_viewport(mDoGph_gInf_c::getMinXF(), mDoGph_gInf_c::getMinYF(),
        mDoGph_gInf_c::getMaxXF(), mDoGph_gInf_c::getMaxYF());
}

CollectionViewport item_bank_viewport(dMenu_Ring_c* ring) {
    auto viewport = item_bank_content_viewport();
    viewport.left += ring->mCenterPosX;
    viewport.top += ring->mCenterPosY;
    return viewport;
}

int item_bank_cell(dMenu_Ring_c* ring, unsigned index) {
    if (index >= ring->mItemsTotal || index >= MAX_ITEM_SLOTS) return -1;
    return bank_cell(ring->mItemSlots[index], s_itemBank.singleClawshot);
}

void update_item_bank_positions(dMenu_Ring_c* ring) {
    if (!uses_item_bank(ring)) return;
    const auto viewport = item_bank_content_viewport();
    for (unsigned i = 0; i < ring->mItemsTotal && i < MAX_ITEM_SLOTS; ++i) {
        const int cell = item_bank_cell(ring, i);
        if (cell < 0) continue;
        ring->mItemSlotPosX[i] = viewport.left + bank_x(cell) * viewport.scale;
        ring->mItemSlotPosY[i] = viewport.top + bank_y(cell) * viewport.scale;
    }
}

J2DTextBox* item_bank_text(float x, float y, float width, float height, float size,
    J2DTextBoxHBinding binding = HBIND_CENTER) {
    auto* text = JKR_NEW J2DTextBox(0,
        JGeometry::TBox2<f32>(x, y, x + width, y + height),
        nullptr, "", 256, binding, VBIND_CENTER);
    text->setFont(mDoExt_getMesgFont());
    text->setFontSize(size, size);
    text->setCharSpace(0);
    text->setFontColor(JUtility::TColor(238, 238, 222, 255),
        JUtility::TColor(255, 255, 246, 255));
    s_itemBank.root->appendChild(text);
    return text;
}

J2DPicture* item_bank_picture(const ResTIMG* texture,
    float x, float y, float width, float height) {
    if (texture == nullptr) return nullptr;
    auto* picture = JKR_NEW J2DPicture(0,
        JGeometry::TBox2<f32>(x, y, x + width, y + height), texture, nullptr);
    configure_hd_picture(picture);
    s_itemBank.root->appendChild(picture);
    return picture;
}

void copy_item_bank_text(J2DTextBox* target, const char* value) {
    if (target == nullptr) return;
    if (value == nullptr) value = "";
    if (std::strcmp(text_box_string(target), value) != 0) target->setString(256, value);
}

const char* item_bank_source_text(J2DScreen* screen, u64 western, u64 japanese) {
    if (screen == nullptr) return "";
    for (const auto tag : {western, japanese}) {
        auto* source = static_cast<J2DTextBox*>(screen->search(tag));
        if (source != nullptr && source->isVisible()) return text_box_string(source);
    }
    return "";
}

void create_item_bank(dMenu_Ring_c* ring) {
    destroy_item_bank();
    if (!item_bank_enabled() || ring == nullptr || ring->mpScreen == nullptr ||
        ring->mpCenterScreen == nullptr || ring->mpDrawCursor == nullptr ||
        ring->mpItemExplain == nullptr) return;
    const auto* frame = resource_texture(s_collectEquipmentFrameResource);
    const auto* banner = resource_texture(s_collectBannerResource);
    const auto* cellBacking = resource_texture(s_itemBankCellResource);
    const auto* circle = resource_texture(s_itemBankCircleResource);
    const auto* shadow = resource_texture(s_itemBankShadowResource);
    const auto* fill = static_cast<const ResTIMG*>(
        dComIfGp_getMain2DArchive()->getResource('TIMG', "tt_block8x8.bti"));
    // Leave the wheel intact if a required asset is unavailable.
    if (frame == nullptr || banner == nullptr || fill == nullptr || cellBacking == nullptr ||
        circle == nullptr || shadow == nullptr) return;
    s_itemBank.ring = ring;
    s_itemBank.screen = JKR_NEW J2DScreen();
    // Unloaded J2DScreens default to an opaque white background. Only the
    // separate dimmer should cover gameplay behind this menu.
    s_itemBank.screen->mColor = JUtility::TColor(0, 0, 0, 0);
    s_itemBank.root = JKR_NEW J2DPane(MULTI_CHAR('hd_ibank'),
        JGeometry::TBox2<f32>(0, 0, 796, 448));
    s_itemBank.screen->appendChild(s_itemBank.root);
    for (unsigned i = 0; i < ring->mItemsTotal && i < MAX_ITEM_SLOTS; ++i)
        s_itemBank.singleClawshot |= ring->mItemSlots[i] == 9;

    // Group equipment, bombs, and bottles without drawing a wheel behind them.
    for (const auto& bounds : {
        JGeometry::TBox2<f32>(119, 164, 425, 385),
        JGeometry::TBox2<f32>(455, 164, 509, 385),
        JGeometry::TBox2<f32>(539, 164, 677, 301)}) {
        auto* panel = item_bank_picture(fill, bounds.i.x, bounds.i.y,
            bounds.getWidth(), bounds.getHeight());
        panel->setBlackWhite(JUtility::TColor(111, 96, 65, 125),
            JUtility::TColor(111, 96, 65, 125));
        auto* inset = item_bank_picture(fill, bounds.i.x + 5, bounds.i.y + 5,
            bounds.getWidth() - 10, bounds.getHeight() - 10);
        inset->setBlackWhite(JUtility::TColor(41, 36, 27, 85),
            JUtility::TColor(41, 36, 27, 85));
    }
    for (int cell = 0; cell < 24; ++cell) {
        const float size = bank_cell_size(cell);
        if (cell == kBankOoccooCell) {
            s_itemBank.frames[cell] = item_bank_picture(circle,
                bank_x(cell) - size * 0.5f, bank_y(cell) - size * 0.5f, size, size);
            continue;
        }
        auto* shade = item_bank_picture(shadow, bank_x(cell) - size * 0.5f - 3,
            bank_y(cell) - size * 0.5f - 2, size + 7, size + 7);
        auto* backing = item_bank_picture(cellBacking, bank_x(cell) - size * 0.5f,
            bank_y(cell) - size * 0.5f, size, size);
        auto* border = item_bank_picture(frame, bank_x(cell) - size * 0.5f,
            bank_y(cell) - size * 0.5f, size, size);
        border->setWhite(JUtility::TColor(164, 161, 125, 255));
        s_itemBank.frames[cell] = border;
        if (cell >= 21 && bank_entry(cell, ring->mItemSlots, ring->mItemsTotal) < 0) {
            shade->hide();
            backing->hide();
            border->hide();
        }
    }
    item_bank_picture(banner, 239, 51, 318, 46);
    if (const auto* pattern = static_cast<const ResTIMG*>(
            dComIfGp_getMain2DArchive()->getResource('TIMG', "tt_kazari_kani_00.bti"))) {
        // Collection uses this same repeating motif and color mapping.
        for (int tile = 0; tile < 5; ++tile) {
            auto* decoration = item_bank_picture(pattern, 251 + tile * 58.8f, 64, 58.8f, 27.1f);
            decoration->setBlackWhite(JUtility::TColor(58, 63, 30, 255),
                JUtility::TColor(30, 34, 13, 255));
        }
    }
    s_itemBank.name = item_bank_text(248, 60, 300, 34, 20);
    s_itemBank.name->setFontColor(JUtility::TColor(230, 240, 139, 255),
        JUtility::TColor(247, 255, 177, 255));
    s_itemBank.assignment = item_bank_text(310, 104, 176, 22, 12);
    s_itemBank.buttons[0] = item_bank_picture(item_assignment_button_texture(true), 352, 127, 24, 24);
    s_itemBank.buttons[1] = item_bank_picture(item_assignment_button_texture(false), 386, 127, 24, 24);
    s_itemBank.buttons[2] = item_bank_picture(styled_r_button_texture(), 425, 123, 32, 32);
    s_itemBank.combo = item_bank_text(kBankComboX + 43, kBankComboY - 1, 240, 25, 12, HBIND_LEFT);
    s_itemBank.comboButton = item_bank_picture(styled_zl_button_texture(),
        kBankComboX, kBankComboY, 32, 22);
    update_item_bank_positions(ring);
    ring->mWaitFrames = 0;
}

void after_item_bank_rotate(ModContext*, void* args, void*, void*) {
    update_item_bank_positions(mods::arg<dMenu_Ring_c*>(args, 0));
}

HookAction before_item_bank_move(ModContext*, void* args, void*, void*) {
    update_item_bank_positions(mods::arg<dMenu_Ring_c*>(args, 0));
    return HOOK_CONTINUE;
}

void after_item_bank_wait_init(ModContext*, void* args, void*, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (uses_item_bank(ring)) ring->mWaitFrames = 0; // STControl owns grid repeat timing.
}

HookAction before_item_bank_stick(ModContext*, void* args, void* retval, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (!uses_item_bank(ring) || retval == nullptr) return HOOK_CONTINUE;
    auto* stick = mods::arg<STControl*>(args, 1);
    *static_cast<u8*>(retval) = 0;
    if (stick == nullptr || ring->mCurrentSlot >= ring->mItemsTotal) return HOOK_SKIP_ORIGINAL;
    stick->checkTrigger();
    BankDirection direction;
    if (stick->checkRightTrigger() || mDoCPd_c::getTrigRight(PAD_1)) direction = BankDirection::Right;
    else if (stick->checkLeftTrigger() || mDoCPd_c::getTrigLeft(PAD_1)) direction = BankDirection::Left;
    else if (stick->checkUpTrigger() || mDoCPd_c::getTrigUp(PAD_1)) direction = BankDirection::Up;
    else if (stick->checkDownTrigger() || mDoCPd_c::getTrigDown(PAD_1)) direction = BankDirection::Down;
    else return HOOK_SKIP_ORIGINAL;
    std::array<bool, 24> occupied{};
    for (unsigned i = 0; i < ring->mItemsTotal && i < MAX_ITEM_SLOTS; ++i) {
        const int cell = item_bank_cell(ring, i);
        if (cell >= 0 && dComIfGs_getItem(ring->mItemSlots[i], false) != dItemNo_NONE_e)
            occupied[cell] = true;
    }
    const int current = item_bank_cell(ring, ring->mCurrentSlot);
    const int next = bank_neighbor(current, direction, occupied);
    const int entry = bank_entry(next, ring->mItemSlots, ring->mItemsTotal);
    if (next == current || entry < 0) return HOOK_SKIP_ORIGINAL;
    ring->mDirectSelectCursorPos.x = ring->mItemSlotPosX[ring->mCurrentSlot];
    ring->mDirectSelectCursorPos.z = ring->mItemSlotPosY[ring->mCurrentSlot];
    ring->mCurrentSlot = static_cast<u8>(entry);
    // Reuse the native straight-line transition used by pointer selection.
    ring->mDirectSelectActive = true;
    ring->field_0x670 = ring->field_0x63e[entry];
    *static_cast<u8*>(retval) = 1;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_item_bank_move_end(ModContext*, void* args, void* retval, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (ring == nullptr || retval == nullptr) return HOOK_CONTINUE;
    if (!uses_item_bank(ring) && !dMw_START_TRIGGER()) return HOOK_CONTINUE;
    // D-pad directions navigate the bank; Start and B close it. The Original
    // Wheel also accepts Start, while retaining its native direction handling.
    // Preserve forced
    // warp/touch closing and the native guards around explanation transitions.
    const bool close = ring->mStatus == dMenu_Ring_c::STATUS_WAIT &&
        ring->mOldStatus != dMenu_Ring_c::STATUS_EXPLAIN &&
        ring->mOldStatus != dMenu_Ring_c::STATUS_EXPLAIN_FORCE &&
        (dMw_B_TRIGGER() || dMw_START_TRIGGER() || dMeter2Info_getWarpStatus() == 1 ||
            dMeter2Info_getWarpStatus() == 2 || dMeter2Info_isTouchKeyCheck(0xe));
    if (close) {
        ring->mRingOrigin = 0xff;
        Z2GetAudioMgr()->seStart(Z2SE_ITEM_RING_OUT, nullptr, 0, 0, 1, 1, -1, -1, 0);
        dMeter2Info_set2DVibrationM();
    }
    *static_cast<bool*>(retval) = close;
    return HOOK_SKIP_ORIGINAL;
}

void draw_item_bank_icons(dMenu_Ring_c* ring) {
    const auto viewport = item_bank_viewport(ring);
    const float scale = viewport.scale;
    const u8 alpha = clamp_hud_alpha(ring->mAlphaRate * (ring->mPlayerIsWolf ? 100.0f : 255.0f));
    for (unsigned i = 0; i < ring->mItemsTotal && i < MAX_ITEM_SLOTS; ++i) {
        const int cell = item_bank_cell(ring, i);
        if (cell < 0) continue;
        const auto item = dComIfGs_getItem(ring->mItemSlots[i], false);
        if (item == dItemNo_NONE_e) continue;
        const float cx = viewport.left + bank_x(cell) * scale;
        const float cy = viewport.top + bank_y(cell) * scale;
        const float extent = bank_icon_extent(ring->mItemSlots[i], cell, i == ring->mCurrentSlot) * scale;
        const float fit = extent / std::max({1.0f, ring->mItemSlotParam1[i], ring->mItemSlotParam2[i]});
        const float width = ring->mItemSlotParam1[i] * fit;
        const float height = ring->mItemSlotParam2[i] * fit;
        for (int layer = 0; layer < 3; ++layer) {
            if (auto* picture = ring->mpItemTex[i][layer]) {
                const float oldX = picture->getScaleX(), oldY = picture->getScaleY();
                picture->scale(1, 1);
                picture->setAlpha(alpha);
                picture->draw(cx - width * 0.5f, cy - height * 0.5f, width, height, false, false, false);
                picture->scale(oldX, oldY);
            }
        }
        const auto max = ring->getItemMaxNum(ring->mItemSlots[i]);
        if (max != 0) ring->drawNumber(ring->getItemNum(ring->mItemSlots[i]), max,
            cx - 10.0f * scale, cy + 25.0f * scale);
        if (item == dItemNo_KANTERA_e && ring->mpKanteraMeter != nullptr && dComIfGs_getMaxOil() != 0) {
            ring->mpKanteraMeter->setPos(cx, cy + 23.0f * scale);
            ring->mpKanteraMeter->setScale(0.5f * scale, 0.5f * scale);
            ring->mpKanteraMeter->setNowGauge(dComIfGs_getMaxOil(), dComIfGs_getOil());
            ring->mpKanteraMeter->setAlphaRate(alpha / 255.0f);
            ring->mpKanteraMeter->drawSelf();
        }
    }
}

void position_item_bank_cursor(dMenu_Ring_c* ring, int cell) {
    const auto viewport = item_bank_viewport(ring);
    auto* cursor = ring->mpDrawCursor;
    cursor->setScale(viewport.scale);
    cursor->setPos(viewport.left + bank_x(cell) * viewport.scale,
        viewport.top + bank_y(cell) * viewport.scale, nullptr, false);
    const float extent = bank_cursor_half_extent(cell, 1.0f, cursor->field_0x40);
    constexpr u64 tags[] = {MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null')};
    // The wheel cursor includes extra item spacing and widescreen expansion.
    // Place its corners from the bank cell instead; retain the gentle pulse.
    for (int corner = 0; corner < 4; ++corner) {
        const float x = corner < 2 ? -extent : extent;
        const float y = corner % 2 == 0 ? -extent : extent;
        cursor->field_0x94[corner] = cursor->field_0x74[corner] = x;
        cursor->field_0xa4[corner] = cursor->field_0x84[corner] = y;
        if (auto* pane = cursor->mpScreen->search(tags[corner])) cursor->moveCenter(pane, x, y);
    }
}

void refresh_item_bank_cursor(dSelect_cursor_c* cursor) {
    auto* ring = s_itemBank.ring;
    if (!uses_item_bank(ring) || cursor != ring->mpDrawCursor) return;
    const int cell = item_bank_cell(ring, ring->mCurrentSlot);
    if (cell >= 0) position_item_bank_cursor(ring, cell);
}

void draw_item_bank(dMenu_Ring_c* ring) {
    auto* graf = dComIfGp_getCurrentGrafPort();
    graf->setup2D();
    update_item_bank_positions(ring);
    const auto viewport = item_bank_viewport(ring);
    const u8 alpha = clamp_hud_alpha(ring->mAlphaRate * 255.0f);
    if (ring->mDrawFlag == 0) {
        ring->mpBlackTex->setAlpha(clamp_hud_alpha(ring->mAlphaRate * 153.0f));
        // Dim the complete game surface, independently of safe-area changes
        // and the menu's native opening/closing translation.
        ring->mpBlackTex->draw(mDoGph_gInf_c::getMinXF(), mDoGph_gInf_c::getMinYF(),
            mDoGph_gInf_c::getMaxXF() - mDoGph_gInf_c::getMinXF(),
            mDoGph_gInf_c::getMaxYF() - mDoGph_gInf_c::getMinYF(), false, false, false);
        auto* root = s_itemBank.root;
        root->scale(viewport.scale, viewport.scale);
        root->move(viewport.left, viewport.top);
        root->setAlpha(alpha);
        copy_item_bank_text(s_itemBank.name, item_bank_source_text(ring->mpCenterScreen,
            MULTI_CHAR('fitem_n1'), MULTI_CHAR('item_n04')));
        const bool prompts = show_ring_assignment_prompts(ring->mPlayerIsWolf, ring->mpItemExplain->getStatus());
        copy_item_bank_text(s_itemBank.assignment, prompts ? item_bank_source_text(ring->mpScreen,
            MULTI_CHAR('fyx_tex'), MULTI_CHAR('yx_text')) : "");
        const ResTIMG* textures[] = {item_assignment_button_texture(true),
            item_assignment_button_texture(false), styled_r_button_texture()};
        for (int i = 0; i < 3; ++i) {
            auto* button = s_itemBank.buttons[i];
            if (button == nullptr) continue;
            if (textures[i] != nullptr) {
                button->changeTexture(textures[i], 0);
                const auto bounds = bank_assignment_prompt(i, textures[i]->width, textures[i]->height);
                button->resize(bounds.width, bounds.height);
                button->move(bounds.x, bounds.y);
                button->setTexCoord(button->getTexture(0), BIND15, MIRROR0, false);
            }
            button->setAlpha(prompts && (i != 2 || !midna_uses_right_shoulder()) ? 255 : 0);
        }
        const bool mixOff = ring->isMixItemOff();
        const bool comboVisible = prompts && (mixOff || ring->isMixItemOn());
        std::string combo = comboVisible ? text_box_string(mixOff ?
            ring->mpBowArrowComboString[0] : ring->mpComboOffString[0]) : "";
        if (const auto placeholder = combo.find("%s"); placeholder != std::string::npos)
            combo.erase(placeholder, 2);
        copy_item_bank_text(s_itemBank.combo, combo.c_str());
        if (auto* button = s_itemBank.comboButton) {
            if (auto* texture = styled_zl_button_texture()) button->changeTexture(texture, 0);
            button->setAlpha(bank_combo_button_alpha(prompts, comboVisible));
        }
        s_itemBank.screen->draw(0, 0, graf);
        if (comboVisible && ring->mPikariFlashingSpeed > 0.0f && ring->mAlphaRate >= 1.0f) {
            if (auto* meter = dMeter2Info_getMeterClass()) {
                // Preserve the game's combination flash, timing and sound.
                meter->getMeterDrawPtr()->drawPikari(
                    viewport.left + (kBankComboX + 16) * viewport.scale,
                    viewport.top + (kBankComboY + 11) * viewport.scale,
                    &ring->mPikariFlashingSpeed, g_ringHIO.mPikariScale * viewport.scale,
                    g_ringHIO.mPikariFrontOuter, g_ringHIO.mPikariFrontInner,
                    g_ringHIO.mPikariBackOuter, g_ringHIO.mPikariBackInner,
                    g_ringHIO.mPikariAnimSpeed, 2);
            }
        }
        draw_item_bank_icons(ring);
        ring->mDrawFlag = 1;
    } else {
        // This draw advances and commits native assignment animations. It must
        // run even while closing, otherwise pending assignments never finish.
        ring->drawSelectItem();
        const int cell = item_bank_cell(ring, ring->mCurrentSlot);
        if (cell >= 0 && ring->mpItemExplain->getStatus() == 0) {
            ring->mpDrawCursor->setAlphaRate(ring->mAlphaRate);
            ring->mpDrawCursor->setParam(1, 1, 0.04f, 0.6f, 0.5f);
            position_item_bank_cursor(ring, cell);
            ring->mpDrawCursor->draw();
        }
        ring->mpItemExplain->trans(ring->mCenterPosX, ring->mCenterPosY);
        ring->mpItemExplain->draw(static_cast<J2DOrthoGraph*>(graf));
        ring->drawFlag0();
    }
}
