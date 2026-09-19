ResTIMG const* file_select_archive_texture(dFile_select_c* menu,
    const char* textureName) {
    return menu != nullptr && menu->mpArchive != nullptr && textureName != nullptr ?
        static_cast<ResTIMG const*>(menu->mpArchive->getResource('TIMG', textureName)) : nullptr;
}

// File Selection and copy menus --------------------------------------------

void replace_file_select_prompt(CPaneMgrAlpha* paneManager,
    ResTIMG const* replacement, const u64 buttonTag) {
    J2DPane* group = paneManager != nullptr ? paneManager->getPanePtr() : nullptr;
    if (group == nullptr || replacement == nullptr) {
        return;
    }

    auto* button = static_cast<J2DPicture*>(group->searchUserInfo(buttonTag));
    if (button == nullptr) {
        f32 largestArea = 0.0f;
        find_largest_picture(group, button, largestArea);
        if (button == nullptr) {
            return;
        }
        // Mark the native disc once. Later layout changes can then update the
        // same picture without treating the Back label or flourish overlays
        // as candidate button art and without hiding those overlays again.
        button->setUserInfo(buttonTag);
        hide_other_pictures(group, button);
    }

    if (button->getTexture(0) == nullptr ||
        button->getTexture(0)->getTexInfo() != replacement) {
        const JGeometry::TBox2<f32> bounds = button->getBounds();
        button->changeTexture(replacement, 0);
        button->resize(bounds.getWidth(), bounds.getHeight());
        button->move(bounds.i.x, bounds.i.y);
        button->setTexCoord(button->getTexture(0), BIND15, MIRROR0, false);
        button->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        button->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    }
    button->show();
}

void style_file_select_prompt_text(dFile_select_c* menu) {
    if (menu == nullptr) {
        return;
    }

    auto* back = menu->mModoruTxtPane != nullptr ?
        static_cast<J2DTextBox*>(menu->mModoruTxtPane->getPanePtr()) : nullptr;
    auto* confirm = menu->mKetteiTxtPane != nullptr ?
        static_cast<J2DTextBox*>(menu->mKetteiTxtPane->getPanePtr()) : nullptr;

    // Keep the native buffer valid for the file-select state machine, but
    // leave it empty.  The custom Back picture below is parented to the B
    // prompt and is the only label that should ever be visible.
    if (back != nullptr) {
        back->setString(0x20, "");
        menu->mModoruStringPtr = back->getStringPtr();
        back->setAlpha(0);
        if (menu->mModoruTxtPane != nullptr) {
            menu->mModoruTxtPane->setAlpha(0);
        }
    }

    // The GameCube labels fill almost the entire footer. TPHD uses compact
    // prompt typography, leaving the button discs as the visual anchors.
    for (J2DTextBox* text : {back, confirm}) {
        if (text != nullptr) {
            text->setFontSize(13.5f, 13.5f);
            text->setFontColor(JUtility::TColor(235, 235, 230, 255),
                JUtility::TColor(255, 255, 255, 255));
        }
    }
}

void style_file_select_dynamic_text(dFile_select_c* menu) {
    if (menu == nullptr) {
        return;
    }

    // File Selection rewrites and fades the native Back pane as its state
    // machine advances. Suppress it every frame so entering a file submenu
    // cannot produce a second Back label beside the custom B-linked one.
    auto* back = menu->mModoruTxtPane != nullptr ?
        static_cast<J2DTextBox*>(menu->mModoruTxtPane->getPanePtr()) : nullptr;
    if (back != nullptr) {
        back->setAlpha(0);
        if (menu->mModoruTxtPane != nullptr) {
            menu->mModoruTxtPane->setAlpha(0);
        }
    }

    for (CPaneMgrAlpha* titlePane : menu->mHeaderTxtPane) {
        auto* title = titlePane != nullptr ?
            static_cast<J2DTextBox*>(titlePane->getPanePtr()) : nullptr;
        if (title == nullptr) {
            continue;
        }
        if (menu->fileSel.font[0] != nullptr) {
            // font[0] is the heavier sans face also used by the file names;
            // font[1] is the thin handwritten face visible in the last build.
            title->setFont(menu->fileSel.font[0]);
        }
        title->setFontSize(24.0f, 24.0f);
        title->setCharSpace(-0.5f);
        title->setFontColor(JUtility::TColor(234, 201, 55, 255),
            JUtility::TColor(255, 244, 120, 255));
    }
}

void add_file_select_title_rules(dFile_select_c* menu) {
    ResTIMG const* rules = resource_texture(s_fileSelectTitleRulesResource);
    if (menu == nullptr || menu->fileSel.Scr == nullptr || rules == nullptr) {
        return;
    }

    J2DPane* root = menu->fileSel.Scr;
    const JGeometry::TBox2<f32> rootBounds = root->getBounds();
    const f32 width = rootBounds.getWidth();
    constexpr f32 centerY = 52.0f;

    // A single transparent picture supplies only the softly faded upper and
    // lower rules. Keeping it independent of the archive's animated bands
    // prevents transition-strip copies from appearing during menu movement.
    auto* overlay = as_picture(root->search(MULTI_CHAR('hd_tline')));
    if (overlay == nullptr) {
        overlay = JKR_NEW J2DPicture(MULTI_CHAR('hd_tline'),
            JGeometry::TBox2<f32>(24.0f, centerY - 28.0f,
                width - 24.0f, centerY + 28.0f), rules, nullptr);
        overlay->setTexCoord(overlay->getTexture(0), BIND15, MIRROR0, false);
        overlay->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        overlay->setCornerColor(JUtility::TColor(255, 255, 255, 190));
        root->appendChild(overlay);
    }
    overlay->resize(width - 48.0f, 56.0f);
    overlay->move(24.0f, centerY - 28.0f);
    overlay->show();
}

void add_file_select_back_label(dFile_select_c* menu) {
    J2DPane* buttonPane = menu != nullptr && menu->mBbtnPane != nullptr ?
        menu->mBbtnPane->getPanePtr() : nullptr;
    if (buttonPane == nullptr ||
        buttonPane->search(MULTI_CHAR('hd_back')) != nullptr) {
        return;
    }

    // The label is a true child of B. It therefore inherits every move,
    // resize, aspect-ratio correction, and visibility animation of the
    // button instead of drifting in screen coordinates.
    auto* overlay = make_hd_prompt_label(MULTI_CHAR('hd_back'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), "Back");
    buttonPane->appendChild(overlay);
}

void add_file_select_prompt_flourish(dFile_select_c* menu) {
    ResTIMG const* flourish = resource_texture(s_fileSelectPromptFlourishResource);
    J2DPane* buttonPane = menu != nullptr && menu->mBbtnPane != nullptr ?
        menu->mBbtnPane->getPanePtr() : nullptr;
    if (buttonPane == nullptr ||
        flourish == nullptr ||
        buttonPane->search(MULTI_CHAR('hd_swrl')) != nullptr) {
        return;
    }

    // Match TPHD's thin, open-hook accent and keep it attached to the same B
    // prompt transform as the Back label. The texture deliberately contains
    // no closed circles or linked-ring knot.
    auto* overlay = JKR_NEW J2DPicture(MULTI_CHAR('hd_swrl'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), flourish, nullptr);
    overlay->setTexCoord(overlay->getTexture(0), BIND15, MIRROR0, false);
    overlay->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    overlay->setCornerColor(JUtility::TColor(255, 255, 255, 205));
    buttonPane->appendChild(overlay);
}

void position_file_select_prompt_overlays(dFile_select_c* menu) {
    J2DPane* buttonPane = menu != nullptr && menu->mBbtnPane != nullptr ?
        menu->mBbtnPane->getPanePtr() : nullptr;
    if (buttonPane == nullptr) {
        return;
    }

    J2DPane* label = buttonPane->search(MULTI_CHAR('hd_back'));
    J2DPane* flourish = buttonPane->search(MULTI_CHAR('hd_swrl'));
    const f32 buttonWidth = buttonPane->getWidth();
    const f32 buttonHeight = buttonPane->getHeight();

    // These are local B-pane coordinates. They remain fixed relative to the
    // button at every window size and through submenu prompt animations.
    if (label != nullptr) {
        label->resize(43.0f, 16.0f);
        // The label texture has transparent side bearings. Let its picture
        // overlap the button pane slightly so the visible word sits close to
        // the B disc, as it does in TPHD.
        label->move(-29.0f, (buttonHeight - 16.0f) * 0.5f);
    }

    if (flourish != nullptr) {
        // Preserve the taller three-curl silhouette used by the TPHD prompt.
        flourish->resize(30.0f, 32.0f);
        flourish->move(buttonWidth - 12.0f,
            (buttonHeight - 32.0f) * 0.5f - 2.0f);
    }
}

void add_file_select_fixed_prompts(dFile_select_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->fileSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }

    const auto rootBounds = screen->getBounds();
    const f32 right = rootBounds.f.x;
    auto* group = screen->search(MULTI_CHAR('hd_fprm'));
    if (group == nullptr) {
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_fprm'), rootBounds);
        screen->appendChild(group);

        auto addPicture = [group](u64 tag,
                              const JGeometry::TBox2<f32>& bounds,
                              ResTIMG const* texture, u8 alpha = 255) {
            if (texture == nullptr) {
                return;
            }
            auto* picture = JKR_NEW J2DPicture(tag, bounds, texture, nullptr);
            picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
            picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            picture->setCornerColor(JUtility::TColor(255, 255, 255, alpha));
            group->appendChild(picture);
        };

        // The flourish is behind both discs: TPHD hides the middle of each
        // loop beneath A and B so the visible line appears to wrap them.
        addPicture(MULTI_CHAR('hd_fflr'),
            JGeometry::TBox2<f32>(right - 74.0f, 18.0f,
                right - 2.0f, 84.0f),
            resource_texture(s_fileSelectPromptFlourishResource), 220);

        auto* confirm = make_hd_prompt_label(MULTI_CHAR('hd_fcon'),
            JGeometry::TBox2<f32>(right - 114.0f, 30.0f,
                right - 30.0f, 51.0f), "Confirm", HBIND_RIGHT);
        group->appendChild(confirm);

        addPicture(MULTI_CHAR('hd_fapi'),
            JGeometry::TBox2<f32>(right - 50.0f, 28.0f,
                right - 23.0f, 55.0f), menu_face_button_texture(true));
        group->appendChild(make_hd_prompt_label(MULTI_CHAR('hd_fbck'),
            JGeometry::TBox2<f32>(right - 109.0f, 49.0f,
                right - 53.0f, 70.0f), "Back"));
        addPicture(MULTI_CHAR('hd_fbpi'),
            JGeometry::TBox2<f32>(right - 62.0f, 49.0f,
                right - 35.0f, 76.0f), menu_face_button_texture(false));
    }

    update_menu_face_button(screen, MULTI_CHAR('hd_fapi'), true);
    update_menu_face_button(screen, MULTI_CHAR('hd_fbpi'), false);
    group->show();

    // The stock prompt groups are animated and can be hidden or translated by
    // the file-select state machine.  Suppress them completely; the fixed
    // canvas group above is deterministic at every desktop aspect ratio.
    for (CPaneMgrAlpha* pane : {
             menu->mKetteiTxtPane, menu->mAbtnPane,
             menu->mModoruTxtPane, menu->mBbtnPane,
         }) {
        if (pane != nullptr && pane->getPanePtr() != nullptr) {
            pane->getPanePtr()->hide();
        }
    }
}

