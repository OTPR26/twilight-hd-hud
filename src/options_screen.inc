void hide_option_row_residue(J2DPane* pane, J2DPicture* keep) {
    if (pane == nullptr) {
        return;
    }
    if ((as_picture(pane) != nullptr || pane->getTypeID() == 17) && pane != keep) {
        pane->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_option_row_residue(child, keep);
    }
}

J2DPane* ensure_option_row_overlay(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpShadowScreen == nullptr) {
        return nullptr;
    }
    J2DPane* group = menu->mpShadowScreen->search(MULTI_CHAR('hd_orws'));
    if (group != nullptr) {
        return group;
    }

    group = JKR_NEW J2DPane(MULTI_CHAR('hd_orws'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    menu->mpShadowScreen->appendChild(group);
    constexpr f32 rowY[] = {118.0f, 171.0f, 224.0f};
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_orf0'), MULTI_CHAR('hd_orf1'), MULTI_CHAR('hd_orf2'),
    };
    constexpr u64 labelTags[] = {
        MULTI_CHAR('hd_orl0'), MULTI_CHAR('hd_orl1'), MULTI_CHAR('hd_orl2'),
    };
    constexpr u64 valueTags[] = {
        MULTI_CHAR('hd_orv0'), MULTI_CHAR('hd_orv1'), MULTI_CHAR('hd_orv2'),
    };
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    for (std::size_t index = 0; index < 3; ++index) {
        if (frameTexture != nullptr) {
            auto* frame = JKR_NEW J2DPicture(frameTags[index],
                JGeometry::TBox2<f32>(271.0f, rowY[index] - 16.0f,
                    507.0f, rowY[index] + 16.0f), frameTexture, nullptr);
            configure_hd_picture(frame);
            group->appendChild(frame);
        }

        auto* label = JKR_NEW J2DTextBox(labelTags[index],
            JGeometry::TBox2<f32>(112.0f, rowY[index] - 15.0f,
                250.0f, rowY[index] + 15.0f), nullptr, "", 48,
            HBIND_LEFT, VBIND_CENTER);
        label->setFont(menu->mpFont);
        label->setFontSize(16.0f, 16.0f);
        label->setFontColor(JUtility::TColor(238, 238, 232, 255),
            JUtility::TColor(255, 255, 255, 255));
        group->appendChild(label);

        auto* value = JKR_NEW J2DTextBox(valueTags[index],
            JGeometry::TBox2<f32>(282.0f, rowY[index] - 15.0f,
                496.0f, rowY[index] + 15.0f), nullptr, "", 48,
            HBIND_CENTER, VBIND_CENTER);
        value->setFont(menu->mpFont);
        value->setFontSize(16.0f, 16.0f);
        value->setFontColor(JUtility::TColor(244, 244, 238, 255),
            JUtility::TColor(255, 255, 255, 255));
        group->appendChild(value);
    }
    return group;
}

void update_option_row_overlay(dMenu_Option_c* menu, J2DPane* group) {
    if (menu == nullptr || group == nullptr) {
        return;
    }
    constexpr u64 labelTags[] = {
        MULTI_CHAR('hd_orl0'), MULTI_CHAR('hd_orl1'), MULTI_CHAR('hd_orl2'),
    };
    constexpr u64 valueTags[] = {
        MULTI_CHAR('hd_orv0'), MULTI_CHAR('hd_orv1'), MULTI_CHAR('hd_orv2'),
    };
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_orf0'), MULTI_CHAR('hd_orf1'), MULTI_CHAR('hd_orf2'),
    };
    constexpr const char* fallbackLabels[] = {
        "Lock-on Type", "Rumble Feature", "Sound",
    };

    for (std::size_t index = 0; index < 3; ++index) {
        auto* label = static_cast<J2DTextBox*>(group->search(labelTags[index]));
        const char* nativeLabel = text_box_string(menu->field_0x21c[index][0]);
        label->setString(48, nativeLabel != nullptr && nativeLabel[0] != '\0' ?
            nativeLabel : fallbackLabels[index]);

        const char* nativeValue = nullptr;
        for (CPaneMgr* valuePane : menu->mpMenuText[index]) {
            auto* text = valuePane != nullptr ?
                static_cast<J2DTextBox*>(valuePane->getPanePtr()) : nullptr;
            const char* candidate = text_box_string(text);
            if (candidate != nullptr && candidate[0] != '\0') {
                nativeValue = candidate;
                break;
            }
        }
        auto* value = static_cast<J2DTextBox*>(group->search(valueTags[index]));
        value->setString(48, nativeValue != nullptr ? nativeValue : "");

        auto* frame = as_picture(group->search(frameTags[index]));
        if (frame != nullptr) {
            frame->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                index == menu->getSelectType() ? JUtility::TColor(72, 69, 57, 255) :
                    JUtility::TColor(255, 255, 255, 255));
        }
    }
}

