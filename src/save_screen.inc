ResTIMG const* save_menu_archive_texture(dMenu_save_c* menu,
    const char* textureName) {
    return menu != nullptr && menu->mpArchive != nullptr && textureName != nullptr ?
        static_cast<ResTIMG const*>(
            menu->mpArchive->getResource('TIMG', textureName)) : nullptr;
}

// Save menu -----------------------------------------------------------------

void replace_save_menu_background(dMenu_save_c* menu) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr) {
        return;
    }
    J2DScreen* screen = menu->mSaveSel.Scr;
    ResTIMG const* replacement = resource_texture(s_fileSelectBackgroundResource);
    if (replacement == nullptr) {
        return;
    }

    // The save layout has two opaque full-screen stone pictures. A sibling
    // overlay is necessarily either behind both of them or over the menu data,
    // so replace the actual rear canvas and suppress the front stone sheet.
    if (auto* background = as_picture(screen->search(MULTI_CHAR('w_dmbas0')))) {
        apply_file_select_row_texture(background, replacement);
        // The native save canvas stops almost exactly at the virtual-screen
        // edges. Its widescreen transform can leave a thin view of gameplay at
        // the top and bottom, so overscan the replacement vertically while
        // preserving the native horizontal coverage.
        background->move(-73.5f, -45.5f);
        background->resize(712.0f, 486.0f);
        background->show();
    }
    if (J2DPane* obsoleteOverlay = screen->search(MULTI_CHAR('hd_sbg'))) {
        obsoleteOverlay->hide();
    }
    for (u64 tag : {MULTI_CHAR('w_bas01'), MULTI_CHAR('w_bas13'),
             MULTI_CHAR('w_bas14')}) {
        if (J2DPane* stone = screen->search(tag)) {
            stone->hide();
        }
    }
}