f32 copy_title_text_width(JUTFont* font, const char* text, f32 size,
    f32 spacing);
J2DTextBox* copy_metadata_text(J2DPane* group, u64 tag,
    const JGeometry::TBox2<f32>& bounds, JUTFont* font,
    J2DTextBoxHBinding binding, f32 size,
    const JUtility::TColor& color);

void add_file_select_detail_frame(dFile_select_c* menu) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    J2DScreen* screen = menu != nullptr ? menu->mSelDt.ScrDt : nullptr;
    J2DPane* detail = screen != nullptr ?
        screen->search(MULTI_CHAR('d_win_n')) : nullptr;
    if (detail == nullptr || frameTexture == nullptr) {
        return;
    }

    // Retire the first-pass stretched panel if this function is reached on a
    // live reconstructed screen. A button texture cannot be scaled to the
    // presentation's aspect without turning its fill into heavy black rails.
    if (J2DPane* oldFrame = detail->search(MULTI_CHAR('hd_dtfr'))) {
        oldFrame->hide();
    }

    const f32 width = detail->getWidth();
    const f32 height = detail->getHeight();
    auto ensureRule = [&](u64 tag, const JGeometry::TBox2<f32>& bounds,
                          const JUtility::TColor& color, u8 alpha) {
        auto* rule = as_picture(detail->search(tag));
        if (rule == nullptr) {
            rule = JKR_NEW J2DPicture(tag, bounds, frameTexture, nullptr);
            rule->setTexCoord(rule->getTexture(0), BIND15, MIRROR0, false);
            detail->insertChild(detail->getFirstChildPane(), rule);
        }
        rule->resize(bounds.getWidth(), bounds.getHeight());
        rule->move(bounds.i.x, bounds.i.y);
        // Mapping both texture endpoints to one color makes each piece a
        // clean solid rule while retaining the resource's soft alpha edge.
        rule->setBlackWhite(color, color);
        rule->setCornerColor(JUtility::TColor(255, 255, 255, 255));
        rule->setAlpha(alpha);
        rule->show();
    };

    const JUtility::TColor gold(208, 203, 145, 255);
    const JUtility::TColor inner(145, 145, 132, 255);
    ensureRule(MULTI_CHAR('hd_dto1'), {-2.5f, -2.5f, width + 2.5f, -1.0f}, gold, 235);
    ensureRule(MULTI_CHAR('hd_dto2'), {-2.5f, height + 1.0f, width + 2.5f, height + 2.5f}, gold, 235);
    ensureRule(MULTI_CHAR('hd_dto3'), {-2.5f, -1.0f, -1.0f, height + 1.0f}, gold, 235);
    ensureRule(MULTI_CHAR('hd_dto4'), {width + 1.0f, -1.0f, width + 2.5f, height + 1.0f}, gold, 235);
    ensureRule(MULTI_CHAR('hd_dti1'), {-0.75f, -0.75f, width + 0.75f, 0.0f}, inner, 190);
    ensureRule(MULTI_CHAR('hd_dti2'), {-0.75f, height, width + 0.75f, height + 0.75f}, inner, 190);
    ensureRule(MULTI_CHAR('hd_dti3'), {-0.75f, 0.0f, 0.0f, height}, inner, 190);
    ensureRule(MULTI_CHAR('hd_dti4'), {width, 0.0f, width + 0.75f, height}, inner, 190);
}

void style_file_select_action_buttons(dFile_select_c* menu) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (menu == nullptr || frameTexture == nullptr) {
        return;
    }

    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_3m0'), MULTI_CHAR('hd_3m1'), MULTI_CHAR('hd_3m2'),
    };
    constexpr f32 targetWidth = 142.0f;
    constexpr f32 targetHeight = 26.0f;

    // The native procedure is authoritative. The reset artifacts previously
    // attributed to this screen were the separate w_abase/w_bbase prompt
    // windows and are now suppressed by exact tag in the main-screen cleanup.
    const bool actionMenuVisible =
        (menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT ||
            menu->mDataSelProc ==
                dFile_select_c::DATASELPROC_MENU_SELECT_MOVE_ANM);
    const bool launchingFromStart =
        menu->mDataSelProc == dFile_select_c::DATASELPROC_NEXT_MODE_WAIT &&
        menu->mIsSelectEnd && menu->mSelectMenuNum == 1;

    // Move the complete three-action window, including its clipping pane,
    // instead of pushing the individual choices outside that window. This
    // preserves the native reveal animation and keeps the lower row visible.
    static J2DPane* actionMenuPane = nullptr;
    static f32 actionMenuBaseX = 0.0f;
    static f32 actionMenuBaseY = 0.0f;
    if (menu->m3mMenuPane != nullptr) {
        if (actionMenuPane != menu->m3mMenuPane) {
            actionMenuPane = menu->m3mMenuPane;
            actionMenuBaseX = menu->m3mMenuPane->getTranslateX();
            actionMenuBaseY = menu->m3mMenuPane->getTranslateY();
        }
        if (actionMenuVisible) {
            menu->m3mMenuPane->show();
            menu->m3mMenuPane->setAlpha(255);
            menu->m3mMenuPane->translate(actionMenuBaseX,
                actionMenuBaseY + 22.0f);
        }
    }

    for (std::size_t index = 0; index < 3; ++index) {
        J2DPane* group = menu->m3mSelPane[index] != nullptr ?
            menu->m3mSelPane[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }

        CPaneMgr* textManager = menu->m3mSelTextPane[index];
        auto* text = textManager != nullptr ?
            static_cast<J2DTextBox*>(textManager->getPanePtr()) : nullptr;

        // Yes/No animations leave these managers at zero alpha. The native
        // procedure returning to MENU_SELECT is the authoritative signal to
        // restore the complete action row; consulting the stale label alpha
        // here makes the hidden state self-perpetuating after choosing No.
        const bool panelDrawable = actionMenuVisible;

        if (panelDrawable) {
            // Erase -> No restores MENU_SELECT and the cursor, but leaves the
            // three selection-group managers at zero alpha. A visible child
            // frame cannot draw through that stale ancestor state.
            menu->m3mSelPane[index]->show();
            menu->m3mSelPane[index]->setAlphaRate(1.0f);
            menu->m3mSelPane[index]->setAlpha(255);
            group->show();
            group->setAlpha(255);
        }

        auto* frame = as_picture(group->search(frameTags[index]));
        if (frame == nullptr) {
            frame = JKR_NEW J2DPicture(frameTags[index],
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f),
                frameTexture, nullptr);
            frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
            frame->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            group->insertChild(group->getFirstChildPane(), frame);
        }

        hide_other_pictures(group, frame);
        frame->resize(targetWidth, targetHeight);
        frame->move((group->getWidth() - targetWidth) * 0.5f,
            (group->getHeight() - targetHeight) * 0.5f);
        if (menu->mSelectMenuNum == index) {
            frame->setBlackWhite(JUtility::TColor(67, 66, 60, 255),
                JUtility::TColor(255, 247, 154, 255));
        } else {
            frame->setBlackWhite(JUtility::TColor(8, 8, 6, 255),
                JUtility::TColor(205, 201, 116, 255));
        }
        if (panelDrawable) {
            frame->setAlpha(255);
            frame->show();
        } else {
            frame->setAlpha(0);
            frame->hide();
            frame->resize(1.0f, 1.0f);
            frame->move(-1000.0f, -1000.0f);
        }

        if (text != nullptr) {
            // Start removes the action panels before switching to gameplay.
            // Match the cursor's existing NEXT_MODE_WAIT cutoff so these
            // labels cannot linger alone during that transition.
            if (panelDrawable && !launchingFromStart) {
                textManager->show();
                textManager->setAlphaRate(1.0f);
                textManager->setAlpha(255);
                text->show();
                text->setAlpha(255);
            } else {
                text->setAlpha(0);
            }
            text->setFontSize(16.0f, 16.0f);
            text->setCharSpace(0.0f);
            const JUtility::TColor textColor =
                menu->mSelectMenuNum == index ?
                    JUtility::TColor(242, 242, 236, 255) :
                    JUtility::TColor(155, 155, 150, 255);
            text->setWhite(textColor);

            // The TPHD font's lowercase p sits visibly above the other
            // glyphs when J2D lays out "Copy" as one string. Draw this one
            // label in three pieces so only p receives the small baseline
            // correction already used by the Copy destination title.
            const char* currentLabel = text_box_string(text);
            const bool isCopyLabel =
                text->search(MULTI_CHAR('hd_ac0')) != nullptr ||
                (currentLabel != nullptr && std::strcmp(currentLabel, "Copy") == 0);
            if (isCopyLabel) {
                text->setString("");
                constexpr f32 labelSize = 16.0f;
                constexpr f32 spacing = 0.0f;
                constexpr const char* pieces[] = {"Co", "p", "y"};
                constexpr u64 tags[] = {
                    MULTI_CHAR('hd_ac0'), MULTI_CHAR('hd_ac1'),
                    MULTI_CHAR('hd_ac2'),
                };
                JUTFont* font = menu->fileSel.font[0];
                const f32 totalWidth = copy_title_text_width(font, "Copy",
                    labelSize, spacing);
                f32 left = (text->getWidth() - totalWidth) * 0.5f;
                const f32 top = (text->getHeight() - labelSize) * 0.5f;
                for (std::size_t piece = 0; piece < 3; ++piece) {
                    const f32 pieceWidth = copy_title_text_width(font,
                        pieces[piece], labelSize, spacing);
                    auto* corrected = copy_metadata_text(text, tags[piece],
                        JGeometry::TBox2<f32>(left,
                            top,
                            left + pieceWidth + 2.0f,
                            top + labelSize),
                        font, HBIND_LEFT, labelSize, textColor);
                    if (corrected != nullptr) {
                        corrected->mFlags = static_cast<u8>(
                            (corrected->mFlags & ~0x03) | VBIND_TOP);
                        corrected->setString(pieces[piece]);
                    }
                    left += pieceWidth;
                }
            }
        }

    }
}

void find_file_select_prompt_panel(J2DPane* pane, const f32 targetX,
    const f32 targetY, J2DPicture*& result, f32& bestDistance) {
    if (pane == nullptr) {
        return;
    }
    if (J2DPicture* picture = as_picture(pane)) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        const f32 width = bounds.getWidth();
        const f32 height = bounds.getHeight();
        const JGeometry::TBox2<f32>& global = picture->getGlbBounds();
        const f32 centerX = (global.i.x + global.f.x) * 0.5f;
        const f32 centerY = (global.i.y + global.f.y) * 0.5f;
        const f32 distance = (centerX - targetX) * (centerX - targetX) +
            (centerY - targetY) * (centerY - targetY);
        if (width >= 100.0f && width <= 260.0f &&
            height >= 14.0f && height <= 70.0f && distance < bestDistance) {
            result = picture;
            bestDistance = distance;
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        find_file_select_prompt_panel(child, targetX, targetY, result, bestDistance);
    }
}