void add_option_background_and_title(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpBackScreen == nullptr) {
        return;
    }

    J2DPane* root = menu->mpBackScreen;
    if (root->search(MULTI_CHAR('hd_obg')) != nullptr) {
        return;
    }

    // The stock layout builds the Options screen as a stone window. Clear its
    // pictures and authored labels, then reuse the same restrained background
    // and rules as File Selection and Save.
    hide_option_native_art(root);
    ResTIMG const* backgroundTexture = resource_texture(s_fileSelectBackgroundResource);
    if (backgroundTexture != nullptr) {
        auto* background = JKR_NEW J2DPicture(MULTI_CHAR('hd_obg'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f),
            backgroundTexture, nullptr);
        configure_hd_picture(background);
        root->appendChild(background);
    }

    ResTIMG const* rulesTexture = resource_texture(s_fileSelectTitleRulesResource);
    if (rulesTexture != nullptr) {
        auto* upperRules = JKR_NEW J2DPicture(MULTI_CHAR('hd_otr1'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 56.0f), rulesTexture, nullptr);
        configure_hd_picture(upperRules);
        root->appendChild(upperRules);

        auto* lowerRules = JKR_NEW J2DPicture(MULTI_CHAR('hd_otr2'),
            JGeometry::TBox2<f32>(0.0f, 392.0f, 608.0f, 448.0f), rulesTexture, nullptr);
        configure_hd_picture(lowerRules);
        root->appendChild(lowerRules);
    }

    auto* titleGroup = JKR_NEW J2DPane(MULTI_CHAR('hd_otit'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    root->appendChild(titleGroup);
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (frameTexture != nullptr) {
        auto* titleFrame = JKR_NEW J2DPicture(MULTI_CHAR('hd_otfr'),
            JGeometry::TBox2<f32>(35.0f, 17.0f, 211.0f, 50.0f), frameTexture, nullptr);
        configure_hd_picture(titleFrame);
        titleGroup->appendChild(titleFrame);
    }

    auto* title = JKR_NEW J2DTextBox(MULTI_CHAR('hd_ottx'),
        JGeometry::TBox2<f32>(48.0f, 18.0f, 198.0f, 49.0f), nullptr,
        localized_label(MenuLabel::Options).c_str(), 512, HBIND_CENTER, VBIND_CENTER);
    title->setFont(menu->mpFont);
    title->setFontSize(21.0f, 21.0f);
    title->setCharSpace(-0.5f);
    fit_localized_label(title, 21.0f);
    title->setFontColor(JUtility::TColor(242, 242, 235, 255),
        JUtility::TColor(255, 255, 255, 255));
    titleGroup->appendChild(title);
}

void update_option_widescreen_canvas(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpBackScreen == nullptr) {
        return;
    }

    const f32 left = mDoGph_gInf_c::getMinXF();
    const f32 top = mDoGph_gInf_c::getMinYF();
    const f32 width = mDoGph_gInf_c::getWidthF();
    const f32 height = mDoGph_gInf_c::getHeightF();
    if (J2DPane* background = menu->mpBackScreen->search(MULTI_CHAR('hd_obg'))) {
        // Cover the active viewport uniformly. The source painting is 4:3;
        // cropping its top and bottom in widescreen preserves round motifs
        // instead of stretching them into ovals.
        f32 canvasWidth = width;
        f32 canvasHeight = canvasWidth * (448.0f / 608.0f);
        if (canvasHeight < height) {
            canvasHeight = height;
            canvasWidth = canvasHeight * (608.0f / 448.0f);
        }
        background->resize(canvasWidth, canvasHeight);
        background->move(left - (canvasWidth - width) * 0.5f,
            top - (canvasHeight - height) * 0.5f);
    }
    if (J2DPane* upperRules = menu->mpBackScreen->search(MULTI_CHAR('hd_otr1'))) {
        upperRules->resize(width, 56.0f);
        upperRules->move(left, top);
    }
    if (J2DPane* lowerRules = menu->mpBackScreen->search(MULTI_CHAR('hd_otr2'))) {
        lowerRules->resize(width, 56.0f);
        lowerRules->move(left, top + height - 56.0f);
    }
}

