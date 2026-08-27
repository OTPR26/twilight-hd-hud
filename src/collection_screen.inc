// Collection-specific native integration. Included inside item_slot_hooks.cpp
// so it can share the existing texture/style resources without new hooks into
// gameplay or a second owner for the game's panes.
struct CollectionScreen {
    J2DPane* root = nullptr;
    J2DPicture* background = nullptr;
    std::array<J2DPicture*, 4> rules{};
    std::array<std::array<J2DPicture*, 2>, 7> equipmentFlourishes{};
    J2DPane* prompts = nullptr;
    std::array<J2DTextBox*, 2> promptLabels{};
    std::array<J2DPicture*, 2> promptIcons{};
    J2DTextBox* description = nullptr;
    std::string walletDescription;
    bool japanese = false;
    J2DTextBox::TFontSize measuredIconFont{};
    J2DTextBox::TFontSize renderedIconFont{};
    float iconBaseline = 0;
    float measuredIconSpacing = 0;
    float renderedIconSpacing = 0;
    struct TextSlot {
        float x;
        float width;
        bool followsSpace;
    };
    std::array<TextSlot, 35> textSlots{};
    std::size_t textSlotCount = 0;
    J2DPrint* drawingText = nullptr;
    std::string drawnText;
    float slotStart = 0;
    bool slotFollowsSpace = false;
    bool readingSlot = false;
    std::array<J2DPane*, kCollectionCells.size()> cells{};
    std::array<J2DPicture*, kCollectionCells.size()> frames{};
};

CollectionScreen s_collectionScreen;
dMenu_Collect3D_c* s_collectionModelScope = nullptr;
cXyz s_collectionModelPosition;

CollectionViewport current_collection_viewport() {
    return collection_viewport(mDoGph_gInf_c::getSafeMinXF(), mDoGph_gInf_c::getSafeMinYF(),
        mDoGph_gInf_c::getSafeMaxXF(), mDoGph_gInf_c::getSafeMaxYF());
}

void collection_place(dMenu_Collect2D_c* menu, J2DPane* pane,
    float x, float y, float width, float height, bool uniform = true) {
    if (pane == nullptr || pane->getWidth() <= 0 || pane->getHeight() <= 0) return;
    const auto viewport = current_collection_viewport();
    float parentX = 1.0f, parentY = 1.0f;
    for (auto* parent = pane->getParentPane(); parent != nullptr; parent = parent->getParentPane()) {
        parentX *= parent->getScaleX();
        parentY *= parent->getScaleY();
    }
    if (std::fabs(parentX) < 0.001f || std::fabs(parentY) < 0.001f) return;
    float sx = width / pane->getWidth();
    float sy = height / pane->getHeight();
    if (uniform) sx = sy = std::min(sx, sy);
    pane->scale(sx * viewport.scale / parentX, sy * viewport.scale / parentY);
    const Vec center = menu->mpLinkPm->getGlobalVtxCenter(pane, false, 0);
    offset_diamond_group(std::array<J2DPane*, 1>{pane},
        viewport.left + x * viewport.scale - center.x,
        viewport.top + y * viewport.scale - center.y);
}

void collection_text(J2DScreen* screen, J2DPane* parent, u64 tag,
    float x, float y, float width, float height, float size, JUtility::TColor color) {
    auto* text = static_cast<J2DTextBox*>(screen->search(tag));
    if (text == nullptr) return;
    parent->appendChild(text);
    text->scale(1.0f, 1.0f);
    text->resize(width, height);
    text->move(x, y);
    text->setFontSize(size, size);
    text->setCharSpace(0.0f);
    text->setLineSpace(size * 1.3f);
    text->field_0x10c = 0;
    text->field_0x110 = 0;
    text->mFlags = static_cast<u8>((text->mFlags & ~0x0f) | (HBIND_LEFT << 2) | VBIND_TOP);
    text->setFontColor(color, color);
    text->setBlackWhite(JUtility::TColor(0, 0, 0, 0), JUtility::TColor(255, 255, 255, 255));
    text->setAlpha(255);
    text->show();
}

J2DPicture* collection_flourish(J2DPane* parent, u64 tag,
    float x, float y, float width, float height, bool flipped = false,
    const char* textureName = "tt_gold_uzu_long2.bti") {
    const auto* texture = collect_archive_texture(textureName);
    if (texture == nullptr) return nullptr;
    auto* picture = JKR_NEW J2DPicture(tag,
        JGeometry::TBox2<f32>(x, y, x + width, y + height), texture, nullptr);
    configure_hd_picture(picture);
    picture->setTexCoord(picture->getTexture(0), BIND15,
        flipped ? static_cast<J2DMirror>(3) : MIRROR0, false);
    picture->setWhite(JUtility::TColor(229, 222, 165, 255));
    parent->appendChild(picture);
    return picture;
}