void clear_file_select_prompt_panel(CPaneMgrAlpha* textManager) {
    ResTIMG const* clear = resource_texture(s_fileSelectClearRowResource);
    J2DPane* textPane = textManager != nullptr ? textManager->getPanePtr() : nullptr;
    if (textPane == nullptr || clear == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32>& textBounds = textPane->getGlbBounds();
    const f32 targetX = (textBounds.i.x + textBounds.f.x) * 0.5f;
    const f32 targetY = (textBounds.i.y + textBounds.f.y) * 0.5f;
    J2DPicture* panel = nullptr;
    for (J2DPane* ancestor = textPane->getParentPane();
         ancestor != nullptr && panel == nullptr;
         ancestor = ancestor->getParentPane()) {
        f32 bestDistance = 1.0e30f;
        find_file_select_prompt_panel(ancestor, targetX, targetY, panel, bestDistance);
    }
    if (panel == nullptr) {
        return;
    }
    if (panel->getTexture(0) == nullptr ||
        panel->getTexture(0)->getTexInfo() != clear) {
        const JGeometry::TBox2<f32> bounds = panel->getBounds();
        panel->changeTexture(clear, 0);
        panel->resize(bounds.getWidth(), bounds.getHeight());
        panel->move(bounds.i.x, bounds.i.y);
        panel->setTexCoord(panel->getTexture(0), BIND15, MIRROR0, false);
        panel->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        panel->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    }
    panel->show();
}

void move_file_select_pane_center(J2DPane* pane, const f32 targetX,
    const f32 targetY) {
    if (pane == nullptr) {
        return;
    }
    const JGeometry::TBox2<f32>& bounds = pane->getGlbBounds();
    const f32 centerX = (bounds.i.x + bounds.f.x) * 0.5f;
    const f32 centerY = (bounds.i.y + bounds.f.y) * 0.5f;
    f32 parentScaleX = 1.0f;
    f32 parentScaleY = 1.0f;
    if (J2DPane* parent = pane->getParentPane()) {
        const JGeometry::TBox2<f32>& parentGlobal = parent->getGlbBounds();
        if (std::fabs(parent->getWidth()) > 0.001f) {
            parentScaleX = parentGlobal.getWidth() / parent->getWidth();
        }
        if (std::fabs(parent->getHeight()) > 0.001f) {
            parentScaleY = parentGlobal.getHeight() / parent->getHeight();
        }
    }
    // add() consumes parent-local coordinates, while getGlbBounds() and the
    // target are global layout coordinates. Converting the delta prevents an
    // over-correction/under-correction loop when a desktop window is resized.
    pane->add((targetX - centerX) / parentScaleX,
        (targetY - centerY) / parentScaleY);
}

void position_file_select_prompts(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }
    const JGeometry::TBox2<f32> rootBounds = menu->fileSel.Scr->getBounds();
    // The archive remains authored on a 608-unit 4:3 canvas. Follow the live
    // widescreen edge so the prompt cluster reaches TPHD's upper-right corner
    // and remains responsive to arbitrary desktop window widths.
#if TARGET_PC
    // Anchor to Dusklight's visible HUD-safe edge.  The full virtual canvas can
    // extend beyond the window at both narrow and very wide desktop aspects;
    // using it (or blending toward it) makes these panes disappear or jump
    // while the host window is resized.
    const f32 right = mDoGph_gInf_c::getSafeMaxXF();
#else
    const f32 right = mDoGph_gInf_c::getMaxXF();
#endif
    const f32 top = rootBounds.i.y;
    move_file_select_pane_center(
        menu->mKetteiTxtPane != nullptr ? menu->mKetteiTxtPane->getPanePtr() : nullptr,
        right - 66.0f, top + 28.0f);
    move_file_select_pane_center(
        menu->mAbtnPane != nullptr ? menu->mAbtnPane->getPanePtr() : nullptr,
        right - 30.0f, top + 28.0f);
    move_file_select_pane_center(
        menu->mModoruTxtPane != nullptr ? menu->mModoruTxtPane->getPanePtr() : nullptr,
        right - 67.0f, top + 46.0f);
    move_file_select_pane_center(
        menu->mBbtnPane != nullptr ? menu->mBbtnPane->getPanePtr() : nullptr,
        right - 47.0f, top + 46.0f);
    position_file_select_prompt_overlays(menu);
    add_file_select_fixed_prompts(menu);
}

void replace_large_file_select_backgrounds(J2DPane* pane,
    const ResTIMG* brick, const ResTIMG* replacement, J2DPicture* keep) {
    if (pane == nullptr || brick == nullptr || replacement == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == brick) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        // File Selection keeps multiple full-screen backgrounds positioned
        // outside the viewport for its transitions. Replace every one so the
        // brick texture cannot slide back into view with a state change.
        if (bounds.getWidth() >= 600.0f && bounds.getHeight() >= 400.0f) {
            if (picture == keep) {
                picture->changeTexture(replacement, 0);
                picture->resize(bounds.getWidth(), bounds.getHeight());
                picture->move(bounds.i.x, bounds.i.y);
                picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
                picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    JUtility::TColor(255, 255, 255, 255));
                picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
                picture->setAlpha(255);
                picture->show();
            } else {
                // Several independently positioned copies of the vanilla
                // brick field overlap during menu transitions. Mapping a
                // complete painted backdrop onto every copy exposes a narrow
                // repeated strip at the left edge. Keep one persistent base
                // canvas and suppress the redundant transition copies.
                picture->hide();
            }
        }
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        replace_large_file_select_backgrounds(child, brick, replacement, keep);
    }
}

void replace_file_select_background(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }

    ResTIMG const* replacement = resource_texture(s_fileSelectBackgroundResource);
    ResTIMG const* brick = file_select_archive_texture(menu, "tt_block128_00.bti");
    if (replacement == nullptr || brick == nullptr) {
        return;
    }

    J2DPane* root = menu->fileSel.Scr;
    J2DPicture* background = as_picture(
        root->search(MULTI_CHAR('hd_fsbg')));
    if (background == nullptr) {
        background = JKR_NEW J2DPicture(MULTI_CHAR('hd_fsbg'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f),
            replacement, nullptr);
        background->setTexCoord(background->getTexture(0), BIND15, MIRROR0,
            false);
        background->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        background->setCornerColor(JUtility::TColor(255, 255, 255, 255));
        background->setAlpha(255);
        if (J2DPane* first = root->getFirstChildPane()) {
            root->insertChild(first, background);
        } else {
            root->appendChild(background);
        }
    } else if (background->getTexture(0) == nullptr ||
        background->getTexture(0)->getTexInfo() != replacement) {
        background->changeTexture(replacement, 0);
    }
    background->show();

    // Every archive-authored background lives below additional animated
    // containers. Suppress all of them and use one direct child of the root;
    // only then can the root transform be inverted without inherited offsets
    // producing black strips or resize-dependent jumps.
    replace_large_file_select_backgrounds(root, brick, replacement, nullptr);

    // The native file-select root itself is translated and horizontally
    // scaled by Dusklight. Counter that live transform when sizing only the
    // backdrop, mapping it exactly to the full viewport. This avoids both
    // safe-area bars and the resize-dependent crop/jump caused by overscan.
    const f32 rootScaleX = std::fabs(root->getScaleX()) > 0.001f ?
        root->getScaleX() : 1.0f;
    const f32 rootScaleY = std::fabs(root->getScaleY()) > 0.001f ?
        root->getScaleY() : 1.0f;
    background->resize(mDoGph_gInf_c::getWidthF() / rootScaleX,
        mDoGph_gInf_c::getHeightF() / rootScaleY);
    background->move(
        (mDoGph_gInf_c::getMinXF() - root->getTranslateX()) / rootScaleX,
        (mDoGph_gInf_c::getMinYF() - root->getTranslateY()) / rootScaleY);
}

void find_picture_with_texture(J2DPane* pane, ResTIMG const* texture,
    J2DPicture*& result) {
    if (pane == nullptr || texture == nullptr || result != nullptr) {
        return;
    }
    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == texture) {
        result = picture;
        return;
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        find_picture_with_texture(child, texture, result);
    }
}

void simplify_file_select_numbers(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }

    constexpr const char* numberTextures[] = {
        "tt_1_metal_40x40.bti", "tt_2_metal_40x40.bti", "tt_3_metal_40x40.bti",
    };
    for (std::size_t index = 0; index < 3; ++index) {
        ResTIMG const* numberTexture =
            file_select_archive_texture(menu, numberTextures[index]);
        J2DPicture* number = nullptr;
        find_picture_with_texture(menu->fileSel.Scr, numberTexture, number);
        ResTIMG const* replacement =
            resource_texture(s_fileSelectNumberResources[index]);
        if (number != nullptr && replacement != nullptr) {
            // The book, numeral, and inner flare are baked into this single
            // image. Swap that image directly for a transparent gold glyph.
            const JGeometry::TBox2<f32> bounds = number->getBounds();
            number->changeTexture(replacement, 0);
            number->resize(bounds.getWidth(), bounds.getHeight());
            number->move(bounds.i.x, bounds.i.y);
            number->setTexCoord(number->getTexture(0), BIND15, MIRROR0, false);
            number->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            number->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            number->show();

            if (J2DPane* parent = number->getParentPane()) {
                hide_other_pictures(parent, number);
            }
        }
        // The pulsing book/plaque effect is a separate layer from the numeral
        // picture. Disable that layer once; its alpha animation may continue,
        // but visibility remains off and therefore cannot flicker.
        if (menu->mSelFilePane_Book_l[index] != nullptr) {
            menu->mSelFilePane_Book_l[index]->hide();
        }
    }
}

void simplify_file_select_rows(dFile_select_c* menu) {
    ResTIMG const* replacement = resource_texture(s_fileSelectRowResource);
    ResTIMG const* clear = resource_texture(s_fileSelectClearRowResource);
    if (menu == nullptr || menu->fileSel.Scr == nullptr || replacement == nullptr ||
        clear == nullptr) {
        return;
    }

    constexpr u64 rowBaseTags[] = {
        MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'), MULTI_CHAR('w_go_b02'),
    };
    for (u64 tag : rowBaseTags) {
        J2DPane* group = menu->fileSel.Scr->search(tag);
        J2DPicture* base = as_picture(group);
        if (base == nullptr) {
            f32 largestArea = 0.0f;
            find_largest_picture(group, base, largestArea);
        }
        if (base != nullptr) {
            apply_file_select_row_texture(base, replacement);
            base->show();
        }
    }

    // The names, hearts, and timestamps are drawn by embedded file-info
    // screens. Their patterned black interiors therefore are not descendants
    // of w_go_b00..02; replace the largest static picture in each data group.
    for (std::size_t index = 0; index < 3; ++index) {
        if (menu->mSelFileMoyoPane[index] != nullptr) {
            // This is the repeating ornamental fill. The independent Gold and
            // Gold2 layers continue to supply the selected-row cursor bloom.
            menu->mSelFileMoyoPane[index]->hide();
        }
        if (menu->mSelFileGoldPane[index] != nullptr) {
            menu->mSelFileGoldPane[index]->hide();
        }
        if (menu->mSelFileGold2Pane[index] != nullptr) {
            menu->mSelFileGold2Pane[index]->hide();
        }
        for (CPaneMgrAlpha* groupManager : {
                 menu->mFileInfoDatBasePane[index],
                 menu->mFileInfoNoDatBasePane[index],
             }) {
            J2DPane* group = groupManager != nullptr ? groupManager->getPanePtr() : nullptr;
            J2DPicture* base = nullptr;
            f32 largestArea = 0.0f;
            find_largest_picture(group, base, largestArea);
            if (base != nullptr) {
                apply_file_select_row_texture(base, clear);
                base->show();
            }
        }
    }
}