void add_option_prompts(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpScreenIcon == nullptr) {
        return;
    }
    if (menu->mpScreenIcon->search(MULTI_CHAR('hd_oprm')) != nullptr) {
        update_menu_face_button(menu->mpScreenIcon, MULTI_CHAR('hd_oapi'), true);
        update_menu_face_button(menu->mpScreenIcon, MULTI_CHAR('hd_obpi'), false);
        set_menu_face_button_texture(menu->mpScreenIcon,
            MULTI_CHAR('hd_orbt'), styled_zr_button_texture());
        return;
    }

    // Replace the oversized GameCube prompt cluster with the compact pairing
    // used elsewhere in the HD-styled menus.
    for (J2DPane* child = menu->mpScreenIcon->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        child->hide();
    }
    auto* group = JKR_NEW J2DPane(MULTI_CHAR('hd_oprm'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    menu->mpScreenIcon->appendChild(group);

    auto addPicture = [group](u64 tag, const JGeometry::TBox2<f32>& bounds,
                          ResTIMG const* texture, u8 alpha = 255) {
        if (texture == nullptr) {
            return;
        }
        auto* picture = JKR_NEW J2DPicture(tag, bounds, texture, nullptr);
        configure_hd_picture(picture);
        picture->setAlpha(alpha);
        group->appendChild(picture);
    };

    // Reuse the working Quest Log / Save pairing as a complete cluster.
    // Moving only the discs above the flourish exposes its middle loops.
    constexpr f32 right = 608.0f;
    addPicture(MULTI_CHAR('hd_oflr'),
        JGeometry::TBox2<f32>(right - 74.0f, 18.0f,
            right - 2.0f, 84.0f),
        resource_texture(s_fileSelectPromptFlourishResource), 220);

    auto* confirm = make_hd_prompt_label(MULTI_CHAR('hd_ocfm'),
        JGeometry::TBox2<f32>(right - 114.0f, 30.0f,
            right - 30.0f, 51.0f), "Confirm", HBIND_RIGHT);
    group->appendChild(confirm);

    addPicture(MULTI_CHAR('hd_oapi'),
        JGeometry::TBox2<f32>(right - 50.0f, 28.0f,
            right - 23.0f, 55.0f),
        menu_face_button_texture(true));
    group->appendChild(make_hd_prompt_label(MULTI_CHAR('hd_obck'),
        JGeometry::TBox2<f32>(right - 109.0f, 49.0f,
            right - 53.0f, 70.0f), "Back"));
    addPicture(MULTI_CHAR('hd_obpi'),
        JGeometry::TBox2<f32>(right - 62.0f, 49.0f,
            right - 35.0f, 76.0f),
        menu_face_button_texture(false));

    auto* displayPrompt = JKR_NEW J2DTextBox(MULTI_CHAR('hd_odsp'),
        JGeometry::TBox2<f32>(403.0f, 412.0f, 570.0f, 435.0f), nullptr,
        localized_label(MenuLabel::Brightness).c_str(), 512, HBIND_RIGHT, VBIND_CENTER);
    displayPrompt->setFont(menu->mpFont);
    displayPrompt->setFontSize(11.5f, 11.5f);
    fit_localized_label(displayPrompt, 11.5f);
    displayPrompt->setFontColor(JUtility::TColor(205, 201, 176, 255),
        JUtility::TColor(244, 238, 191, 255));
    group->appendChild(displayPrompt);
    addPicture(MULTI_CHAR('hd_orbt'),
        JGeometry::TBox2<f32>(575.0f, 410.5f, 601.0f, 436.5f),
        styled_zr_button_texture());
}

bool option_confirmation_active(const dMenu_Option_c* menu) {
    // Native process enums include Ruby, but non-Japanese selection indices
    // omit it. The warning's actual opening/closing frames are region-neutral.
    return menu != nullptr && menu->mpWarning != nullptr &&
        (menu->mpWarning->field_0x28 > 2849.0f ||
         menu->mpWarning->field_0x2c > 2849);
}

bool fit_option_warning_frame(J2DPicture* frame, J2DTextBox* text, J2DPane* title) {
    const char* message = text_box_string(text);
    JUTFont* font = text != nullptr ? text->getFont() : nullptr;
    if (frame == nullptr || font == nullptr || message == nullptr ||
        message[0] == '\0' || font->getCellHeight() <= 0 || text->getWidth() < 1.0f ||
        title == nullptr || frame->getParentPane() == nullptr ||
        text->getParentPane() == nullptr)
    {
        return false;
    }

    // Measure with the native parser (without drawing), including localized
    // multibyte characters, control codes, and wrapping. Replacement fonts
    // retain these native advances, so this fits all four font options.
    J2DTextBox::TFontSize size;
    text->getFontSize(size);
    J2DPrint print(font, text->getCharSpace(), text->getLineSpace(),
        JUtility::TColor(255, 255, 255, 255),
        JUtility::TColor(255, 255, 255, 255),
        JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    print.setFontSize(size.mSizeX, size.mSizeY);
    print.initchar();
    print.locate(0.0f, 0.0f);
    J2DPrint::TSize measured = {};
    const f32 belowBaseline = print.parse(
        reinterpret_cast<const u8*>(message), std::strlen(message),
        static_cast<int>(text->getWidth()), nullptr, measured, 255, false);
    const f32 textHeight = belowBaseline +
        font->getAscent() * (size.mSizeY / font->getCellHeight());
    CPaneMgr manager;
    Mtx titleMatrix, textMatrix, frameMatrix;
    const Vec titleTop = manager.getGlobalVtx(title, &titleMatrix, 0, false, 0);
    const Vec titleBottom = manager.getGlobalVtx(title, &titleMatrix, 3, false, 0);
    manager.getGlobalVtx(text, &textMatrix, 0, false, 0);
    manager.getGlobalVtx(frame, &frameMatrix, 0, false, 0);
    const f32 frameScaleX = std::fabs(frameMatrix[0][0]);
    const f32 frameScaleY = std::fabs(frameMatrix[1][1]);
    const f32 textScaleX = std::fabs(textMatrix[0][0]);
    const f32 textScaleY = std::fabs(textMatrix[1][1]);
    if (!std::isfinite(frameScaleX) || !std::isfinite(frameScaleY) ||
        !std::isfinite(textScaleX) || !std::isfinite(textScaleY) ||
        frameScaleX < 0.001f || frameScaleY < 0.001f ||
        textScaleX < 0.001f || textScaleY < 0.001f) return false;

    // Replace the Options heading at its live top-left, rather than leaving
    // the warning at the old stone-window title position below it. Fit in
    // screen space because the warning and title have different parents.
    if (titleBottom.x <= titleTop.x || titleBottom.y <= titleTop.y) return false;
    const auto fitted = option_warning_size(titleBottom.x - titleTop.x,
        titleBottom.y - titleTop.y,
        std::ceil(measured.field_0x0) * textScaleX,
        std::ceil(std::max(size.mSizeY, textHeight)) * textScaleY,
        textScaleX, textScaleY);
    const f32 width = fitted.width;
    const f32 height = fitted.height;
    const f32 centerX = titleTop.x + width * 0.5f;
    const f32 centerY = titleTop.y + height * 0.5f;
    if (!std::isfinite(width) || !std::isfinite(height) ||
        !std::isfinite(centerX) || !std::isfinite(centerY)) return false;
    frame->resize(width / frameScaleX, height / frameScaleY);
    position_dmap_global_center(frame, centerX, centerY);
    text->mFlags = static_cast<u8>((text->mFlags & ~0x0f) |
        (HBIND_CENTER << 2) | VBIND_CENTER);
    text->field_0x10c = 0.0f;
    text->field_0x110 = 0.0f;
    position_dmap_global_center(text, centerX, centerY);
    return true;
}

void style_option_confirmation(dMenu_Option_c* menu) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (menu == nullptr || frameTexture == nullptr) {
        return;
    }

    J2DPane* titleGroup = menu->mpBackScreen != nullptr ?
        menu->mpBackScreen->search(MULTI_CHAR('hd_otit')) : nullptr;
    J2DPane* titleFrame = titleGroup != nullptr ?
        titleGroup->search(MULTI_CHAR('hd_otfr')) : nullptr;
    bool warningReady = false;

    // Save and discard questions share this warning instance. Preserve the
    // localized message, native visibility, and confirmation state machine.
    if (dFile_warning_c* warning = menu->mpWarning;
        warning != nullptr && warning->mpRootPane != nullptr)
    {
        J2DPane* root = warning->mpRootPane->getPanePtr();
        auto* frame = as_picture(root->search(MULTI_CHAR('hd_owfr')));
        if (frame == nullptr) {
            J2DPicture* nativeBackground = nullptr;
            f32 largestArea = 0.0f;
            find_largest_picture(root, nativeBackground, largestArea);
            if (nativeBackground != nullptr &&
                nativeBackground->getParentPane() != nullptr)
            {
                frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_owfr'),
                    nativeBackground->getBounds(), frameTexture, nullptr);
                configure_hd_picture(frame);
                nativeBackground->getParentPane()->insertChild(nativeBackground, frame);
            }
        }
        if (frame != nullptr) {
            hide_other_pictures(root, frame);
            if (J2DTextBox* text = warning->field_0x20) {
                text->setFontSize(18.0f, 18.0f);
                text->setCharSpace(0.0f);
                text->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    JUtility::TColor(255, 255, 255, 255));
                text->setFontColor(JUtility::TColor(242, 242, 235, 255),
                    JUtility::TColor(255, 255, 255, 255));
                warningReady = fit_option_warning_frame(frame, text, titleFrame);
            }
        }
    }

    // A fading/clipped warning must not expose a second title behind it.
    // Restore the complete heading immediately after confirmation closes.
    if (titleGroup != nullptr) {
        if (warningReady && option_confirmation_active(menu)) titleGroup->hide();
        else titleGroup->show();
    }

    // Match the compact gold-edged choices already used by Save/File Select.
    // Keep the original parent panes and label strings so mouse targets,
    // controller input, and save/discard semantics remain entirely native.
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_ono'), MULTI_CHAR('hd_oyes'),
    };
    for (std::size_t index = 0; index < 2; ++index) {
        J2DPane* group = menu->mpYesNoSelBase_c[index] != nullptr ?
            menu->mpYesNoSelBase_c[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }
        auto* frame = as_picture(group->search(frameTags[index]));
        if (frame == nullptr) {
            frame = JKR_NEW J2DPicture(frameTags[index],
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f),
                frameTexture, nullptr);
            configure_hd_picture(frame);
            group->insertChild(group->getFirstChildPane(), frame);
        }
        hide_other_pictures(group, frame);
        frame->resize(142.0f, 26.0f);
        frame->move((group->getWidth() - 142.0f) * 0.5f,
            (group->getHeight() - 26.0f) * 0.5f);
        const bool selected = menu->field_0x3f9 == index;
        frame->setBlackWhite(selected ?
                JUtility::TColor(67, 66, 60, 255) :
                JUtility::TColor(8, 8, 6, 255),
            selected ? JUtility::TColor(255, 247, 154, 255) :
                JUtility::TColor(205, 201, 116, 255));
        if (menu->mpYesNoTxt_c[index] != nullptr) {
            auto* text = static_cast<J2DTextBox*>(
                menu->mpYesNoTxt_c[index]->getPanePtr());
            text->setFontSize(16.0f, 16.0f);
            text->setCharSpace(0.0f);
            text->setWhite(selected ? JUtility::TColor(242, 242, 236, 255) :
                JUtility::TColor(155, 155, 150, 255));
        }
        if (selected && option_confirmation_active(menu)) {
            // Do not force cursor alpha: the native state machine controls
            // its fades during opening, selection changes, and closing.
            position_cursor_outside_frame(menu->mpDrawCursor, frame);
        }
    }
}