void create_collection_prompts(dMenu_Collect2D_c* menu) {
    auto* screen = menu->getIconScreen();
    if (screen == nullptr) return;
    auto* root = JKR_NEW J2DPane(MULTI_CHAR('hd_cprm'),
        JGeometry::TBox2<f32>(0, 0, 160, 70));
    screen->appendChild(root);
    s_collectionScreen.prompts = root;
    collection_flourish(root, MULTI_CHAR('hd_cpf1'), 106, 16, 45, 43);
    for (int i = 0; i < 2; ++i) {
        const float x = i == 0 ? 128.0f : 108.0f;
        const float y = i == 0 ? 18.0f : 40.0f;
        auto* label = JKR_NEW J2DTextBox(MULTI_CHAR('hd_cpt0') + i,
            JGeometry::TBox2<f32>(0, y - 10, x - 14, y + 10),
            nullptr, "", 128, HBIND_RIGHT, VBIND_CENTER);
        label->setFont(mDoExt_getMesgFont());
        label->setFontSize(11.5f, 11.5f);
        label->setCharSpace(0);
        label->setFontColor(JUtility::TColor(238, 238, 232, 255),
            JUtility::TColor(255, 255, 255, 255));
        root->appendChild(label);
        s_collectionScreen.promptLabels[i] = label;
        if (const auto* texture = menu_face_button_texture(i == 0)) {
            auto* icon = JKR_NEW J2DPicture(MULTI_CHAR('hd_cpi0') + i,
                JGeometry::TBox2<f32>(x - 11, y - 11, x + 11, y + 11), texture, nullptr);
            configure_hd_picture(icon);
            root->appendChild(icon);
            s_collectionScreen.promptIcons[i] = icon;
        }
    }
}

void apply_collection_prompts(dMenu_Collect2D_c* menu) {
    auto* root = s_collectionScreen.prompts;
    if (root == nullptr || menu == nullptr || menu != s_activeCollectMenu) return;
    const auto viewport = current_collection_viewport();
    const float right = (mDoGph_gInf_c::getSafeMaxXF() - viewport.left) / viewport.scale;
    // The visible top of A meets the inner top rule at this height.
    collection_place(menu, root, right - 92, kCollectionPromptCenterY, 160, 70);
    auto* screen = menu->getIconScreen();
    for (u64 tag : {MULTI_CHAR('abtn_n'), MULTI_CHAR('bbtn_n'),
             MULTI_CHAR('a_text_n'), MULTI_CHAR('b_text_n')}) {
        if (auto* pane = screen->search(tag)) pane->hide();
    }
    bool anyVisible = false;
    constexpr u64 tags[] = {MULTI_CHAR('atext1_5'), MULTI_CHAR('btext1_5')};
    for (int i = 0; i < 2; ++i) {
        auto* source = static_cast<J2DTextBox*>(screen->search(tags[i]));
        const char* value = text_box_string(source);
        if (value == nullptr) value = "";
        auto* label = s_collectionScreen.promptLabels[i];
        // Despite its variadic signature, this setter copies a literal string.
        if (std::strcmp(text_box_string(label), value) != 0) label->setString(128, value);
        const bool visible = value[0] != '\0';
        anyVisible |= visible;
        label->setAlpha(visible ? 255 : 0);
        if (auto* icon = s_collectionScreen.promptIcons[i]) {
            const auto* texture = menu_face_button_texture(i == 0);
            if (texture != nullptr && (icon->getTexture(0) == nullptr ||
                    icon->getTexture(0)->getTexInfo() != texture)) icon->changeTexture(texture, 0);
            icon->setAlpha(visible ? 255 : 0);
        }
    }
    if (anyVisible) root->show();
    else root->hide();
}