constexpr u64 kFileSelectHeartTags[] = {
    MULTI_CHAR('hear_20'), MULTI_CHAR('hear_21'),
    MULTI_CHAR('hear_22'), MULTI_CHAR('hear_23'),
    MULTI_CHAR('hear_24'), MULTI_CHAR('hear_25'),
    MULTI_CHAR('hear_26'), MULTI_CHAR('hear_27'),
    MULTI_CHAR('hear_28'), MULTI_CHAR('hear_29'),
    MULTI_CHAR('hear_30'), MULTI_CHAR('hear_31'),
    MULTI_CHAR('hear_32'), MULTI_CHAR('hear_33'),
    MULTI_CHAR('hear_34'), MULTI_CHAR('hear_35'),
    MULTI_CHAR('hear_36'), MULTI_CHAR('hear_37'),
    MULTI_CHAR('hear_38'), MULTI_CHAR('hear_39'),
};

bool file_info_has_second_heart_line(J2DPane* group) {
    if (group == nullptr) {
        return false;
    }
    for (std::size_t heart = 10; heart < std::size(kFileSelectHeartTags); ++heart) {
        J2DPane* pane = group->search(kFileSelectHeartTags[heart]);
        if (pane != nullptr && pane->isVisible()) {
            return true;
        }
    }
    return false;
}

struct FileSelectRowGeometry {
    J2DPicture* picture = nullptr;
    f32 left = 0.0f;
    f32 top = 0.0f;
    f32 width = 0.0f;
    f32 height = 0.0f;
};

void size_file_select_row_for_hearts(J2DPicture* row, const bool hasSecondLine,
    FileSelectRowGeometry& geometry) {
    if (row == nullptr) {
        return;
    }
    if (geometry.picture != row) {
        const JGeometry::TBox2<f32> bounds = row->getBounds();
        geometry = {
            .picture = row,
            .left = bounds.i.x,
            .top = bounds.i.y,
            .width = bounds.getWidth(),
            .height = bounds.getHeight(),
        };
    }

    // Set a stable local width and let the native parent animation scale the
    // card along with its contents. J2D's cached global bounds only add
    // translations, so their width/local-width ratio was not an inherited
    // scale; it fed stale resize values back into the next frame instead.
    constexpr f32 kFourThreeWidthInHeights = 0.93f;
    constexpr f32 kWideWidthInHeights = 1.03f;
#if TARGET_PC || defined(__ANDROID__)
    const f32 aspect = mDoGph_gInf_c::getHeight() > 0.001f ?
        mDoGph_gInf_c::getWidth() / mDoGph_gInf_c::getHeight() : 4.0f / 3.0f;
#else
    const f32 aspect = mDoGph_gInf_c::getHeightF() > 0.001f ?
        mDoGph_gInf_c::getWidthF() / mDoGph_gInf_c::getHeightF() : 4.0f / 3.0f;
#endif
    const f32 aspectBlend = std::clamp(
        (aspect - 4.0f / 3.0f) / (4.0f / 9.0f), 0.0f, 1.0f);
    const f32 widthInHeights = kFourThreeWidthInHeights +
        (kWideWidthInHeights - kFourThreeWidthInHeights) * aspectBlend;
#if TARGET_PC || defined(__ANDROID__)
    const f32 viewportHeight = static_cast<f32>(mDoGph_gInf_c::getHeight());
#else
    const f32 viewportHeight = mDoGph_gInf_c::getHeightF();
#endif
    const f32 targetWidth = viewportHeight * widthInHeights;
    const f32 extraHeight = hasSecondLine ? 24.0f : 8.0f;
    const f32 targetHeight = geometry.height + extraHeight;
    row->resize(targetWidth, targetHeight);
    row->move(geometry.left + (geometry.width - targetWidth) * 0.5f,
        geometry.top - extraHeight * 0.5f);
}

void update_file_select_row_shadow(J2DPicture* row, const bool selected) {
    ResTIMG const* texture = resource_texture(s_fileSelectRowShadowResource);
    if (row == nullptr || texture == nullptr) {
        return;
    }

    auto* shadow = as_picture(row->search(MULTI_CHAR('hd_rshd')));
    if (shadow == nullptr) {
        shadow = JKR_NEW J2DPicture(MULTI_CHAR('hd_rshd'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), texture, nullptr);
        shadow->setTexCoord(shadow->getTexture(0), BIND15, MIRROR0, false);
        shadow->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
            JUtility::TColor(255, 255, 255, 255));
        shadow->setCornerColor(JUtility::TColor(255, 255, 255, 255));
        row->appendChild(shadow);
    }

    // Keep the drop shadow outside the panel so neither metadata nor cursor
    // bounds need to include it.
    shadow->resize(row->getWidth() - 12.0f, 24.0f);
    shadow->move(6.0f, row->getHeight() - 1.0f);
    if (selected) {
        shadow->show();
    } else {
        shadow->hide();
    }
}

J2DPicture* copy_destination_row_picture(dFile_select_c* menu,
    std::size_t index) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr || index >= 2) {
        return nullptr;
    }
    constexpr u64 rowTags[] = {
        MULTI_CHAR('hd_cp0'), MULTI_CHAR('hd_cp1'),
    };
    if (auto* row = as_picture(screen->search(rowTags[index]))) {
        return row;
    }

    ResTIMG const* texture = resource_texture(s_fileSelectRowResource);
    if (texture == nullptr) {
        return nullptr;
    }
    auto* row = JKR_NEW J2DPicture(rowTags[index],
        JGeometry::TBox2<f32>(0.0f, 0.0f, 435.0f, 62.0f), texture, nullptr);
    row->setTexCoord(row->getTexture(0), BIND15, MIRROR0, false);
    row->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    // Keep replacement cards directly on the copy screen.  The stock
    // destination groups are continuously transformed by two BCK animations;
    // parenting the HD cards there made them visibly jump between the native
    // and corrected positions on alternating frames.
    screen->appendChild(row);
    return row;
}

J2DPicture* copy_destination_background(dFile_select_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr) {
        return nullptr;
    }
    if (auto* background =
            as_picture(screen->search(MULTI_CHAR('hd_cpbg')))) {
        return background;
    }
    ResTIMG const* texture = resource_texture(s_fileSelectBackgroundResource);
    if (texture == nullptr) {
        return nullptr;
    }
    auto* background = JKR_NEW J2DPicture(MULTI_CHAR('hd_cpbg'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f), texture, nullptr);
    background->setTexCoord(background->getTexture(0), BIND15, MIRROR0, false);
    background->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    screen->insertChild(screen->getFirstChildPane(), background);
    return background;
}

J2DPicture* copy_destination_number_picture(dFile_select_c* menu,
    std::size_t index, J2DPicture* row) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr || index >= 2 || row == nullptr) {
        return nullptr;
    }
    constexpr u64 tags[] = {MULTI_CHAR('hd_cn0'), MULTI_CHAR('hd_cn1')};
    auto* number = as_picture(screen->search(tags[index]));
    const u8 saveIndex = menu->getCptoNum(static_cast<u8>(index));
    ResTIMG const* texture = saveIndex < 3 ?
        resource_texture(s_fileSelectNumberResources[saveIndex]) : nullptr;
    if (number == nullptr && texture != nullptr) {
        number = JKR_NEW J2DPicture(tags[index],
            JGeometry::TBox2<f32>(0.0f, 0.0f, 22.0f, 22.0f),
            texture, nullptr);
        number->setTexCoord(number->getTexture(0), BIND15, MIRROR0, false);
        number->setCornerColor(JUtility::TColor(255, 255, 255, 255));
        screen->appendChild(number);
    }
    if (number != nullptr && texture != nullptr) {
        apply_file_select_row_texture(number, texture);
        const auto rowBounds = row->getBounds();
        number->resize(22.0f, 22.0f);
        number->move(rowBounds.i.x + 42.0f,
            rowBounds.i.y + (rowBounds.getHeight() - 22.0f) * 0.5f);
        number->show();
    }
    return number;
}

J2DPane* copy_destination_info_anchor(dFile_select_c* menu,
    std::size_t index, f32 rowY) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr || index >= 2) {
        return nullptr;
    }
    constexpr u64 tags[] = {MULTI_CHAR('hd_ca0'), MULTI_CHAR('hd_ca1')};
    J2DPane* anchor = screen->search(tags[index]);
    if (anchor == nullptr) {
        anchor = JKR_NEW J2DPane(tags[index],
            JGeometry::TBox2<f32>(67.0f, rowY - 47.5f,
                541.0f, rowY + 39.5f));
        screen->appendChild(anchor);
    }
    anchor->resize(474.0f, 87.0f);
    anchor->move(67.0f, rowY - 47.5f);
    anchor->show();
    if (menu->mCpFileInfo[index] != nullptr) {
        menu->mCpFileInfo[index]->setBasePane(anchor);
    }
    return anchor;
}

J2DTextBox* copy_metadata_text(J2DPane* group, u64 tag,
    const JGeometry::TBox2<f32>& bounds, JUTFont* font,
    J2DTextBoxHBinding binding, f32 size,
    const JUtility::TColor& color) {
    if (group == nullptr) {
        return nullptr;
    }
    auto* text = static_cast<J2DTextBox*>(group->search(tag));
    if (text == nullptr) {
        text = JKR_NEW J2DTextBox(tag, bounds, nullptr, "", 64,
            binding, VBIND_CENTER);
        text->setFont(font);
        group->appendChild(text);
    }
    text->resize(bounds.getWidth(), bounds.getHeight());
    text->move(bounds.i.x, bounds.i.y);
    text->setFontSize(size, size);
    text->setCharSpace(0.0f);
    text->setWhite(color);
    text->show();
    return text;
}