void style_save_select_title(dMenu_save_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->mSaveSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }
    auto* nativeTitle = menu->mHeaderTxtType < 2 &&
            menu->mpHeaderTxtPane[menu->mHeaderTxtType] != nullptr ?
        static_cast<J2DTextBox*>(
            menu->mpHeaderTxtPane[menu->mHeaderTxtType]->getPanePtr()) :
        nullptr;
    const char* nativeString = text_box_string(nativeTitle);
    const bool saveQuestion = nativeString != nullptr &&
        std::strstr(nativeString, "Save to which log?") != nullptr;
    const bool overwriteQuestion = nativeString != nullptr &&
        std::strstr(nativeString, "Overwrite this log?") != nullptr;
    const bool active = saveQuestion || overwriteQuestion;
    if (J2DPane* rules = screen->search(MULTI_CHAR('hd_stlin'))) {
        // Both log questions use one compact banner. Restore the original
        // bounds for Saved/Continue and other native status/error messages.
        const f32 top = active ? file_select_layout::kSaveQuestionBannerTop : 24.0f;
        const f32 bottom = active ? file_select_layout::kSaveQuestionBannerBottom : 80.0f;
        rules->resize(screen->getBounds().getWidth() - 48.0f, bottom - top);
        rules->move(24.0f, top);
    }
    J2DPane* group = screen->search(MULTI_CHAR('hd_stit'));
    if (group == nullptr) {
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_stit'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 78.0f));
        screen->appendChild(group);
    }
    if (!active) {
        group->hide();
        if (menu->mHeaderTxtType < 2) {
            const std::size_t current = menu->mHeaderTxtType;
            const std::size_t incoming = current ^ 1;

            // The native save screen cross-fades between two title panes.
            // At the larger TPHD title size, the outgoing and incoming text
            // visibly collide (most noticeably between "Saved." and
            // "Continue playing?"). Keep the native transition timing, but
            // render only the title that is taking over during the handoff.
            if (!menu->mHeaderAnmComplete) {
                if (menu->mpHeaderTxtPane[current] != nullptr) {
                    menu->mpHeaderTxtPane[current]->setAlpha(0);
                }
                if (menu->mpHeaderTxtPane[incoming] != nullptr) {
                    menu->mpHeaderTxtPane[incoming]->setAlpha(255);
                }
            } else {
                if (menu->mpHeaderTxtPane[current] != nullptr) {
                    menu->mpHeaderTxtPane[current]->setAlpha(255);
                }
                if (menu->mpHeaderTxtPane[incoming] != nullptr) {
                    menu->mpHeaderTxtPane[incoming]->setAlpha(0);
                }
            }
        }
        return;
    }
    group->show();
    for (CPaneMgrAlpha* title : menu->mpHeaderTxtPane) {
        if (title != nullptr) title->setAlpha(0);
    }

    constexpr f32 size = file_select_layout::kSaveQuestionFontSize;
    constexpr f32 spacing = -0.5f * (size / 24.0f);
    constexpr f32 textTop = file_select_layout::kSaveQuestionTextCenter -
        file_select_layout::kSaveQuestionTextHeight * 0.5f;
    constexpr f32 textBottom = file_select_layout::kSaveQuestionTextCenter +
        file_select_layout::kSaveQuestionTextHeight * 0.5f;
    JUTFont* font = menu->mSaveSel.font[0];
    const char* question = overwriteQuestion ? "Overwrite this log?" : "Save to which log?";
    const char* pieces[] = {
        overwriteQuestion ? "Overwrite this lo" : "Save to which lo", "g", "?",
    };
    constexpr u64 tags[] = {
        MULTI_CHAR('hd_st0'), MULTI_CHAR('hd_st1'), MULTI_CHAR('hd_st2'),
    };
    const f32 totalWidth = copy_title_text_width(font, question, size, spacing);
    f32 left = (608.0f - totalWidth) * 0.5f;
    for (std::size_t index = 0; index < 3; ++index) {
        const f32 pieceWidth = copy_title_text_width(font, pieces[index],
            size, spacing);
        const f32 opticalDrop = index == 1 ? 2.0f * (size / 24.0f) : 0.0f;
        auto* text = copy_metadata_text(group, tags[index],
            JGeometry::TBox2<f32>(left, textTop + opticalDrop,
                left + pieceWidth + 4.0f, textBottom + opticalDrop),
            font, HBIND_LEFT, size,
            JUtility::TColor(234, 201, 55, 255));
        if (text != nullptr) {
            text->setCharSpace(spacing);
            text->setString(pieces[index]);
        }
        left += pieceWidth + spacing;
    }
}

void simplify_save_menu_rows(dMenu_save_c* menu) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr) {
        return;
    }
    if (resource_texture(s_fileSelectRowResource) == nullptr) {
        return;
    }

    for (std::size_t index = 0; index < 3; ++index) {
        J2DPane* row = menu->mpSelData[index] != nullptr ?
            menu->mpSelData[index]->getPanePtr() : nullptr;
        constexpr u64 oldRowTags[] = {
            MULTI_CHAR('hd_sr0'), MULTI_CHAR('hd_sr1'), MULTI_CHAR('hd_sr2'),
        };
        constexpr u64 nativeRowTags[] = {
            MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'),
            MULTI_CHAR('w_go_b02'),
        };
        if (row != nullptr) {
            if (J2DPane* oldPanel = row->search(oldRowTags[index])) {
                oldPanel->hide();
            }
            auto* panel = as_picture(row->search(nativeRowTags[index]));
            if (panel != nullptr) {
                apply_file_select_row_texture(panel,
                    resource_texture(s_fileSelectRowResource));
                panel->show();
            }
            for (J2DPane* child = row->getFirstChildPane(); child != nullptr;
                 child = child->getNextChildPane()) {
                const u64 tag = child->mInfoTag;
                const char c0 = static_cast<char>(tag >> 56);
                const char c1 = static_cast<char>(tag >> 48);
                const char c2 = static_cast<char>(tag >> 40);
                const char c3 = static_cast<char>(tag >> 32);
                if (c0 == 'w' && c1 == '_' && c2 == 'w' && c3 == 's') {
                    child->hide();
                }
            }
        }

        if (menu->mpSelWakuMoyo[index] != nullptr) {
            menu->mpSelWakuMoyo[index]->hide();
        }
        if (menu->mpSelWakuGold[index] != nullptr) {
            menu->mpSelWakuGold[index]->hide();
        }
        if (menu->mpSelWakuGold2[index] != nullptr) {
            menu->mpSelWakuGold2[index]->hide();
        }
        if (menu->mpBookWaku[index] != nullptr) {
            menu->mpBookWaku[index]->hide();
        }

    }
}