void create_collection_screen(dMenu_Collect2D_c* menu) {
    s_collectionScreen = {};
    if (menu == nullptr || menu->mpScreen == nullptr || menu->mpLinkPm == nullptr) return;
    auto* screen = menu->mpScreen;
    auto* background = as_picture(screen->search(MULTI_CHAR('w_bas05')));
    const auto* paperTexture = collect_archive_texture("tt_block8x8.bti");
    // Keep the original screen intact if a different game revision lacks the
    // expected resources. Nothing is detached until all required panes exist.
    if (background == nullptr || paperTexture == nullptr ||
        menu->mpHeartParent == nullptr || s_collectTitleFrame == nullptr ||
        s_collectTitleLabel == nullptr) return;
    for (const auto& cell : kCollectionCells) {
        if (menu->mpSelPm[cell.column][cell.row] == nullptr) return;
    }

    auto* root = JKR_NEW J2DPane(MULTI_CHAR('hd_colly'),
        JGeometry::TBox2<f32>(0, 0, 796, 448));
    screen->appendChild(root);
    s_collectionScreen.root = root;
    root->appendChild(background);
    s_collectionScreen.background = background;

    // These rules sit behind the banner and footer buttons, not in the
    // original decorative subtree that the new layout hides.
    constexpr float ruleY[] = {20, 23, 420, 423};
    for (std::size_t i = 0; i < s_collectionScreen.rules.size(); ++i) {
        auto* rule = JKR_NEW J2DPicture(MULTI_CHAR('hd_cedg0') + i,
            JGeometry::TBox2<f32>(0, ruleY[i], 796, ruleY[i] + 0.7f), paperTexture, nullptr);
        configure_hd_picture(rule);
        const JUtility::TColor color(185, 179, 132, i % 2 == 0 ? 220 : 150);
        rule->setBlackWhite(color, color);
        root->appendChild(rule);
        s_collectionScreen.rules[i] = rule;
    }

    // Frame art lives outside the icon groups. Move the actual equipment
    // frames so the game's equipped-item color updates still reach them.
    constexpr u64 equipmentFrames[] = {
        MULTI_CHAR('fuku_g_1'), MULTI_CHAR('fuku_g_0'), MULTI_CHAR('fuku_g_2'),
        MULTI_CHAR('tate_g_0'), MULTI_CHAR('tate_g_1'),
        MULTI_CHAR('ken_g_0'), MULTI_CHAR('ken_g_1'),
    };
    auto* frameSource = as_picture(screen->search(MULTI_CHAR('ken_g_0')));
    const auto* frameTexture = frameSource != nullptr && frameSource->getTexture(0) != nullptr ?
        frameSource->getTexture(0)->getTexInfo() : nullptr;
    for (std::size_t i = 0; i < kCollectionCells.size(); ++i) {
        const auto& cell = kCollectionCells[i];
        J2DPicture* frame = nullptr;
        if (i < std::size(equipmentFrames)) {
            frame = as_picture(screen->search(equipmentFrames[i]));
        } else if (cell.row == 3 || cell.row == 4) {
            if (frameTexture != nullptr) {
                frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_cfr00') + i,
                    JGeometry::TBox2<f32>(0, 0, 64, 64), frameTexture, nullptr);
                configure_hd_picture(frame);
                frame->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    JUtility::TColor(153, 149, 112, 255));
            }
        }
        if (frame != nullptr) {
            if (i < std::size(equipmentFrames)) {
                if (const auto* replacement = resource_texture(s_collectEquipmentFrameResource)) {
                    const auto tint = frame->getWhite();
                    frame->changeTexture(replacement, 0);
                    configure_hd_picture(frame);
                    frame->setBlackWhite(JUtility::TColor(0, 0, 0, 0), tint);
                }
            }
            root->appendChild(frame);
            frame->show();
            s_collectionScreen.frames[i] = frame;
        }
        if (i < s_collectionScreen.equipmentFlourishes.size()) {
            for (int corner = 0; corner < 2; ++corner) {
                const float offset = cell.width * 0.5f + 3;
                s_collectionScreen.equipmentFlourishes[i][corner] = collection_flourish(root,
                    MULTI_CHAR('hd_cef00') + i * 2 + corner,
                    cell.x + (corner == 0 ? -offset - 6 : offset - 18),
                    cell.y + (corner == 0 ? -offset - 6 : offset - 18), 24, 24, corner != 0,
                    "tt_kazari_2nd_okan_64.bti");
                if (auto* flourish = s_collectionScreen.equipmentFlourishes[i][corner]) {
                    flourish->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                        JUtility::TColor(246, 244, 198, 255));
                }
            }
        }
    }

    for (std::size_t i = 0; i < kCollectionCells.size(); ++i) {
        const auto& cell = kCollectionCells[i];
        J2DPane* pane = cell.column == 5 && cell.row == 0 ?
            menu->mpHeartParent->getPanePtr() : menu->mpSelPm[cell.column][cell.row]->getPanePtr();
        root->appendChild(pane);
        s_collectionScreen.cells[i] = pane;
    }
    // Start with fresh local bounds: the legacy title has already been
    // positioned against the old screen's cached transforms during creation.
    const auto* banner = resource_texture(s_collectBannerResource);
    auto* title = JKR_NEW J2DPicture(MULTI_CHAR('hd_cban'),
        JGeometry::TBox2<f32>(42, 19, 277, 57),
        banner != nullptr ? banner : resource_texture(s_collectMenuButtonResource), nullptr);
    configure_hd_picture(title);
    if (banner == nullptr) title->setBlackWhite(JUtility::TColor(38, 40, 20, 255),
        JUtility::TColor(224, 224, 178, 255));
    root->appendChild(title);
    if (banner != nullptr) {
        if (const auto* pattern = collect_archive_texture("tt_kazari_kani_00.bti")) {
            // Preserve the motif's aspect ratio; repeating it avoids stretching
            // a single crest across the whole banner. Stay inside the inset corners.
            for (int tile = 0; tile < 4; ++tile) {
                const float x = 51.0f + tile * 54.25f;
                auto* decoration = JKR_NEW J2DPicture(MULTI_CHAR('hd_cbpa0') + tile,
                    JGeometry::TBox2<f32>(x, 29, x + 54.25f, 54), pattern, nullptr);
                configure_hd_picture(decoration);
                decoration->setBlackWhite(JUtility::TColor(58, 63, 30, 255),
                    JUtility::TColor(30, 34, 13, 255));
                root->appendChild(decoration);
            }
        }
    }
    auto* titleLabel = JKR_NEW J2DTextBox(MULTI_CHAR('hd_cbtxt'),
        JGeometry::TBox2<f32>(48, 28, 271, 56), nullptr, "Collection", 32,
        HBIND_CENTER, VBIND_CENTER);
    titleLabel->setFont(s_collectTitleLabel->getFont());
    titleLabel->setFontSize(17, 17);
    titleLabel->setCharSpace(0);
    titleLabel->setFontColor(JUtility::TColor(245, 245, 238, 255),
        JUtility::TColor(255, 255, 255, 255));
    root->appendChild(titleLabel);
    s_collectTitleFrame->hide();
    s_collectTitleLabel->hide();

    // Retain the game's heart-piece count and animation, and reveal its
    // original ornament rather than drawing a second heart on top of it.
    for (u64 tag : {MULTI_CHAR('uzu_00'), MULTI_CHAR('uzu_01'), MULTI_CHAR('uzu_02'),
             MULTI_CHAR('uzu_03'), MULTI_CHAR('uzu_04'), MULTI_CHAR('uzu_05'),
             MULTI_CHAR('uzu_06'), MULTI_CHAR('uzu_07'), MULTI_CHAR('hishi')}) {
        if (auto* ornament = as_picture(screen->search(tag))) {
            ornament->show();
            ornament->setWhite(JUtility::TColor(229, 222, 165, 255));
        }
    }
    for (u64 tag : {MULTI_CHAR('f_sav_0'), MULTI_CHAR('f_opt_0')}) {
        if (auto* label = static_cast<J2DTextBox*>(screen->search(tag))) label->setFontSize(14, 14);
    }
    create_collection_prompts(menu);

    const auto* parchment = resource_texture(s_collectParchmentResource);
    auto* paper = JKR_NEW J2DPicture(MULTI_CHAR('hd_cpap'),
        JGeometry::TBox2<f32>(500, 280, 770, 392),
        parchment != nullptr ? parchment : paperTexture, nullptr);
    configure_hd_picture(paper);
    if (parchment == nullptr) {
        paper->setBlackWhite(JUtility::TColor(184, 174, 140, 255),
            JUtility::TColor(234, 228, 202, 255));
    }
    root->appendChild(paper);
    if (const auto* ornament = collect_archive_texture("tt_gold_uzu_long2.bti")) {
        auto* flourish = JKR_NEW J2DPicture(MULTI_CHAR('hd_cpfl'),
            JGeometry::TBox2<f32>(735, 287, 759, 309), ornament, nullptr);
        configure_hd_picture(flourish);
        flourish->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(173, 145, 83, 170));
        root->appendChild(flourish);
    }
    auto* rule = JKR_NEW J2DPicture(MULTI_CHAR('hd_crul'),
        JGeometry::TBox2<f32>(515, 309, 755, 309.7f), paperTexture, nullptr);
    configure_hd_picture(rule);
    rule->setBlackWhite(JUtility::TColor(162, 137, 75, 255),
        JUtility::TColor(162, 137, 75, 255));
    root->appendChild(rule);

    // The game already chooses the region-specific text layer. Inspect that
    // choice instead of depending on a host-only version API not in the SDK.
    const auto* westernText = screen->search(MULTI_CHAR('f_text0'));
    const bool japanese = westernText != nullptr && !westernText->isVisible();
    s_collectionScreen.japanese = japanese;
    collection_text(screen, root, japanese ? MULTI_CHAR('item_n00') : MULTI_CHAR('item_n04'),
        516, 289, 238, 19, 13, JUtility::TColor(133, 91, 32, 255));
    collection_text(screen, root, japanese ? MULTI_CHAR('i_text0') : MULTI_CHAR('f_text0'),
        516, 317, 238, 70, 10.5f, JUtility::TColor(91, 73, 36, 255));
    s_collectionScreen.description = static_cast<J2DTextBox*>(screen->search(
        japanese ? MULTI_CHAR('i_text0') : MULTI_CHAR('f_text0')));
    // Shadow copies remain in the hidden original hierarchy. The native
    // renderer can still write them and look up its old layout containers.
}