void style_copy_destination_metadata_overlay(dFile_select_c* menu,
    std::size_t index, f32 rowY, bool visible) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr || index >= 2 || menu->mCpFileInfo[index] == nullptr) {
        return;
    }
    constexpr u64 groupTags[] = {MULTI_CHAR('hd_cm0'), MULTI_CHAR('hd_cm1')};
    J2DPane* overlay = screen->search(groupTags[index]);
    if (overlay == nullptr) {
        overlay = JKR_NEW J2DPane(groupTags[index],
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
        screen->appendChild(overlay);
    }
    if (!visible) {
        overlay->hide();
        set_copy_file_info_visible(menu, index, false);
        return;
    }
    overlay->show();

    J2DPane* source = menu->mCpFileInfo[index]->getDatBase() != nullptr ?
        menu->mCpFileInfo[index]->getDatBase()->getPanePtr() : nullptr;
    if (source == nullptr) {
        return;
    }
    const bool hasSecondHeartLine = file_info_has_second_heart_line(source);
    JUTFont* font = menu->fileSel.font[0];
    auto sourceText = [source](u64 tag) {
        return static_cast<J2DTextBox*>(source->search(tag));
    };
    auto copyString = [](J2DTextBox* target, J2DTextBox* from,
        const char* fallback) {
        if (target == nullptr) {
            return;
        }
        const char* value = from != nullptr ? text_box_string(from) : fallback;
        target->setString(value != nullptr ? value : fallback);
    };

    const f32 left = (608.0f - 435.0f) * 0.5f;
    const f32 nameTop = hasSecondHeartLine ? rowY - 28.0f : rowY - 21.0f;
    const f32 nameBottom = hasSecondHeartLine ? rowY - 10.0f : rowY - 3.0f;
    const f32 saveTop = hasSecondHeartLine ? rowY + 1.0f : rowY - 5.0f;
    const f32 playTop = hasSecondHeartLine ? rowY + 16.0f : rowY + 10.0f;
    auto* name = copy_metadata_text(overlay, MULTI_CHAR('hd_mnam'),
        JGeometry::TBox2<f32>(left + 125.0f, nameTop,
            left + 235.0f, nameBottom), font, HBIND_RIGHT, 13.5f,
        JUtility::TColor(235, 232, 211, 255));
    auto* saveLabel = copy_metadata_text(overlay, MULTI_CHAR('hd_msav'),
        JGeometry::TBox2<f32>(left + 125.0f, saveTop,
            left + 235.0f, saveTop + 14.0f), font, HBIND_RIGHT, 11.5f,
        JUtility::TColor(190, 202, 255, 255));
    auto* playLabel = copy_metadata_text(overlay, MULTI_CHAR('hd_mply'),
        JGeometry::TBox2<f32>(left + 105.0f, playTop,
            left + 235.0f, playTop + 15.0f), font, HBIND_RIGHT, 11.5f,
        JUtility::TColor(188, 225, 112, 255));
    auto* saveValue = copy_metadata_text(overlay, MULTI_CHAR('hd_msva'),
        JGeometry::TBox2<f32>(left + 245.0f, saveTop,
            left + 420.0f, saveTop + 14.0f), font, HBIND_LEFT, 11.5f,
        JUtility::TColor(190, 202, 255, 255));
    auto* playValue = copy_metadata_text(overlay, MULTI_CHAR('hd_mpva'),
        JGeometry::TBox2<f32>(left + 245.0f, playTop,
            left + 420.0f, playTop + 15.0f), font, HBIND_LEFT, 11.5f,
        JUtility::TColor(188, 225, 112, 255));
    copyString(name, sourceText(MULTI_CHAR('f_name01')), "Link");
    copyString(saveLabel, sourceText(MULTI_CHAR('f_s_t_02')), "Save time");
    copyString(playLabel, sourceText(MULTI_CHAR('f_p_t_02')), "Total play time");
    copyString(saveValue, sourceText(MULTI_CHAR('w_time01')), "");
    copyString(playValue, sourceText(MULTI_CHAR('w_ptim01')), "");

    constexpr u64 heartTags[] = {
        MULTI_CHAR('hear_20'), MULTI_CHAR('hear_21'),
        MULTI_CHAR('hear_22'), MULTI_CHAR('hear_23'),
        MULTI_CHAR('hear_24'), MULTI_CHAR('hear_25'),
        MULTI_CHAR('hear_26'), MULTI_CHAR('hear_27'),
        MULTI_CHAR('hear_28'), MULTI_CHAR('hear_29'),
        MULTI_CHAR('hear_30'), MULTI_CHAR('hear_31'),
        MULTI_CHAR('hear_32'), MULTI_CHAR('hear_33'),
        MULTI_CHAR('hear_34'), MULTI_CHAR('hear_35'),
        MULTI_CHAR('hear_36'), MULTI_CHAR('hear_37'),
        MULTI_CHAR('hear_38'), MULTI_CHAR('hear_39'),
    };
    for (std::size_t heart = 0; heart < 20; ++heart) {
        auto* sourceHeart = as_picture(source->search(heartTags[heart]));
        auto* heartPicture = as_picture(overlay->search(heartTags[heart]));
        ResTIMG const* texture = sourceHeart != nullptr &&
                sourceHeart->getTexture(0) != nullptr ?
            sourceHeart->getTexture(0)->getTexInfo() : nullptr;
        if (heartPicture == nullptr && texture != nullptr) {
            heartPicture = JKR_NEW J2DPicture(heartTags[heart],
                JGeometry::TBox2<f32>(0.0f, 0.0f, 12.0f, 12.0f),
                texture, nullptr);
            heartPicture->setTexCoord(heartPicture->getTexture(0),
                BIND15, MIRROR0, false);
            overlay->appendChild(heartPicture);
        }
        if (heartPicture != nullptr) {
            const std::size_t column = heart % 10;
            const std::size_t line = heart / 10;
            heartPicture->resize(12.0f, 12.0f);
            const f32 heartTop = hasSecondHeartLine ? rowY - 25.0f : rowY - 17.0f;
            heartPicture->move(left + 245.0f + column * 15.0f,
                heartTop + line * 12.0f);
            if (sourceHeart != nullptr) {
                heartPicture->setBlackWhite(sourceHeart->getBlack(),
                    sourceHeart->getWhite());
                heartPicture->setCornerColor(
                    JUtility::TColor(255, 255, 255, 255));
            }
            if (sourceHeart != nullptr && sourceHeart->isVisible()) {
                heartPicture->show();
            } else {
                heartPicture->hide();
            }
        }
    }
    // The live strings remain game-owned, but their animated renderer stays
    // hidden; only the fixed overlay above is drawn.
    set_copy_file_info_visible(menu, index, false);
}

void hide_copy_destination_architecture(J2DPane* pane,
    J2DPicture* background, J2DPicture* row0, J2DPicture* row1) {
    if (pane == nullptr) {
        return;
    }
    if (pane->mInfoTag == MULTI_CHAR('hd_cm0') ||
        pane->mInfoTag == MULTI_CHAR('hd_cm1')) {
        return;
    }
    if (auto* picture = as_picture(pane);
        picture != nullptr && picture != background &&
        picture != row0 && picture != row1) {
        // The stock copy window is assembled from dozens of small stone
        // tiles, so size-based filtering leaves a complete second frame
        // around our rows.  This screen needs only the custom background and
        // cards; the native number pictures are explicitly restored below.
        picture->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_copy_destination_architecture(child, background, row0, row1);
    }
}

void simplify_copy_destination_numbers(dFile_select_c* menu) {
    if (menu == nullptr || menu->mCpSel.Scr == nullptr) {
        return;
    }
    constexpr const char* numberTextures[] = {
        "tt_1_metal_40x40.bti", "tt_2_metal_40x40.bti",
        "tt_3_metal_40x40.bti",
    };
    for (std::size_t index = 0; index < 3; ++index) {
        J2DPicture* number = nullptr;
        find_picture_with_texture(menu->mCpSel.Scr,
            file_select_archive_texture(menu, numberTextures[index]), number);
        ResTIMG const* replacement =
            resource_texture(s_fileSelectNumberResources[index]);
        if (number != nullptr && replacement != nullptr) {
            apply_file_select_row_texture(number, replacement);
            if (J2DPane* parent = number->getParentPane()) {
                hide_other_pictures(parent, number);
            }
        }
    }
}

void style_copy_destination_metadata(dFile_select_c* menu) {
    if (menu == nullptr || menu->mCpSel.Scr == nullptr ||
        menu->mCpSel.Scr->search(MULTI_CHAR('hd_cpmd')) != nullptr) {
        return;
    }
    constexpr u64 labelTags[] = {
        MULTI_CHAR('f_s_t_02'), MULTI_CHAR('f_p_t_02'),
    };
    constexpr u64 valueTags[] = {
        MULTI_CHAR('w_time01'), MULTI_CHAR('w_ptim01'),
    };
    for (std::size_t index = 0; index < 2; ++index) {
        J2DPane* group = menu->mCpFileInfo[index] != nullptr &&
                menu->mCpFileInfo[index]->getDatBase() != nullptr ?
            menu->mCpFileInfo[index]->getDatBase()->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }
        // File-info is drawn by a separate screen using the destination
        // anchor's matrix.  Its authored local positions already line up;
        // deriving offsets from global bounds here made those offsets capture
        // the copy transition's temporary off-screen position permanently.
        if (auto* name = static_cast<J2DTextBox*>(
                group->search(MULTI_CHAR('f_name01')))) {
            name->setFontSize(13.5f, 13.5f);
        }
        constexpr u64 heartTags[] = {
            MULTI_CHAR('hear_20'), MULTI_CHAR('hear_21'),
            MULTI_CHAR('hear_22'), MULTI_CHAR('hear_23'),
            MULTI_CHAR('hear_24'), MULTI_CHAR('hear_25'),
            MULTI_CHAR('hear_26'), MULTI_CHAR('hear_27'),
            MULTI_CHAR('hear_28'), MULTI_CHAR('hear_29'),
            MULTI_CHAR('hear_30'), MULTI_CHAR('hear_31'),
            MULTI_CHAR('hear_32'), MULTI_CHAR('hear_33'),
            MULTI_CHAR('hear_34'), MULTI_CHAR('hear_35'),
            MULTI_CHAR('hear_36'), MULTI_CHAR('hear_37'),
            MULTI_CHAR('hear_38'), MULTI_CHAR('hear_39'),
        };
        for (u64 tag : heartTags) {
            if (J2DPane* heartPane = group->search(tag)) {
                heartPane->add(0.0f, 5.0f);
            }
        }
        for (u64 tag : labelTags) {
            if (auto* text = static_cast<J2DTextBox*>(group->search(tag))) {
                text->setFontSize(11.5f, 11.5f);
            }
        }
        for (u64 tag : valueTags) {
            if (auto* text = static_cast<J2DTextBox*>(group->search(tag))) {
                text->setFontSize(11.5f, 11.5f);
                text->mFlags = static_cast<u8>((text->mFlags & ~0x0c) |
                    (HBIND_LEFT << 2));
                if (tag == MULTI_CHAR('w_time01')) {
                    text->setCharSpace(-0.25f);
                }
                J2DPane* firstHeart = group->search(MULTI_CHAR('hear_20'));
                if (firstHeart != nullptr) {
                    const f32 heartLeft = firstHeart->getGlbBounds().i.x;
                    const f32 valueLeft = text->getGlbBounds().i.x;
                    if (std::isfinite(heartLeft) && std::isfinite(valueLeft) &&
                        heartLeft > -1000.0f && heartLeft < 1000.0f &&
                        valueLeft > -1000.0f && valueLeft < 1000.0f) {
                        text->add(heartLeft + 5.0f - valueLeft, 0.0f);
                    }
                }
            }
        }
        hide_collect_decoration_texture(group,
            file_select_archive_texture(menu, "tt_line2.bti"));
    }
    menu->mCpSel.Scr->appendChild(JKR_NEW J2DPane(MULTI_CHAR('hd_cpmd'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f)));
}

bool copy_destination_list_state(const dFile_select_c* menu) {
    return menu != nullptr &&
        (menu->mDataSelProc == dFile_select_c::DATASELPROC_COPY_DATA_TO_SELECT ||
            menu->mDataSelProc ==
                dFile_select_c::DATASELPROC_COPY_DATA_TO_SELECT_MOVE_ANM);
}

bool file_select_confirmation_active(const dFile_select_c* menu) {
    if (menu == nullptr) {
        return false;
    }
    return menu->mDataSelProc >= dFile_select_c::DATASELPROC_YES_NO_SELECT &&
        menu->mDataSelProc <= dFile_select_c::DATASELPROC_CMD_EXEC_PANE_MOVE0;
}