void simplify_save_menu_numbers(dMenu_save_c* menu) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr) {
        return;
    }
    constexpr const char* numberTextures[] = {
        "tt_1_metal_40x40.bti", "tt_2_metal_40x40.bti", "tt_3_metal_40x40.bti",
    };
    for (std::size_t index = 0; index < 3; ++index) {
        J2DPicture* number = nullptr;
        find_picture_with_texture(menu->mSaveSel.Scr,
            save_menu_archive_texture(menu, numberTextures[index]), number);
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

J2DPicture* save_menu_row_picture(dMenu_save_c* menu, std::size_t index) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr || index >= 3) {
        return nullptr;
    }
    constexpr u64 nativeRowTags[] = {
        MULTI_CHAR('w_go_b00'), MULTI_CHAR('w_go_b01'),
        MULTI_CHAR('w_go_b02'),
    };
    J2DPane* row = menu->mpSelData[index] != nullptr ?
        menu->mpSelData[index]->getPanePtr() : nullptr;
    return as_picture(row != nullptr ? row->search(nativeRowTags[index]) : nullptr);
}

void update_save_menu_row_selection(dMenu_save_c* menu) {
    ResTIMG const* normal = resource_texture(s_fileSelectRowResource);
    ResTIMG const* selected = resource_texture(s_fileSelectSelectedRowResource);
    if (menu == nullptr || normal == nullptr || selected == nullptr) {
        return;
    }
    static dMenu_save_c* geometryOwner = nullptr;
    static std::array<FileSelectRowGeometry, 3> rowGeometry = {};
    if (geometryOwner != menu) {
        geometryOwner = menu;
        rowGeometry = {};
    }
    for (std::size_t index = 0; index < 3; ++index) {
        J2DPicture* row = save_menu_row_picture(menu, index);
        J2DPane* infoGroup = menu->mpDataBase[index] != nullptr ?
            menu->mpDataBase[index]->getPanePtr() : nullptr;
        size_file_select_row_for_hearts(row,
            file_info_has_second_heart_line(infoGroup), rowGeometry[index]);
        apply_file_select_row_texture(row,
            index == menu->mSelectedFile ? selected : normal);
        update_file_select_row_shadow(row, index == menu->mSelectedFile);
    }
}

void style_save_menu_metadata(dMenu_save_c* menu) {
    if (menu == nullptr) {
        return;
    }
    constexpr u64 labelTags[] = {
        MULTI_CHAR('f_s_t_02'), MULTI_CHAR('f_p_t_02'),
    };
    constexpr u64 valueTags[] = {
        MULTI_CHAR('w_time01'), MULTI_CHAR('w_ptim01'),
    };
    for (std::size_t index = 0; index < 3; ++index) {
        J2DPane* group = menu->mpDataBase[index] != nullptr ?
            menu->mpDataBase[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }
        auto* name = static_cast<J2DTextBox*>(
            group->search(MULTI_CHAR('f_name01')));
        auto* saveLabel = static_cast<J2DTextBox*>(
            group->search(MULTI_CHAR('f_s_t_02')));
        auto* playLabel = static_cast<J2DTextBox*>(
            group->search(MULTI_CHAR('f_p_t_02')));
        auto* playValue = static_cast<J2DTextBox*>(
            group->search(MULTI_CHAR('w_ptim01')));
        J2DPane* firstHeart = group->search(MULTI_CHAR('hear_20'));
        const auto groupGlobal = live_metadata_bounds(group);
        const f32 scaleX = group->getWidth() > 0.0f ?
            groupGlobal.getWidth() / group->getWidth() : 1.0f;
        const f32 scaleY = group->getHeight() > 0.0f ?
            groupGlobal.getHeight() / group->getHeight() : 1.0f;
        const bool hasSecondHeartLine = file_info_has_second_heart_line(group);
        J2DPicture* row = save_menu_row_picture(menu, index);
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
                if (text == playLabel) {
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
                if (text == playValue) {
                    align_file_select_play_time(row, text);
                }
            }
        }
        hide_collect_decoration_texture(group,
            save_menu_archive_texture(menu, "tt_line2.bti"));
    }
}