bool collection_description_reference(jmessage_string_tReference* reference) {
    return s_collectionScreen.description != nullptr && reference != nullptr &&
        reference->getPanePtr() == s_collectionScreen.description;
}

void after_collection_wallet_message(ModContext*, void* args, void* result, void*) {
    auto* control = mods::arg<JMessage::TControl*>(args, 0);
    auto* menu = s_activeCollectMenu;
    if (result == nullptr || !*static_cast<bool*>(result) || menu == nullptr ||
        menu->mCursorX != 0 || menu->mCursorY != 3 || menu->mpString == nullptr ||
        control == nullptr || control != menu->mpString->mpCtrl ||
        !collection_description_reference(menu->mpString->mpRefer) ||
        control->pResourceCache_ == nullptr || control->pMessageText_begin_ == nullptr) return;

    const auto* resource = control->pResourceCache_;
    // Byte-oriented BMG text only. Never reinterpret UTF-16 or write into the
    // shared message archive (including any separately patched game image).
    if (resource->oParse_THeader_.get_encoding() == 2 || resource->pMessageText_ == nullptr) return;
    const JMessage::data::TParse_TBlock_messageText block(resource->pMessageText_ - 8);
    const auto blockSize = block.get_size();
    const auto start = reinterpret_cast<std::uintptr_t>(resource->pMessageText_);
    const auto text = reinterpret_cast<std::uintptr_t>(control->pMessageText_begin_);
    if (blockSize < 8 || text < start || text - start >= blockSize - 8) return;
    auto replacement = wallet_description_capacity({control->pMessageText_begin_,
        blockSize - 8 - (text - start)}, dComIfGs_getRupeeMax());
    if (replacement.empty()) return;
    s_collectionScreen.walletDescription = std::move(replacement);
    // getString runs on every Collection draw, so toggling Bigger Wallets is
    // reflected on the next draw. Both measuring and rendering use this copy.
    control->pMessageText_begin_ = s_collectionScreen.walletDescription.c_str();
    control->pMessageText_current_ = s_collectionScreen.walletDescription.c_str();
}