bool file_select_confirmation_interactive(const dFile_select_c* menu) {
    return menu != nullptr &&
        (menu->mDataSelProc == dFile_select_c::DATASELPROC_YES_NO_SELECT ||
            menu->mDataSelProc == dFile_select_c::DATASELPROC_YES_NO_CURSOR_MOVE_ANM);
}

constexpr u64 kFileSelectConfirmationFrames[] = {
    MULTI_CHAR('hd_cno'), MULTI_CHAR('hd_cyes'), // native order: No, Yes
};

void position_file_select_confirmation_cursor(dFile_select_c* menu) {
    if (!file_select_confirmation_interactive(menu) ||
        menu->mSelIcon == nullptr || menu->field_0x0268 >= 2) {
        return;
    }
    const auto index = menu->field_0x0268;
    J2DPane* group = menu->mYnSelPane[index] != nullptr ?
        menu->mYnSelPane[index]->getPanePtr() : nullptr;
    J2DPane* frame = group != nullptr ?
        group->search(kFileSelectConfirmationFrames[index]) : nullptr;
    if (frame != nullptr) {
        position_cursor_outside_frame(menu->mSelIcon, frame,
            file_select_layout::kActionCursorPaddingX,
            file_select_layout::kActionCursorPaddingY);
    }
}

f32 copy_title_text_width(JUTFont* font, const char* text, f32 size,
    f32 spacing) {
    if (font == nullptr || text == nullptr || font->getHeight() <= 0) {
        return 0.0f;
    }
    const f32 scale = size / static_cast<f32>(font->getHeight());
    f32 width = 0.0f;
    for (const unsigned char* cursor =
             reinterpret_cast<const unsigned char*>(text);
         *cursor != 0; ++cursor) {
        if (cursor != reinterpret_cast<const unsigned char*>(text)) {
            width += spacing;
        }
        width += static_cast<f32>(font->getWidth(*cursor)) * scale;
    }
    return width;
}

void style_copy_destination_title(dFile_select_c* menu, bool active) {
    J2DScreen* screen = menu != nullptr ? menu->mCpSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }
    J2DPane* group = screen->search(MULTI_CHAR('hd_ctit'));
    if (group == nullptr) {
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_ctit'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 78.0f));
        screen->appendChild(group);
    }
    if (!active) {
        group->hide();
        if (menu->mHeaderTxtDispIdx < 2) {
            for (std::size_t index = 0; index < 2; ++index) {
                if (menu->mHeaderTxtPane[index] != nullptr) {
                    menu->mHeaderTxtPane[index]->setAlpha(
                        index == menu->mHeaderTxtDispIdx ? 255 : 0);
                }
            }
        }
        return;
    }
    group->show();
    for (CPaneMgrAlpha* nativeTitle : menu->mHeaderTxtPane) {
        if (nativeTitle != nullptr) nativeTitle->setAlpha(0);
    }

    constexpr f32 size = 24.0f;
    constexpr f32 spacing = -0.5f;
    JUTFont* font = menu->fileSel.font[0];
    const char* pieces[] = {"Co", "p", "y to where?"};
    constexpr u64 tags[] = {
        MULTI_CHAR('hd_ct0'), MULTI_CHAR('hd_ct1'), MULTI_CHAR('hd_ct2'),
    };
    const f32 totalWidth = copy_title_text_width(font,
        "Copy to where?", size, spacing);
    f32 left = (608.0f - totalWidth) * 0.5f;
    for (std::size_t index = 0; index < 3; ++index) {
        const f32 pieceWidth = copy_title_text_width(font, pieces[index],
            size, spacing);
        auto* text = copy_metadata_text(group, tags[index],
            JGeometry::TBox2<f32>(left, 30.0f + (index == 1 ? 2.0f : 0.0f),
                left + pieceWidth + 4.0f,
                66.0f + (index == 1 ? 2.0f : 0.0f)),
            font, HBIND_LEFT, size,
            JUtility::TColor(234, 201, 55, 255));
        if (text != nullptr) {
            text->setCharSpace(spacing);
            text->setString(pieces[index]);
        }
        left += pieceWidth + spacing;
    }
}

void set_copy_file_info_visible(dFile_select_c* menu, std::size_t index,
    bool visible) {
    if (menu == nullptr || index >= 2 || menu->mCpFileInfo[index] == nullptr) {
        return;
    }
    CPaneMgrAlpha* data = menu->mCpFileInfo[index]->getDatBase();
    CPaneMgrAlpha* noData = menu->mCpFileInfo[index]->getNoDatBase();
    if (!visible) {
        if (data != nullptr) data->hide();
        if (noData != nullptr) noData->hide();
        return;
    }
    const u8 saveIndex = menu->getCptoNum(static_cast<u8>(index));
    const bool hasData = saveIndex < 3 && !menu->mIsNoData[saveIndex];
    if (data != nullptr) {
        hasData ? data->show() : data->hide();
    }
    if (noData != nullptr) {
        hasData ? noData->hide() : noData->show();
    }
}

void style_copy_destination_yes_no(dFile_select_c* menu, bool active) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (menu == nullptr || menu->mYnSel.ScrYn == nullptr ||
        frameTexture == nullptr) {
        return;
    }
    constexpr f32 centers[] = {390.0f, 218.0f}; // native order: No, Yes
    if (active) {
        // The confirmation is a modal replacement for the three-action row,
        // not another row stacked on top of it.
        for (CPaneMgrAlpha* action : menu->m3mSelPane) {
            if (action != nullptr) {
                action->hide();
            }
        }
    }
    for (std::size_t index = 0; index < 2; ++index) {
        J2DPane* group = menu->mYnSelPane[index] != nullptr ?
            menu->mYnSelPane[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }
        auto* frame = as_picture(group->search(kFileSelectConfirmationFrames[index]));
        if (frame == nullptr) {
            frame = JKR_NEW J2DPicture(kFileSelectConfirmationFrames[index],
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f),
                frameTexture, nullptr);
            frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
            frame->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            group->insertChild(group->getFirstChildPane(), frame);
        }
        if (!active) {
            frame->hide();
            group->hide();
            continue;
        }
        group->show();
        // Preserve native animation attachments: Dusklight advances the
        // confirmation state through them. Override only the rendered pose
        // after presentation; detaching here stalls selection/cancellation.
        group->scale(1.0f, 1.0f);
        if (s_fileSelectYesNoLayoutReady) {
            group->translate(s_fileSelectYesNoX[index],
                s_fileSelectYesNoY[index]);
        } else {
            move_file_select_pane_center(group, centers[index], 365.0f);
            s_fileSelectYesNoX[index] = group->getTranslateX();
            s_fileSelectYesNoY[index] = group->getTranslateY();
        }
        group->setAlpha(255);
        hide_other_pictures(group, frame);
        frame->resize(142.0f, 26.0f);
        frame->move((group->getWidth() - 142.0f) * 0.5f,
            (group->getHeight() - 26.0f) * 0.5f);
        const bool selected = menu->field_0x0268 == index;
        frame->setBlackWhite(selected ?
                JUtility::TColor(67, 66, 60, 255) :
                JUtility::TColor(8, 8, 6, 255),
            selected ? JUtility::TColor(255, 247, 154, 255) :
                JUtility::TColor(205, 201, 116, 255));
        frame->setAlpha(255);
        frame->show();
        if (menu->mYnSelPane_m[index] != nullptr) menu->mYnSelPane_m[index]->hide();
        if (menu->mYnSelPane_g[index] != nullptr) menu->mYnSelPane_g[index]->hide();
        if (menu->mYnSelPane_gr[index] != nullptr) menu->mYnSelPane_gr[index]->hide();
        auto* label = menu->mYnSelTxtPane[index] != nullptr ?
            static_cast<J2DTextBox*>(menu->mYnSelTxtPane[index]->getPanePtr()) :
            nullptr;
        if (label != nullptr) {
            label->setAlpha(255);
            label->setFontSize(16.0f, 16.0f);
            label->setCharSpace(0.0f);
            label->setWhite(selected ?
                JUtility::TColor(242, 242, 236, 255) :
                JUtility::TColor(155, 155, 150, 255));
        }
        if (selected && menu->mSelIcon != nullptr &&
            file_select_confirmation_interactive(menu)) {
            // Fixed buttons do not need the native transition's cursor fade.
            menu->mSelIcon->setAlphaRate(1.0f);
            position_file_select_confirmation_cursor(menu);
        }
    }
    if (active) {
        s_fileSelectYesNoLayoutReady = true;
    }
}

void style_copy_destination_screen(dFile_select_c* menu) {
    if (menu == nullptr || menu->mCpSel.Scr == nullptr) {
        return;
    }
    if (!menu->mCpSel.isShow) {
        style_copy_destination_title(menu, false);
        style_copy_destination_yes_no(menu, false);
        return;
    }
    const bool listActive = copy_destination_list_state(menu);
    const bool confirmActive = file_select_confirmation_active(menu);
    const bool overlayActive = listActive || confirmActive;
    style_copy_destination_title(menu, overlayActive);
    J2DPicture* background = copy_destination_background(menu);
    if (background != nullptr) {
        // The title and A/B prompts belong to the underlying File Selection
        // screen and must remain above this secondary content overlay.
        background->resize(608.0f, 370.0f);
        background->move(0.0f, 78.0f);
        overlayActive ? background->show() : background->hide();
    }
    J2DPicture* rows[2] = {};
    for (std::size_t index = 0; index < 2; ++index) {
        rows[index] = copy_destination_row_picture(menu, index);
        const bool rowVisible = listActive ||
            (confirmActive && menu->field_0x026b == index);
        if (rows[index] != nullptr && rowVisible) {
            // Do not inherit the copy layout's multi-stage slide/scale
            // transforms. Keep both destination cards fixed until the whole
            // overlay is removed, so metadata cannot sweep across the source
            // list during entry or exit.
            const f32 rowY = confirmActive ? 220.0f :
                (index == 0 ? 165.0f : 275.0f);
            J2DPane* source = menu->mCpFileInfo[index] != nullptr &&
                    menu->mCpFileInfo[index]->getDatBase() != nullptr ?
                menu->mCpFileInfo[index]->getDatBase()->getPanePtr() : nullptr;
            const bool hasSecondHeartLine = file_info_has_second_heart_line(source);
            const f32 rowHeight = hasSecondHeartLine ? 86.0f : 70.0f;
            rows[index]->resize(435.0f, rowHeight);
            rows[index]->move((608.0f - 435.0f) * 0.5f,
                rowY - rowHeight * 0.5f);
            apply_file_select_row_texture(rows[index],
                resource_texture(menu->field_0x026b == index ?
                    s_fileSelectSelectedRowResource : s_fileSelectRowResource));
            rows[index]->show();
            style_copy_destination_metadata_overlay(menu, index, rowY, true);
        } else {
            if (rows[index] != nullptr) rows[index]->hide();
            style_copy_destination_metadata_overlay(menu, index, 0.0f, false);
        }
        if (menu->mCpSelPane_moyo[index] != nullptr) {
            menu->mCpSelPane_moyo[index]->hide();
        }
        if (menu->mCpSelPane_gold[index] != nullptr) {
            menu->mCpSelPane_gold[index]->hide();
        }
        if (menu->mCpSelPane_gold2[index] != nullptr) {
            menu->mCpSelPane_gold2[index]->hide();
        }
        if (menu->mCpSelPane_book[index] != nullptr) {
            menu->mCpSelPane_book[index]->hide();
        }
        for (CPaneMgrAlpha* data : {
                 menu->mCpFileInfo[index] != nullptr ?
                    menu->mCpFileInfo[index]->getDatBase() : nullptr,
                 menu->mCpFileInfo[index] != nullptr ?
                    menu->mCpFileInfo[index]->getNoDatBase() : nullptr,
             }) {
            J2DPane* groupData = data != nullptr ? data->getPanePtr() : nullptr;
            J2DPicture* base = nullptr;
            f32 area = 0.0f;
            find_largest_picture(groupData, base, area);
            apply_file_select_row_texture(base,
                resource_texture(s_fileSelectClearRowResource));
        }
    }
    hide_copy_destination_architecture(menu->mCpSel.Scr,
        background, rows[0], rows[1]);
    for (std::size_t index = 0; index < 2; ++index) {
        if (rows[index] != nullptr && rows[index]->isVisible()) {
            copy_destination_number_picture(menu, index, rows[index]);
        }
    }
    style_copy_destination_metadata(menu);
    style_copy_destination_yes_no(menu, confirmActive);
    if (listActive && menu->field_0x026b < 2 &&
        menu->mSelIcon2 != nullptr && rows[menu->field_0x026b] != nullptr) {
        position_cursor_outside_frame(menu->mSelIcon2,
            rows[menu->field_0x026b]);
    }
}