void add_save_menu_title_rules(dMenu_save_c* menu) {
    ResTIMG const* rules = resource_texture(s_fileSelectTitleRulesResource);
    J2DScreen* screen = menu != nullptr ? menu->mSaveSel.Scr : nullptr;
    if (screen == nullptr || rules == nullptr ||
        screen->search(MULTI_CHAR('hd_stlin')) != nullptr) {
        return;
    }
    const JGeometry::TBox2<f32> rootBounds = screen->getBounds();
    auto* overlay = JKR_NEW J2DPicture(MULTI_CHAR('hd_stlin'),
        JGeometry::TBox2<f32>(24.0f, 24.0f,
            rootBounds.getWidth() - 24.0f, 80.0f), rules, nullptr);
    overlay->setTexCoord(overlay->getTexture(0), BIND15, MIRROR0, false);
    overlay->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    overlay->setCornerColor(JUtility::TColor(255, 255, 255, 190));
    screen->appendChild(overlay);
}

void position_save_menu_prompts(dMenu_save_c* menu) {
    position_menu_prompt_group(menu != nullptr ? menu->mSaveSel.Scr : nullptr,
        MULTI_CHAR('hd_sprm'));
}

void add_save_menu_fixed_prompts(dMenu_save_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->mSaveSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }

    const auto rootBounds = screen->getBounds();
    const f32 right = rootBounds.f.x;
    auto* group = screen->search(MULTI_CHAR('hd_sprm'));
    if (group == nullptr) {
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_sprm'), rootBounds);
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

        // Keep the flourish behind the two button discs.
        addPicture(MULTI_CHAR('hd_sflr'),
            JGeometry::TBox2<f32>(right - 74.0f, 18.0f,
                right - 2.0f, 84.0f),
            resource_texture(s_fileSelectPromptFlourishResource), 220);

        auto* confirm = make_hd_prompt_label(MULTI_CHAR('hd_sconf'),
            JGeometry::TBox2<f32>(right - 114.0f, 30.0f,
                right - 54.0f, 53.0f), "Confirm", HBIND_RIGHT);
        group->appendChild(confirm);

        addPicture(MULTI_CHAR('hd_sapi'),
            JGeometry::TBox2<f32>(right - 50.0f, 28.0f,
                right - 23.0f, 55.0f), menu_face_button_texture(true));
        group->appendChild(make_hd_prompt_label(MULTI_CHAR('hd_sbck'),
            JGeometry::TBox2<f32>(right - 109.0f, 51.0f,
                right - 66.0f, 74.0f), "Back"));
        addPicture(MULTI_CHAR('hd_sbpi'),
            JGeometry::TBox2<f32>(right - 62.0f, 49.0f,
                right - 35.0f, 76.0f), menu_face_button_texture(false));
    }

    update_menu_face_button(screen, MULTI_CHAR('hd_sapi'), true);
    update_menu_face_button(screen, MULTI_CHAR('hd_sbpi'), false);
    group->show();
    position_save_menu_prompts(menu);

    // Hide the native prompt cluster behind the fixed group.
    for (CPaneMgrAlpha* pane : {
             menu->mpConfirmTxt, menu->mpABtnIcon,
             menu->mpBackTxt, menu->mpBBtnIcon,
         }) {
        if (pane != nullptr && pane->getPanePtr() != nullptr) {
            pane->getPanePtr()->hide();
        }
    }
}