void style_option_rows(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpScreen == nullptr) {
        return;
    }

    constexpr f32 rowY[] = {118.0f, 171.0f, 224.0f};
    constexpr f32 labelX = 162.0f;
    constexpr f32 valueX = 389.0f;
    constexpr f32 frameWidth = 236.0f;
    constexpr f32 frameHeight = 32.0f;
    const u8 selected = menu->getSelectType();
    J2DPane* overlay = ensure_option_row_overlay(menu);

    for (std::size_t index = 0; index < 3; ++index) {
        J2DPicture* frame = menu->field_0x280[index];
        if (menu->mpMenuNull[index] != nullptr) {
            hide_option_row_residue(menu->mpMenuNull[index]->getPanePtr(), frame);
        }
        if (menu->mpMenuPane[index] != nullptr) {
            hide_option_row_residue(menu->mpMenuPane[index]->getPanePtr(), frame);
        }
        if (menu->mpMenuPaneC[index] != nullptr) {
            hide_option_row_residue(menu->mpMenuPaneC[index]->getPanePtr(), nullptr);
        }
        ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
        if (frame != nullptr && frameTexture != nullptr) {
            frame->hide();
        }

        for (J2DTextBox* label : menu->field_0x21c[index]) {
            if (label == nullptr) {
                continue;
            }
            label->resize(178.0f, 30.0f);
            move_file_select_pane_center(label, labelX, rowY[index]);
            label->setFontSize(17.0f, 17.0f);
            label->setCharSpace(-0.25f);
            label->setFontColor(JUtility::TColor(238, 238, 232, 255),
                JUtility::TColor(255, 255, 255, 255));
            label->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            label->hide();
        }
        if (J2DTextBox* labelShadow = menu->field_0x298[index]) {
            labelShadow->resize(178.0f, 30.0f);
            move_file_select_pane_center(labelShadow, labelX, rowY[index]);
            labelShadow->setFontSize(17.0f, 17.0f);
            labelShadow->setCharSpace(-0.25f);
            labelShadow->setFontColor(JUtility::TColor(238, 238, 232, 255),
                JUtility::TColor(255, 255, 255, 255));
            labelShadow->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            labelShadow->hide();
        }

        for (CPaneMgr* valueGroup : {menu->mpMenuPane2[index], menu->mpMenuPane3[index],
                 menu->mpMenuPane32[index]}) {
            if (valueGroup != nullptr) {
                hide_option_row_residue(valueGroup->getPanePtr(), nullptr);
                move_file_select_pane_center(valueGroup->getPanePtr(), valueX, rowY[index]);
            }
        }
        if (menu->mpMenuPaneC[index] != nullptr) {
            move_file_select_pane_center(menu->mpMenuPaneC[index]->getPanePtr(),
                valueX, rowY[index]);
        }

        for (std::size_t arrow = 0; arrow < 4; ++arrow) {
            J2DPane* pane = menu->field_0x2d0[index][arrow];
            if (pane != nullptr) {
                move_file_select_pane_center(pane,
                    arrow < 2 ? valueX + 134.0f : valueX - 134.0f, rowY[index]);
            }
        }
        for (CPaneMgr* valueText : menu->mpMenuText[index]) {
            auto* text = valueText != nullptr ?
                static_cast<J2DTextBox*>(valueText->getPanePtr()) : nullptr;
            if (text != nullptr) {
                text->setFontSize(16.0f, 16.0f);
                text->setFontColor(JUtility::TColor(244, 244, 238, 255),
                    JUtility::TColor(255, 255, 255, 255));
                text->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    JUtility::TColor(255, 255, 255, 255));
                text->hide();
            }
        }
    }

    update_option_row_overlay(menu, overlay);
    if (selected < 3 && overlay != nullptr && !option_confirmation_active(menu)) {
        constexpr u64 frameTags[] = {
            MULTI_CHAR('hd_orf0'), MULTI_CHAR('hd_orf1'), MULTI_CHAR('hd_orf2'),
        };
        position_cursor_outside_frame(menu->mpDrawCursor,
            overlay->search(frameTags[selected]), -6.0f, -3.0f);
    }
}