void apply_file_select_row_texture(J2DPicture* picture, ResTIMG const* texture) {
    if (picture == nullptr || texture == nullptr ||
        (picture->getTexture(0) != nullptr &&
            picture->getTexture(0)->getTexInfo() == texture)) {
        return;
    }
    const JGeometry::TBox2<f32> bounds = picture->getBounds();
    picture->changeTexture(texture, 0);
    picture->resize(bounds.getWidth(), bounds.getHeight());
    picture->move(bounds.i.x, bounds.i.y);
    picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
    picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    picture->show();
}

void update_file_select_row_selection(dFile_select_c* menu) {
    ResTIMG const* normal = resource_texture(s_fileSelectRowResource);
    ResTIMG const* selected = resource_texture(s_fileSelectSelectedRowResource);
    if (menu == nullptr || menu->fileSel.Scr == nullptr || normal == nullptr ||
        selected == nullptr) {
        return;
    }

    constexpr u64 rowBaseTags[] = {
        MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'), MULTI_CHAR('w_go_b02'),
    };
    static dFile_select_c* geometryOwner = nullptr;
    static std::array<FileSelectRowGeometry, 3> rowGeometry = {};
    if (geometryOwner != menu) {
        geometryOwner = menu;
        rowGeometry = {};
    }
    const int selectedIndex = menu->getSelectNum();
    for (std::size_t index = 0; index < 3; ++index) {
        ResTIMG const* texture = static_cast<int>(index) == selectedIndex ? selected : normal;
        J2DPane* rowGroup = menu->fileSel.Scr->search(rowBaseTags[index]);
        J2DPicture* rowBase = as_picture(rowGroup);
        if (rowBase == nullptr) {
            f32 largestArea = 0.0f;
            find_largest_picture(rowGroup, rowBase, largestArea);
        }
        J2DPane* infoGroup = menu->mFileInfoDatBasePane[index] != nullptr ?
            menu->mFileInfoDatBasePane[index]->getPanePtr() : nullptr;
        size_file_select_row_for_hearts(rowBase,
            file_info_has_second_heart_line(infoGroup), rowGeometry[index]);
        apply_file_select_row_texture(rowBase, texture);
        update_file_select_row_shadow(rowBase,
            static_cast<int>(index) == selectedIndex);
    }
}

void position_cursor_outside_frame(dSelect_cursor_c* cursor, J2DPane* target,
    f32 horizontalPadding, f32 verticalPadding) {
    if (cursor == nullptr || target == nullptr || cursor->mpScreen == nullptr) {
        return;
    }

    // J2DPane::getGlbBounds() does not include every ancestor transform in
    // these dynamically appended menu trees.  Resolve the rendered vertices
    // through CPaneMgr, as the pointer hit-testing path does.
    CPaneMgr manager;
    Mtx matrix;
    const Vec vertex0 = manager.getGlobalVtx(target, &matrix, 0, false, 0);
    const Vec vertex3 = manager.getGlobalVtx(target, &matrix, 3, false, 0);
    const f32 left = std::min(vertex0.x, vertex3.x);
    const f32 top = std::min(vertex0.y, vertex3.y);
    const f32 right = std::max(vertex0.x, vertex3.x);
    const f32 bottom = std::max(vertex0.y, vertex3.y);
    cursor->setPos((left + right) * 0.5f,
        (top + bottom) * 0.5f, target, true);

    constexpr u64 cornerTags[] = {
        MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null'),
    };
    const f32 phase = cursor->field_0x40;
    const f32 oscillation = phase < 10.0f ? phase / 10.0f :
        (20.0f - phase) / 10.0f;
    const f32 pulse = (1.0f - cursor->mParam3) +
        oscillation * cursor->mParam3;
    const f32 halfWidth = (right - left) * 0.5f;
    const f32 halfHeight = (bottom - top) * 0.5f;
    const f32 cursorScaleX = std::fabs(cursor->mParam1) > 0.001f ?
        std::fabs(cursor->mParam1) : 1.0f;
    const f32 cursorScaleY = std::fabs(cursor->mParam2) > 0.001f ?
        std::fabs(cursor->mParam2) : 1.0f;

    for (int corner = 0; corner < 4; ++corner) {
        // setPos() only considers the pane's own scale.  HD submenu rows are
        // scaled by an ancestor at narrow aspect ratios, so replace its local
        // extents with the rendered global extents before positioning corners.
        // update() multiplies these stored extents by mParam1/mParam2.  PC
        // additionally aspect-adjusts mParam1, so normalize here to avoid a
        // second expansion after fitting to the rendered frame.
        cursor->field_0x94[corner] = (corner < 2 ? -1.0f : 1.0f) *
            (halfWidth + horizontalPadding) / cursorScaleX;
        cursor->field_0xa4[corner] = (corner % 2 == 0 ? -1.0f : 1.0f) *
            (halfHeight + verticalPadding) / cursorScaleY;
        cursor->field_0x74[corner] = cursor->mParam1 *
            cursor->field_0x94[corner] * pulse;
        cursor->field_0x84[corner] = cursor->mParam2 *
            cursor->field_0xa4[corner] * pulse;
        if (J2DPane* cornerPane = cursor->mpScreen->search(cornerTags[corner])) {
            cursor->moveCenter(cornerPane, cursor->field_0x74[corner],
                cursor->field_0x84[corner]);
        }
    }
}

void position_file_select_cursor(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr || menu->mSelIcon == nullptr) {
        return;
    }

    if (file_select_confirmation_active(menu)) {
        // Draw/update hooks run after styling and must not reattach the
        // shared confirmation cursor to the selected save card.
        position_file_select_confirmation_cursor(menu);
        return;
    }

    constexpr u64 actionFrameTags[] = {
        MULTI_CHAR('hd_3m0'), MULTI_CHAR('hd_3m1'), MULTI_CHAR('hd_3m2'),
    };
    const bool launchingFromStart =
        menu->mDataSelProc == dFile_select_c::DATASELPROC_NEXT_MODE_WAIT &&
        menu->mIsSelectEnd && menu->mSelectMenuNum == 1;
    if (launchingFromStart) {
        // Once Start has been accepted, the native file-select contents fade
        // immediately.  Do not preserve our pane-attached cursor throughout
        // NEXT_MODE_WAIT; otherwise its four corners outlive the rest of the
        // screen while the game is loading.
        menu->mSelIcon->setAlphaRate(0.0f);
        return;
    }
    if ((menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT ||
            menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT_MOVE_ANM) &&
        menu->mSelectMenuNum < 3) {
        J2DPane* group = menu->m3mSelPane[menu->mSelectMenuNum] != nullptr ?
            menu->m3mSelPane[menu->mSelectMenuNum]->getPanePtr() : nullptr;
        J2DPane* frame = group != nullptr ?
            group->search(actionFrameTags[menu->mSelectMenuNum]) : nullptr;
        if (frame != nullptr) {
            position_cursor_outside_frame(menu->mSelIcon, frame,
                file_select_layout::kActionCursorPaddingX,
                file_select_layout::kActionCursorPaddingY);
            return;
        }
    }

    constexpr u64 rowBaseTags[] = {
        MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'), MULTI_CHAR('w_go_b02'),
    };
    const int selectedIndex = menu->getSelectNum();
    if (selectedIndex < 0 || selectedIndex >= 3) {
        return;
    }

    J2DPane* rowGroup = menu->fileSel.Scr->search(rowBaseTags[selectedIndex]);
    J2DPicture* rowBase = as_picture(rowGroup);
    if (rowBase == nullptr) {
        f32 largestArea = 0.0f;
        find_largest_picture(rowGroup, rowBase, largestArea);
    }
    if (rowBase == nullptr) {
        return;
    }

    // Keep the cursor attached to the visible row frame.  Dusklight updates a
    // pane-attached cursor from that pane's global centre and applies the
    // current aspect scale when it draws.  Detaching it here freezes rendered
    // screen coordinates while the file-select panes continue to resize,
    // which makes the brackets drift as the window size/aspect changes.
    CPaneMgr manager;
    const Vec center = manager.getGlobalVtxCenter(rowBase, false, 0);
    menu->mSelIcon->setPos(center.x, center.y, rowBase, true);

    // Apply one small, row-independent inset in the pane's virtual layout
    // units.  The selected box may scale, but these offsets remain tied to its
    // edges instead of to the host window's pixels.
    for (int corner = 0; corner < 4; ++corner) {
        menu->mSelIcon->field_0x94[corner] += corner < 2 ? 2.0f : -2.0f;
    }
}

void align_file_select_play_time(J2DPicture* row, J2DTextBox* text) {
    if (row == nullptr || text == nullptr || text->getParentPane() == nullptr) {
        return;
    }

    // Recompute live matrices: getGlbBounds() still contains the previous
    // draw's geometry while cards resize and their parents animate.
    CPaneMgr manager;
    Mtx matrix;
    const Vec top = manager.getGlobalVtx(row, &matrix, 0, false, 0);
    const Vec bottom = manager.getGlobalVtx(row, &matrix, 3, false, 0);
    const Vec center = manager.getGlobalVtxCenter(text, false, 0);
    const f32 targetY = file_select_layout::play_time_center(top.y, bottom.y);
    manager.getGlobalVtx(text->getParentPane(), &matrix, 0, false, 0);
    const f32 determinant = matrix[0][0] * matrix[1][1] -
        matrix[0][1] * matrix[1][0];
    if (!std::isfinite(targetY) || !std::isfinite(center.y) ||
        !std::isfinite(determinant) || std::fabs(determinant) < 0.0001f ||
        bottom.y <= top.y) {
        return;
    }
    const f32 dy = targetY - center.y;
    const f32 dxLocal = -matrix[0][1] * dy / determinant;
    const f32 dyLocal = matrix[0][0] * dy / determinant;
    if (!std::isfinite(dxLocal) || !std::isfinite(dyLocal)) {
        return;
    }
    // Give the label and value the same vertical binding, retaining their
    // independent horizontal alignment and all native strings/colors.
    text->mFlags = static_cast<u8>((text->mFlags & ~0x03) | VBIND_CENTER);
    text->field_0x110 = 0.0f;
    text->add(dxLocal, dyLocal);
}