HookAction before_collection_icon_measure(ModContext*, void* args, void*, void*) {
    auto* processor = mods::arg<jmessage_string_tMeasureProcessor*>(args, 0);
    const u32 tag = mods::arg<u32>(args, 1);
    if (!collection_description_reference(processor->mpReference) ||
        tag < MSGTAG_ABTN || tag > MSGTAG_DPAD) return HOOK_CONTINUE;
    auto* text = s_collectionScreen.description;
    text->getFontSize(s_collectionScreen.measuredIconFont);
    const auto size = s_collectionScreen.measuredIconFont;
    const float scale = collection_inline_icon_scale(tag - MSGTAG_ABTN);
    s_collectionScreen.measuredIconSpacing = text->getCharSpace();
    text->setCharSpace(text->getCharSpace() + kCollectionIconSpacing);
    text->setFontSize(size.mSizeX * scale, size.mSizeY * scale);
    return HOOK_CONTINUE;
}

void after_collection_icon_measure(ModContext*, void* args, void*, void*) {
    auto* processor = mods::arg<jmessage_string_tMeasureProcessor*>(args, 0);
    const u32 tag = mods::arg<u32>(args, 1);
    if (!collection_description_reference(processor->mpReference) ||
        tag < MSGTAG_ABTN || tag > MSGTAG_DPAD) return;
    const auto size = s_collectionScreen.measuredIconFont;
    s_collectionScreen.description->setFontSize(size.mSizeX, size.mSizeY);
    s_collectionScreen.description->setCharSpace(s_collectionScreen.measuredIconSpacing);
}

float collection_prefix_width(J2DTextBox* text, const char* prefix) {
    // Match J2DPrint's actual cursor advance, including embedded font-size
    // escapes and preceding icon slots, rather than the message parser's estimate.
    const auto parts = collection_text_prefix(prefix);
    J2DTextBox::TFontSize size;
    text->getFontSize(size);
    auto measure = [&](const std::string& value) {
        J2DPrint print(text->getFont(), text->getCharSpace(), text->getLineSpace(),
            JUtility::TColor(255, 255, 255, 255), JUtility::TColor(255, 255, 255, 255),
            JUtility::TColor(0, 0, 0, 0), JUtility::TColor(255, 255, 255, 255));
        print.setFontSize(size.mSizeX, size.mSizeY);
        print.initchar();
        print.locate(0, 0);
        J2DPrint::TSize bounds{};
        print.parse(reinterpret_cast<const u8*>(value.c_str()), value.size(),
            100000, nullptr, bounds, 255, false);
        return bounds.field_0x0;
    };
    // A final space makes trailing cursor-control escapes count in the bounds.
    return measure(parts.line + " ") - measure(parts.format + " ");
}