void style_save_menu_prompts(dMenu_save_c* menu) {
    if (menu == nullptr) {
        return;
    }
    replace_file_select_prompt(menu->mpABtnIcon,
        menu_face_button_texture(true), MULTI_CHAR('hd_smab'));
    replace_file_select_prompt(menu->mpBBtnIcon,
        menu_face_button_texture(false), MULTI_CHAR('hd_smbb'));
    scale_file_select_button(menu->mpABtnIcon, 0.88f);
    clear_file_select_prompt_panel(menu->mpBackTxt);
    clear_file_select_prompt_panel(menu->mpConfirmTxt);

    auto* back = menu->mpBackTxt != nullptr ?
        static_cast<J2DTextBox*>(menu->mpBackTxt->getPanePtr()) : nullptr;
    auto* confirm = menu->mpConfirmTxt != nullptr ?
        static_cast<J2DTextBox*>(menu->mpConfirmTxt->getPanePtr()) : nullptr;
    if (back != nullptr) {
        back->setAlpha(0);
        menu->mpBackTxt->setAlpha(0);
    }
    if (confirm != nullptr) {
        confirm->setFontSize(13.5f, 13.5f);
        confirm->setFontColor(JUtility::TColor(235, 235, 230, 255),
            JUtility::TColor(255, 255, 255, 255));
    }
    position_save_menu_prompts(menu);
}

void style_save_menu_yes_no_buttons(dMenu_save_c* menu) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (menu == nullptr || frameTexture == nullptr) {
        return;
    }
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_sno'), MULTI_CHAR('hd_syes'),
    };
    constexpr f32 targetWidth = 142.0f;
    constexpr f32 targetHeight = 26.0f;

    for (std::size_t index = 0; index < 2; ++index) {
        J2DPane* group = menu->mpNoYes[index] != nullptr ?
            menu->mpNoYes[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
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
        const bool selected = menu->mYesNoCursor == index;
        frame->setBlackWhite(selected ?
                JUtility::TColor(67, 66, 60, 255) :
                JUtility::TColor(8, 8, 6, 255),
            selected ? JUtility::TColor(255, 247, 154, 255) :
                JUtility::TColor(205, 201, 116, 255));
        frame->setAlpha(255);
        frame->show();

        auto* label = menu->mpNoYesTxt[index] != nullptr ?
            static_cast<J2DTextBox*>(menu->mpNoYesTxt[index]->getPanePtr()) : nullptr;
        if (label != nullptr) {
            label->setFontSize(16.0f, 16.0f);
            label->setCharSpace(0.0f);
            label->setWhite(selected ?
                JUtility::TColor(242, 242, 236, 255) :
                JUtility::TColor(155, 155, 150, 255));
        }
    }
}