void replace_option_tv_brick_layer(J2DPane* pane, ResTIMG const* brickTexture,
    ResTIMG const* replacement) {
    if (pane == nullptr || brickTexture == nullptr || replacement == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == brickTexture) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        // The outer TV-settings surround is the only large use of this tiled
        // brick. Keep the small sample borders and calibration artwork intact.
        if (bounds.getWidth() * bounds.getHeight() > 50000.0f) {
            picture->changeTexture(replacement, 0);
            picture->resize(bounds.getWidth(), bounds.getHeight());
            picture->move(bounds.i.x, bounds.i.y);
            picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
            picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        replace_option_tv_brick_layer(child, brickTexture, replacement);
    }
}

void add_tv_settings_hd_overlay(J2DScreen* screen, JUTFont* font) {
    if (screen == nullptr || font == nullptr) {
        return;
    }
    if (screen->search(MULTI_CHAR('hd_tvov')) != nullptr) {
        update_menu_face_button(screen, MULTI_CHAR('hd_tvap'), true);
        return;
    }

    // Remove the oversized GameCube prompt groups. Their text is recreated
    // below with the same compact TPHD treatment as Options.
    for (u64 tag : {MULTI_CHAR('abtn_n'), MULTI_CHAR('gcabtn_n'),
             MULTI_CHAR('g_abtn_n')}) {
        if (J2DPane* pane = screen->search(tag)) {
            pane->hide();
        }
    }

    // The localized title text sits inside the old stone title plate. Hide
    // the complete localized title container so no second box remains behind
    // the replacement. Stop before reaching a full-screen ancestor.
    for (u64 tag : {MULTI_CHAR('t_t00'), MULTI_CHAR('f_t00')}) {
        J2DPane* titlePane = screen->search(tag);
        J2DPane* titleContainer = titlePane;
        while (titleContainer != nullptr && titleContainer->getParentPane() != nullptr) {
            J2DPane* parent = titleContainer->getParentPane();
            const auto& bounds = parent->getBounds();
            if (bounds.getHeight() > 100.0f || bounds.getWidth() > 560.0f) {
                break;
            }
            titleContainer = parent;
        }
        if (titleContainer != nullptr) {
            titleContainer->hide();
        }
    }

    auto* overlay = JKR_NEW J2DPane(MULTI_CHAR('hd_tvov'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    screen->appendChild(overlay);

    ResTIMG const* rulesTexture = resource_texture(s_fileSelectTitleRulesResource);
    if (rulesTexture != nullptr) {
        auto* upperRules = JKR_NEW J2DPicture(MULTI_CHAR('hd_tvr1'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 56.0f), rulesTexture, nullptr);
        configure_hd_picture(upperRules);
        overlay->appendChild(upperRules);
        auto* lowerRules = JKR_NEW J2DPicture(MULTI_CHAR('hd_tvr2'),
            JGeometry::TBox2<f32>(0.0f, 392.0f, 608.0f, 448.0f), rulesTexture, nullptr);
        configure_hd_picture(lowerRules);
        overlay->appendChild(lowerRules);
    }

    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (frameTexture != nullptr) {
        auto* titleFrame = JKR_NEW J2DPicture(MULTI_CHAR('hd_tvfr'),
            JGeometry::TBox2<f32>(35.0f, 17.0f, 211.0f, 50.0f), frameTexture, nullptr);
        configure_hd_picture(titleFrame);
        overlay->appendChild(titleFrame);
    }
    auto* title = JKR_NEW J2DTextBox(MULTI_CHAR('hd_tvtx'),
        JGeometry::TBox2<f32>(48.0f, 18.0f, 198.0f, 49.0f), nullptr,
        localized_label(MenuLabel::DeviceSettings).c_str(), 512, HBIND_CENTER, VBIND_CENTER);
    title->setFont(font);
    title->setFontSize(19.0f, 19.0f);
    fit_localized_label(title, 19.0f);
    title->setCharSpace(-0.5f);
    title->setFontColor(JUtility::TColor(242, 242, 235, 255),
        JUtility::TColor(255, 255, 255, 255));
    overlay->appendChild(title);

    auto* complete = JKR_NEW J2DTextBox(MULTI_CHAR('hd_tvcp'),
        JGeometry::TBox2<f32>(475.0f, 17.0f, 555.0f, 41.0f), nullptr,
        localized_label(MenuLabel::Complete).c_str(), 512, HBIND_RIGHT, VBIND_CENTER);
    complete->setFont(font);
    complete->setFontSize(14.0f, 14.0f);
    fit_localized_label(complete, 14.0f);
    complete->setFontColor(JUtility::TColor(235, 235, 230, 255),
        JUtility::TColor(255, 255, 255, 255));
    overlay->appendChild(complete);

    if (ResTIMG const* greyA = menu_face_button_texture(true)) {
        auto* button = JKR_NEW J2DPicture(MULTI_CHAR('hd_tvap'),
            JGeometry::TBox2<f32>(558.0f, 14.0f, 585.0f, 41.0f), greyA, nullptr);
        configure_hd_picture(button);
        overlay->appendChild(button);
    }
}

void hide_large_tv_settings_frame(J2DPane* pane, ResTIMG const* source) {
    if (pane == nullptr || source == nullptr) {
        return;
    }
    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == source) {
        const auto& bounds = picture->getBounds();
        const f32 width = bounds.getWidth();
        const f32 height = bounds.getHeight();
        // The outer surround, its four tiny end-caps, and the comparison
        // samples share this texture. Preserve only the sample-sized frames.
        const bool isComparisonFrame =
            width >= 30.0f && width <= 150.0f &&
            height >= 25.0f && height <= 120.0f;
        if (!isComparisonFrame) {
            picture->hide();
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_large_tv_settings_frame(child, source);
    }
}

void hide_tv_settings_edge_caps(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }
    if (J2DPicture* picture = as_picture(pane)) {
        const auto& bounds = picture->getGlbBounds();
        const f32 width = bounds.getWidth();
        const f32 height = bounds.getHeight();
        const f32 centerY = (bounds.i.y + bounds.f.y) * 0.5f;
        // These caps are mostly transparent, so their pane bounds are much
        // larger than the few gold pixels they display. Match their contact
        // with the old surround's side edges instead of their visible size.
        const bool touchesOldFrameEdge =
            bounds.i.x <= 50.0f || bounds.f.x >= 558.0f;
        const bool isEdgeCap = width <= 200.0f && height <= 160.0f &&
            touchesOldFrameEdge &&
            centerY >= 60.0f && centerY <= 388.0f;
        if (isEdgeCap) {
            picture->hide();
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_tv_settings_edge_caps(child);
    }
}

void add_tv_settings_crisp_frame(J2DScreen* screen) {
    if (screen == nullptr || screen->search(MULTI_CHAR('hd_tvbd')) != nullptr) {
        return;
    }
    ResTIMG const* rule = resource_texture(s_fileSelectTitleRulesResource);
    if (rule == nullptr) {
        return;
    }
    auto* frame = JKR_NEW J2DPane(MULTI_CHAR('hd_tvbd'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    screen->appendChild(frame);
    const auto addEdge = [frame, rule](u64 tag, const JGeometry::TBox2<f32>& bounds) {
        auto* edge = JKR_NEW J2DPicture(tag, bounds, rule, nullptr);
        configure_hd_picture(edge);
        frame->appendChild(edge);
    };
    addEdge(MULTI_CHAR('hd_tvbt'), JGeometry::TBox2<f32>(34.0f, 62.0f, 574.0f, 66.0f));
    addEdge(MULTI_CHAR('hd_tvbb'), JGeometry::TBox2<f32>(34.0f, 382.0f, 574.0f, 386.0f));
}

void update_tv_settings_wording(J2DScreen* screen) {
    if (screen == nullptr) {
        return;
    }
    for (u64 tag : {MULTI_CHAR('menu_t81'), MULTI_CHAR('menu_t4'),
             MULTI_CHAR('menu_t8s'), MULTI_CHAR('menu_t8')}) {
        if (auto* text = static_cast<J2DTextBox*>(screen->search(tag))) {
            text->setString(512, localized_label(MenuLabel::DisplayInstructions).c_str());
            fit_localized_label(text, 14.0f);
        }
    }
}

void finish_tv_settings_hd_style(J2DScreen* screen, JKRArchive* archive,
    JUTFont* font) {
    if (screen == nullptr || archive == nullptr) {
        return;
    }
    hide_large_tv_settings_frame(screen,
        static_cast<ResTIMG const*>(archive->getResource('TIMG', "tt_yakushima.bti")));
    hide_tv_settings_edge_caps(screen);
    for (const char* textureName : {"tt_gold_uzu_long2.bti",
             "tt_horiwaku_top_rr.bti", "tt_kazari_kani_00.bti"}) {
        hide_collect_decoration_texture(screen,
            static_cast<ResTIMG const*>(archive->getResource('TIMG', textureName)));
    }
    update_tv_settings_wording(screen);
    add_tv_settings_crisp_frame(screen);
    add_tv_settings_hd_overlay(screen, font);
}

void style_option_tv_screen(dMenu_Option_c* menu) {
    if (menu == nullptr || menu->mpTVScreen == nullptr || menu->mpArchive == nullptr) {
        return;
    }

    auto* brick = static_cast<ResTIMG const*>(
        menu->mpArchive->getResource('TIMG', "tt_block128_00.bti"));
    replace_option_tv_brick_layer(menu->mpTVScreen, brick,
        resource_texture(s_fileSelectBackgroundResource));
    finish_tv_settings_hd_style(menu->mpTVScreen, menu->mpArchive, menu->mpFont);
}

void apply_option_hd_style(dMenu_Option_c* menu) {
    // _draw is queued while optres.arc is still mounting. None of the screen
    // pointers are valid until the native _create call assigns mpArchive.
    if (menu == nullptr || menu->mpArchive == nullptr) {
        return;
    }
    add_option_background_and_title(menu);
    update_option_widescreen_canvas(menu);
    add_option_prompts(menu);
    style_option_rows(menu);
    style_option_confirmation(menu);
    style_option_tv_screen(menu);
}

void replace_brightness_check_brick(J2DPane* pane, ResTIMG const* brickTexture,
    ResTIMG const* replacement) {
    if (pane == nullptr || brickTexture == nullptr || replacement == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == brickTexture) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        picture->changeTexture(replacement, 0);
        picture->resize(bounds.getWidth(), bounds.getHeight());
        picture->move(bounds.i.x, bounds.i.y);
        picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
        picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        replace_brightness_check_brick(child, brickTexture, replacement);
    }
}

void style_brightness_check_screen(dBrightCheck_c* screen) {
    if (screen == nullptr || screen->mBrightCheck.Scr == nullptr ||
        screen->mArchive == nullptr) {
        return;
    }

    ResTIMG const* brick = static_cast<ResTIMG const*>(
        screen->mArchive->getResource('TIMG', "tt_block128_00.bti"));
    replace_brightness_check_brick(screen->mBrightCheck.Scr, brick,
        resource_texture(s_fileSelectBackgroundResource));
    finish_tv_settings_hd_style(screen->mBrightCheck.Scr, screen->mArchive,
        mDoExt_getMesgFont());
}