HookAction before_collection_icon_render(ModContext*, void* args, void*, void*) {
    auto* processor = mods::arg<jmessage_string_tRenderingProcessor*>(args, 0);
    if (!collection_description_reference(processor->mpReference)) return HOOK_CONTINUE;
    auto* text = s_collectionScreen.description;
    text->getFontSize(s_collectionScreen.renderedIconFont);
    const auto size = s_collectionScreen.renderedIconFont;
    const auto type = mods::arg<u8>(args, 1);
    const float scale = collection_inline_icon_scale(type);
    const bool controllerIcon = type <= 8;
    const std::string_view prefix = processor->getString();
    const float prefixWidth = collection_prefix_width(text, processor->getString());
    processor->field_0x4c = controllerIcon ? collection_inline_icon_x(
        prefixWidth, !prefix.empty() && prefix.back() == ' ') : prefixWidth;
    s_collectionScreen.renderedIconSpacing = text->getCharSpace();
    text->setCharSpace(text->getCharSpace() + (controllerIcon ? kCollectionIconSpacing : 0.0f));
    s_collectionScreen.iconBaseline = processor->field_0x50;
    const auto* font = text->getFont();
    const float baseline = size.mSizeY * font->getAscent() / font->getCellHeight();
    const float textCenter = baseline - 0.375f * size.mSizeY;
    // OutFont adds one pixel for Western text. Center its visible cap on the
    // text's capital-height band instead of the padded font cell.
    const float nativeYOffset = s_collectionScreen.japanese ? 0.0f : 1.0f;
    processor->field_0x50 += textCenter - scale * size.mSizeY * 0.5f - nativeYOffset;
    // Let the native parser reserve the wider slot, keeping following text
    // clear of the icon. Measurement uses the same width above.
    text->setFontSize(size.mSizeX * scale, size.mSizeY * scale);
    return HOOK_CONTINUE;
}

void after_collection_icon_render(ModContext*, void* args, void*, void*) {
    auto* processor = mods::arg<jmessage_string_tRenderingProcessor*>(args, 0);
    if (!collection_description_reference(processor->mpReference)) return;
    const auto size = s_collectionScreen.renderedIconFont;
    s_collectionScreen.description->setFontSize(size.mSizeX, size.mSizeY);
    s_collectionScreen.description->setCharSpace(s_collectionScreen.renderedIconSpacing);
    processor->field_0x50 = s_collectionScreen.iconBaseline;
}

// Collection draws its text before building the separate icon queue. Capture
// the actual CR slots from that draw, then anchor the queued icons to them.
// This avoids maintaining a second estimate of the font renderer's cursor.
HookAction before_collection_text_parse(ModContext*, void* args, void*, void*) {
    auto& state = s_collectionScreen;
    const auto* string = mods::arg<const u8*>(args, 1);
    if (state.description == nullptr || !mods::arg<bool>(args, 7) ||
        string != reinterpret_cast<const u8*>(text_box_string(state.description))) return HOOK_CONTINUE;
    state.drawingText = mods::arg<J2DPrint*>(args, 0);
    state.drawnText = reinterpret_cast<const char*>(string);
    state.textSlotCount = 0;
    state.readingSlot = false;
    return HOOK_CONTINUE;
}

void after_collection_text_parse(ModContext*, void* args, void*, void*) {
    if (s_collectionScreen.drawingText == mods::arg<J2DPrint*>(args, 0)) {
        s_collectionScreen.drawingText = nullptr;
        s_collectionScreen.readingSlot = false;
    }
}

HookAction before_collection_text_escape(ModContext*, void* args, void*, void*) {
    auto& state = s_collectionScreen;
    auto* print = mods::arg<J2DPrint*>(args, 0);
    if (print != state.drawingText) return HOOK_CONTINUE;
    const auto* code = *mods::arg<const u8**>(args, 1);
    state.readingSlot = code[0] == 'C' && code[1] == 'R';
    if (state.readingSlot) {
        state.slotStart = print->mCursorH;
        const auto* start = reinterpret_cast<const u8*>(text_box_string(state.description));
        state.slotFollowsSpace = code - start >= 2 && code[-2] == ' ';
    }
    return HOOK_CONTINUE;
}

void after_collection_text_escape(ModContext*, void* args, void* retval, void*) {
    auto& state = s_collectionScreen;
    auto* print = mods::arg<J2DPrint*>(args, 0);
    if (print != state.drawingText || !state.readingSlot) return;
    state.readingSlot = false;
    if (*static_cast<u16*>(retval) == MULTI_CHAR('CR') &&
        state.textSlotCount < state.textSlots.size()) {
        state.textSlots[state.textSlotCount++] = {
            state.slotStart - print->field_0x24,
            print->mCursorH - state.slotStart,
            state.slotFollowsSpace,
        };
    }
}

HookAction before_collection_out_font_draw(ModContext*, void* args, void*, void*) {
    auto& state = s_collectionScreen;
    auto* strings = mods::arg<dMsgString_c*>(args, 0);
    auto* text = mods::arg<J2DTextBox*>(args, 1);
    if (text == nullptr || text != state.description || strings->mpOutFont == nullptr)
        return HOOK_CONTINUE;
    std::size_t count = 0;
    for (auto* icon : strings->mpOutFont->mpOfs)
        if (icon != nullptr && icon->getTextBoxPtr() == text && icon->getType() != 0x47) ++count;
    // Other cursor-control escapes must not accidentally be paired with icons.
    if (!collection_icon_slots_match(state.drawnText, text_box_string(text),
        state.textSlotCount, count)) return HOOK_CONTINUE;
    std::size_t index = 0;
    for (auto* icon : strings->mpOutFont->mpOfs) {
        if (icon == nullptr || icon->getTextBoxPtr() != text || icon->getType() == 0x47) continue;
        const auto& slot = state.textSlots[index++];
        icon->mPosX = icon->getType() <= 8 ? collection_inline_icon_x(slot.x, slot.followsSpace) : slot.x;
    }
    return HOOK_CONTINUE;
}