JGeometry::TBox2<f32> live_metadata_bounds(J2DPane* pane) {
    CPaneMgr manager;
    Mtx matrix;
    const Vec a = manager.getGlobalVtx(pane, &matrix, 0, false, 0);
    const Vec b = manager.getGlobalVtx(pane, &matrix, 3, false, 0);
    return JGeometry::TBox2<f32>(a.x, a.y, b.x, b.y);
}

void translate_metadata_global(J2DPane* pane, f32 dx, f32 dy) {
    const auto bounds = live_metadata_bounds(pane);
    position_dmap_global_center(pane, (bounds.i.x + bounds.f.x) * 0.5f + dx,
        (bounds.i.y + bounds.f.y) * 0.5f + dy);
}

void style_file_select_metadata(dFile_select_c* menu) {
    if (menu == nullptr) {
        return;
    }

    constexpr u64 labelTags[] = {
        MULTI_CHAR('f_s_t_02'), MULTI_CHAR('f_p_t_02'),
    };
    constexpr u64 valueTags[] = {
        MULTI_CHAR('w_time01'), MULTI_CHAR('w_ptim01'),
    };
    constexpr u64 rowTags[] = {
        MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'), MULTI_CHAR('w_go_b02'),
    };
    for (std::size_t index = 0; index < 3; ++index) {
        J2DPane* group = menu->mFileInfoDatBasePane[index] != nullptr ?
            menu->mFileInfoDatBasePane[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }

        auto* name = static_cast<J2DTextBox*>(group->search(MULTI_CHAR('f_name01')));
        auto* saveLabel = static_cast<J2DTextBox*>(
            group->search(MULTI_CHAR('f_s_t_02')));
        J2DPane* firstHeart = group->search(MULTI_CHAR('hear_20'));
        const auto groupGlobal = live_metadata_bounds(group);
        const f32 scaleX = group->getWidth() > 0.0f ?
            groupGlobal.getWidth() / group->getWidth() : 1.0f;
        const f32 scaleY = group->getHeight() > 0.0f ?
            groupGlobal.getHeight() / group->getHeight() : 1.0f;
        const bool hasSecondHeartLine = file_info_has_second_heart_line(group);
        J2DPane* rowGroup = menu->fileSel.Scr != nullptr ?
            menu->fileSel.Scr->search(rowTags[index]) : nullptr;
        J2DPicture* row = as_picture(rowGroup);
        if (row == nullptr) {
            f32 area = 0.0f;
            find_largest_picture(rowGroup, row, area);
        }

        // Resolve every position from live pane geometry. The archive applies
        // one last transform after reset/create; additive one-time offsets
        // consequently left individual labels stranded around the screen.
        // These relationship-based corrections are idempotent and safe to
        // repeat immediately before every draw.
        if (saveLabel != nullptr && std::isfinite(scaleY) &&
            std::fabs(scaleY) > 0.001f) {
            const f32 saveTop = live_metadata_bounds(saveLabel).i.y;
            for (std::size_t heart = 0; heart < 20; ++heart) {
                J2DPane* pane = group->search(kFileSelectHeartTags[heart]);
                if (pane == nullptr || !pane->isVisible()) {
                    continue;
                }
                const auto bounds = live_metadata_bounds(pane);
                const f32 targetBottom = saveTop -
                    (hasSecondHeartLine ? 5.0f : 3.0f) * scaleY -
                    (hasSecondHeartLine && heart < 10 ?
                        bounds.getHeight() : 0.0f);
                if (std::isfinite(bounds.f.y) &&
                    std::isfinite(targetBottom)) {
                    translate_metadata_global(pane, 0.0f, targetBottom - bounds.f.y);
                }
            }
        }

        const f32 heartLeft = firstHeart != nullptr ?
            live_metadata_bounds(firstHeart).i.x : 0.0f;
        const bool canAlign = firstHeart != nullptr &&
            std::isfinite(heartLeft) && std::isfinite(scaleX) &&
            std::isfinite(scaleY) && std::fabs(scaleX) > 0.001f &&
            std::fabs(scaleY) > 0.001f;

        if (name != nullptr) {
            name->setFontSize(13.5f, 13.5f);
            if (saveLabel != nullptr && canAlign) {
                name->mFlags = static_cast<u8>((name->mFlags & ~0x0c) |
                    (HBIND_RIGHT << 2));
                const auto nameBounds = live_metadata_bounds(name);
                const JGeometry::TBox2<f32>& labelBounds =
                    live_metadata_bounds(saveLabel);
                const JGeometry::TBox2<f32>& heartBounds =
                    live_metadata_bounds(firstHeart);
                const f32 deltaX = labelBounds.f.x - nameBounds.f.x;
                const f32 deltaY =
                    (heartBounds.i.y + heartBounds.f.y -
                        nameBounds.i.y - nameBounds.f.y) * 0.5f;
                if (std::isfinite(deltaX) && std::isfinite(deltaY)) {
                    translate_metadata_global(name, deltaX, deltaY);
                }
            }
        }
        for (u64 tag : labelTags) {
            auto* text = static_cast<J2DTextBox*>(group->search(tag));
            if (text != nullptr) {
                text->setFontSize(11.5f, 11.5f);
                if (tag == MULTI_CHAR('f_p_t_02')) {
                    align_file_select_play_time(row, text);
                }
            }
        }
        for (u64 tag : valueTags) {
            auto* text = static_cast<J2DTextBox*>(group->search(tag));
            if (text != nullptr) {
                text->setFontSize(11.5f, 11.5f);
                text->mFlags = static_cast<u8>((text->mFlags & ~0x0c) |
                    (HBIND_LEFT << 2));
                if (tag == MULTI_CHAR('w_time01')) {
                    text->setCharSpace(-0.25f);
                }
                if (canAlign) {
                    const f32 deltaX = heartLeft + 5.0f * scaleX -
                        live_metadata_bounds(text).i.x;
                    if (std::isfinite(deltaX)) {
                        translate_metadata_global(text, deltaX, 0.0f);
                    }
                }
                if (tag == MULTI_CHAR('w_ptim01')) {
                    align_file_select_play_time(row, text);
                }
            }
        }

        // Remove the archive's harder line art; the HD row texture supplies
        // the two softer faded rules around Save time instead.
        hide_collect_decoration_texture(group,
            file_select_archive_texture(menu, "tt_line2.bti"));
    }
}

void scale_file_select_button(CPaneMgrAlpha* paneManager, const f32 scale) {
    J2DPane* group = paneManager != nullptr ? paneManager->getPanePtr() : nullptr;
    J2DPicture* button = nullptr;
    f32 largestArea = 0.0f;
    find_largest_picture(group, button, largestArea);
    if (button == nullptr) {
        return;
    }
    const JGeometry::TBox2<f32> bounds = button->getBounds();
    const f32 width = bounds.getWidth() * scale;
    const f32 height = bounds.getHeight() * scale;
    button->resize(width, height);
    button->move(bounds.i.x + (bounds.getWidth() - width) * 0.5f,
        bounds.i.y + (bounds.getHeight() - height) * 0.5f);
}

void hide_wide_file_select_bands(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane)) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        const f32 width = bounds.getWidth();
        const f32 height = bounds.getHeight();
        // The pale carved header and footer are wide shallow pictures. Thin
        // separator lines are deliberately excluded, as are the 435px rows
        // and the full-screen painted backdrop.
        if (width >= 600.0f && height >= 24.0f && height <= 120.0f) {
            picture->hide();
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_wide_file_select_bands(child);
    }
}

void remove_file_select_decorative_bands(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }

    // These two fixed panes carry the pale patterned cap and footer visible
    // outside the main background. Hiding them reveals the layout's existing
    // thin gold separator lines without affecting prompts or transitions.
    for (u64 tag : {MULTI_CHAR('w_dmhibi'), MULTI_CHAR('w_hibi0')}) {
        if (J2DPane* band = menu->fileSel.Scr->search(tag)) {
            band->hide();
        }
    }
    hide_wide_file_select_bands(menu->fileSel.Scr);
}

void remove_file_select_stonework(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }

    // The File Selection layout builds its pale stone architecture from tiled
    // blocks, narrow filler strips, carved rails, and fine edge lines. The
    // save-data panels and selection corners use different textures, so those
    // remain intact when these layers are hidden.
    constexpr const char* stoneTextures[] = {
        "tt_block128_00.bti",
        "tt_block8x8.bti",
        "tt_horiwaku_lu.bti",
        "tt_horiwaku_side_l_rr.bti",
        "tt_horiwaku_side_r_rr.bti",
        "tt_horiwaku_top_rr.bti",
        "tt_i4_gra.bti",
        "tt_line2.bti",
    };
    for (const char* textureName : stoneTextures) {
        hide_collect_decoration_texture(menu->fileSel.Scr,
            file_select_archive_texture(menu, textureName));
    }

    // The prompt backplates are J2DWindow panes, not pictures, so the
    // texture-based prompt cleanup cannot match them. Keep the button discs,
    // custom Back label, and flourish while suppressing only the two opaque
    // native rectangles behind the A/B prompts.
    for (const u64 tag : {
             MULTI_CHAR('w_abase'), MULTI_CHAR('w_bbase')}) {
        if (J2DPane* pane = menu->fileSel.Scr->search(tag)) {
            pane->hide();
        }
    }
}

void apply_file_select_hd_style(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }

    replace_file_select_prompt(menu->mAbtnPane,
        menu_face_button_texture(true), MULTI_CHAR('hd_fsab'));
    replace_file_select_prompt(menu->mBbtnPane,
        menu_face_button_texture(false), MULTI_CHAR('hd_fsbb'));
    hide_collect_decoration_texture(menu->fileSel.Scr,
        file_select_archive_texture(menu, "tt_zelda_button_ab_maru.bti"));
    scale_file_select_button(menu->mAbtnPane, 0.88f);
    style_file_select_prompt_text(menu);
    if (menu != nullptr) {
        clear_file_select_prompt_panel(menu->mModoruTxtPane);
        clear_file_select_prompt_panel(menu->mKetteiTxtPane);
    }
    position_file_select_prompts(menu);
    replace_file_select_background(menu);
    remove_file_select_stonework(menu);
    remove_file_select_decorative_bands(menu);
    simplify_file_select_rows(menu);
    style_copy_destination_screen(menu);
    simplify_file_select_numbers(menu);
    style_file_select_metadata(menu);
    update_file_select_row_selection(menu);
    add_file_select_detail_frame(menu);
    add_file_select_title_rules(menu);
    add_file_select_back_label(menu);
    add_file_select_prompt_flourish(menu);
    style_file_select_action_buttons(menu);

    // On the original light stone field these large vanilla spirals read as
    // faint embossing. Against the new brown background they become bright,
    // oversized curls, so let the replacement texture's subtler linework take
    // their place.
    hide_collect_decoration_texture(menu->fileSel.Scr,
        file_select_archive_texture(menu, "tt_uzumaki_00.bti"));
    hide_collect_decoration_texture(menu->fileSel.Scr,
        file_select_archive_texture(menu, "tt_yakushima.bti"));

    style_file_select_dynamic_text(menu);
}