void hide_save_menu_stonework(dMenu_save_c* menu) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr) {
        return;
    }
    constexpr const char* textures[] = {
        "tt_block128_00.bti", "tt_block8x8.bti", "tt_3setu_w_l.bti",
        "tt_horiwaku_lu.bti", "tt_horiwaku_side_l_rr.bti",
        "tt_horiwaku_side_r_rr.bti", "tt_horiwaku_top_rr.bti",
        "tt_i4_gra.bti", "tt_button_base0_center_tate.bti",
        "tt_button_base0_side.bti", "tt_spot_square3.bti",
        "tt_metal_cube_00.bti", "tt_uzumaki_00.bti",
        "tt_yakushima.bti", "tt_gold_uzu_long2.bti",
    };
    for (const char* textureName : textures) {
        hide_collect_decoration_texture(menu->mSaveSel.Scr,
            save_menu_archive_texture(menu, textureName));
    }
    for (u64 tag : {MULTI_CHAR('w_bas01'), MULTI_CHAR('w_bas13'),
             MULTI_CHAR('w_bas14'), MULTI_CHAR('w_sen02'),
             MULTI_CHAR('w_sen03'), MULTI_CHAR('w_btn_n'),
             MULTI_CHAR('w_a_base')}) {
        if (J2DPane* pane = menu->mSaveSel.Scr->search(tag)) {
            pane->hide();
        }
    }
    constexpr u64 titleStoneTags[] = {
        MULTI_CHAR('w_ti_w00'), MULTI_CHAR('w_ti_w01'),
        MULTI_CHAR('w_ti_w02'), MULTI_CHAR('w_ti_w03'),
        MULTI_CHAR('w_ti_w04'), MULTI_CHAR('w_ti_w05'),
        MULTI_CHAR('w_ti_w06'), MULTI_CHAR('w_ti_w07'),
        MULTI_CHAR('w_ti_w08'), MULTI_CHAR('w_ti_w09'),
        MULTI_CHAR('w_mg_w07'), MULTI_CHAR('w_mg_w06'),
        MULTI_CHAR('w_mg_w05'), MULTI_CHAR('w_mg_w04'),
        MULTI_CHAR('w_mgw03'), MULTI_CHAR('w_mgw2'),
        MULTI_CHAR('w_mgw01'), MULTI_CHAR('w_mgw00'),
        MULTI_CHAR('w_msgb1'), MULTI_CHAR('w_msgb0'),
        MULTI_CHAR('w_mg_w12'), MULTI_CHAR('w_mg_w15'),
        MULTI_CHAR('w_mg_w11'), MULTI_CHAR('w_mg_w16'),
        MULTI_CHAR('w_mg_w10'), MULTI_CHAR('w_mg_w17'),
        MULTI_CHAR('w_mg_w09'), MULTI_CHAR('w_mg_w14'),
        MULTI_CHAR('w_mg_w08'), MULTI_CHAR('w_mg_w13'),
        MULTI_CHAR('w_mgkage'),
    };
    for (u64 tag : titleStoneTags) {
        if (J2DPane* pane = menu->mSaveSel.Scr->search(tag)) {
            pane->hide();
        }
    }
}

void apply_save_menu_hd_style(dMenu_save_c* menu) {
    if (menu == nullptr || menu->mSaveSel.Scr == nullptr) {
        return;
    }
    replace_save_menu_background(menu);
    for (u64 tag : {MULTI_CHAR('w_dmhibi'), MULTI_CHAR('w_hibi0')}) {
        if (J2DPane* pane = menu->mSaveSel.Scr->search(tag)) {
            pane->hide();
        }
    }
    hide_wide_file_select_bands(menu->mSaveSel.Scr);
    hide_save_menu_stonework(menu);

    simplify_save_menu_rows(menu);
    simplify_save_menu_numbers(menu);
    style_save_menu_metadata(menu);
    update_save_menu_row_selection(menu);
    for (CPaneMgrAlpha* titleManager : menu->mpHeaderTxtPane) {
        auto* title = titleManager != nullptr ?
            static_cast<J2DTextBox*>(titleManager->getPanePtr()) : nullptr;
        if (title != nullptr) {
            title->setFont(menu->mSaveSel.font[0]);
            title->setFontSize(24.0f, 24.0f);
            title->setCharSpace(-0.5f);
            title->setFontColor(JUtility::TColor(234, 201, 55, 255),
                JUtility::TColor(255, 244, 120, 255));
        }
    }
    add_save_menu_title_rules(menu);
    style_save_select_title(menu);
    style_save_menu_prompts(menu);
    add_save_menu_fixed_prompts(menu);
    style_save_menu_yes_no_buttons(menu);
}