void apply_collection_screen(dMenu_Collect2D_c* menu) {
    auto* root = s_collectionScreen.root;
    if (menu == nullptr || menu != s_activeCollectMenu || root == nullptr) return;
    collection_place(menu, root, 398, 224, 796, 448);
    // Native widescreen passes still find their original containers, but
    // those empty/decorative subtrees no longer draw the portrait frame.
    for (auto* child = menu->mpScreen->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        if (child != root) child->hide();
    }
    root->show();
    const auto viewport = current_collection_viewport();
    const float safeWidth = (mDoGph_gInf_c::getSafeMaxXF() - mDoGph_gInf_c::getSafeMinXF()) / viewport.scale;
    constexpr float ruleY[] = {20.35f, 23.35f, 420.35f, 423.35f};
    for (std::size_t i = 0; i < s_collectionScreen.rules.size(); ++i)
        collection_place(menu, s_collectionScreen.rules[i], 398, ruleY[i], safeWidth, 0.7f, false);
    collection_place(menu, s_collectionScreen.background, 398, 224,
        (mDoGph_gInf_c::getSafeMaxXF() - mDoGph_gInf_c::getSafeMinXF()) / viewport.scale,
        (mDoGph_gInf_c::getSafeMaxYF() - mDoGph_gInf_c::getSafeMinYF()) / viewport.scale, false);
    for (std::size_t i = 0; i < kCollectionCells.size(); ++i) {
        const auto& cell = kCollectionCells[i];
        collection_place(menu, s_collectionScreen.frames[i], cell.x, cell.y,
            cell.width + 6.0f, cell.width + 6.0f);
        collection_place(menu, s_collectionScreen.cells[i], cell.x, cell.y,
            cell.width, cell.row == 5 ? 28.0f : cell.width);
        if (i < s_collectionScreen.equipmentFlourishes.size()) {
            auto* frame = s_collectionScreen.frames[i];
            // Native equip changes overwrite this tint. Recognize both its
            // original yellow and our ivory; unequipping restores native gray.
            const bool equipped = frame != nullptr && frame->getWhite().r > 200;
            if (frame != nullptr) frame->setWhite(equipped ? JUtility::TColor(246, 244, 198, 255) :
                JUtility::TColor(132, 134, 104, 255));
            for (auto* flourish : s_collectionScreen.equipmentFlourishes[i]) {
                if (flourish == nullptr) continue;
                if (equipped) flourish->show();
                else flourish->hide();
            }
        }
    }
    // cursorPosSet() uses the correct item ID but may have run before native
    // layout resets. Refresh its anchor after the final pane transforms.
    auto* selected = menu->mpSelPm[menu->mCursorX][menu->mCursorY];
    if (selected != nullptr && menu->mpDrawCursor != nullptr) {
        const Vec center = selected->getGlobalVtxCenter(false, 0);
        menu->mpDrawCursor->setPos(center.x, center.y, selected->getPanePtr(), false);
    }
}

HookAction before_collection_cursor(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    if (s_collectionScreen.root == nullptr || menu != s_activeCollectMenu || menu->mpStick == nullptr)
        return HOOK_CONTINUE;
    menu->mpStick->checkTrigger();
    CollectionDirection direction;
    if (menu->mpStick->checkRightTrigger()) direction = CollectionDirection::Right;
    else if (menu->mpStick->checkLeftTrigger()) direction = CollectionDirection::Left;
    else if (menu->mpStick->checkUpTrigger()) direction = CollectionDirection::Up;
    else if (menu->mpStick->checkDownTrigger()) direction = CollectionDirection::Down;
    else return HOOK_SKIP_ORIGINAL;

    std::array<bool, kCollectionCells.size()> available{};
    for (std::size_t i = 0; i < available.size(); ++i) {
        const auto& cell = kCollectionCells[i];
        available[i] = cell.row == 5 || (cell.column == 5 && cell.row == 0) ||
            menu->field_0x22d[cell.column][cell.row] != 0;
    }
    const int current = collection_cell_index(menu->mCursorX, menu->mCursorY);
    const int next = collection_neighbor(current, direction, available);
    if (next >= 0 && next != current) {
        menu->field_0x259 = menu->mCursorX;
        menu->field_0x25a = menu->mCursorY;
        menu->mCursorX = static_cast<u8>(kCollectionCells[next].column);
        menu->mCursorY = static_cast<u8>(kCollectionCells[next].row);
        menu->cursorPosSet();
        menu->setItemNameString(menu->mCursorX, menu->mCursorY);
        Z2GetAudioMgr()->seStart(menu->mCursorY == 5 ? Z2SE_SY_CURSOR_OPTION : Z2SE_SY_CURSOR_ITEM,
            nullptr, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
        apply_collection_screen(menu);
    }
    return HOOK_SKIP_ORIGINAL;
}

void position_collection_cursor(dMenu_Collect2D_c* menu) {
    if (s_collectionScreen.root == nullptr || menu == nullptr || menu->mpDrawCursor == nullptr)
        return;
    const int index = collection_cell_index(menu->mCursorX, menu->mCursorY);
    if (index < 0) return;
    J2DPane* target = s_collectionScreen.frames[index];
    if (menu->mCursorY == 5) target = menu->mCursorX == 0 ? s_collectSaveFrame : s_collectOptionsFrame;
    if (target == nullptr) target = menu->mpSelPm[menu->mCursorX][menu->mCursorY]->getPanePtr();
    if (target == nullptr) return;
    Mtx matrix;
    const Vec first = menu->mpLinkPm->getGlobalVtx(target, &matrix, 0, false, 0);
    const Vec last = menu->mpLinkPm->getGlobalVtx(target, &matrix, 3, false, 0);
    auto* cursor = menu->mpDrawCursor;
    cursor->setPos((first.x + last.x) * 0.5f, (first.y + last.y) * 0.5f, nullptr, false);
    const float scale = current_collection_viewport().scale;
    const float halfWidth = collection_cursor_half_extent(std::fabs(last.x - first.x), scale);
    const float halfHeight = collection_cursor_half_extent(std::fabs(last.y - first.y), scale);
    const float phase = cursor->field_0x40;
    const float pulse = 0.96f + 0.04f * (phase < 10.0f ? phase / 10.0f : (20.0f - phase) / 10.0f);
    constexpr u64 tags[] = {MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null')};
    // Use rendered bounds, not the old unscaled icon group. This also cancels
    // the host's widescreen cursor expansion without changing other menus.
    for (int corner = 0; corner < 4; ++corner) {
        const float x = (corner < 2 ? -halfWidth : halfWidth) * pulse;
        const float y = (corner % 2 == 0 ? -halfHeight : halfHeight) * pulse;
        cursor->field_0x94[corner] = x;
        cursor->field_0xa4[corner] = y;
        cursor->field_0x74[corner] = x;
        cursor->field_0x84[corner] = y;
        if (auto* pane = cursor->mpScreen->search(tags[corner])) cursor->moveCenter(pane, x, y);
    }
}

HookAction before_collection_model_move(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect3D_c*>(args, 0);
    s_collectionModelScope = s_collectionScreen.root != nullptr && menu != nullptr &&
        menu->mpCollect2D == s_activeCollectMenu ? menu : nullptr;
    return HOOK_CONTINUE;
}

void after_collection_model_move(ModContext*, void*, void*, void*) {
    s_collectionModelScope = nullptr;
}

HookAction before_collection_link_pose(ModContext*, void* args, void*, void*) {
    if (s_collectionModelScope == nullptr) return HOOK_CONTINUE;
    const auto viewport = current_collection_viewport();
    // Change this menu's camera-space position, not the actor's model scale.
    // The regular status-window code retains equipment, animation and rotation.
    const float depth = (s_collectionModelScope->mIsWolf ? 450.0f : 400.0f) / viewport.scale;
    dMenu_Collect3D_c::toItem3Dpos(viewport.left + 410.0f * viewport.scale,
        viewport.top + 405.0f * viewport.scale, depth, &s_collectionModelPosition);
    mods::arg_ref<const cXyz*>(args, 1) = &s_collectionModelPosition;
    // Offset the rendered baseline without changing native rotation state.
    // This preserves stick movement and the rotation-disabled return angle.
    mods::arg_ref<s16>(args, 2) = collection_model_angle(
        mods::arg<s16>(args, 2), s_collectionModelScope->mIsWolf != 0);
    return HOOK_CONTINUE;
}

HookAction before_collection_projection(ModContext*, void*, void*, void*) {
    if (s_collectionScreen.root != nullptr && s_activeCollectMenu != nullptr) {
        // setupItem3D resets this back to its normal -100 immediately after
        // consuming it. No change leaks into gameplay or the title screen.
        dMenu_Collect3D_c::setViewPortOffsetY(0.0f);
    }
    return HOOK_CONTINUE;
}

HookAction before_collection_position(ModContext*, void* args, void*, void*) {
    if (s_collectionModelScope != nullptr) {
        mods::arg_ref<f32>(args, 1) = collection_projection_y(mods::arg<f32>(args, 1));
    }
    return HOOK_CONTINUE;
}
