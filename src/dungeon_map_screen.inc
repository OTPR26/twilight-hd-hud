void position_dmap_floor_cursor(dSelect_cursor_c* cursor) {
    if (s_dmapDrawing == nullptr || cursor == nullptr ||
        cursor != s_dmapDrawing->mpDrawCursor || cursor->mpPane == nullptr ||
        cursor->mpScreen == nullptr) return;
    bool floorTarget = false;
    for (auto* pane = cursor->mpPane; pane != nullptr; pane = pane->getParentPane()) {
        if (pane == s_dmapDrawing->mFloorScreen) floorTarget = true;
    }
    if (!floorTarget) return; // Never change dungeon-item or other menu cursors.
    CPaneMgr manager;
    Mtx matrix;
    const Vec tl = manager.getGlobalVtx(cursor->mpPane, &matrix, 0, false, 0);
    const Vec br = manager.getGlobalVtx(cursor->mpPane, &matrix, 3, false, 0);
    const dungeon_map_layout::Rect row{tl.x, tl.y, br.x - tl.x, br.y - tl.y};
    constexpr u64 tags[] = {MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null')};
    for (int i = 0; i < 4; ++i) {
        const auto point = dungeon_map_layout::cursor_corner(row, i);
        auto* corner = cursor->mpScreen->search(tags[i]);
        if (s_compactDmap.active && corner != nullptr) {
            auto& saved = remember_compact_pane(corner);
            corner->scale(saved.scaleX * s_compactDmap.scale, saved.scaleY * s_compactDmap.scale);
        }
        position_dmap_global_center(corner, point.x, point.y);
    }
}

void hide_large_dmap_base_art(J2DPane* pane, J2DPane* keep) {
    if (pane == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> bounds = pane->getBounds();
    const f32 area = bounds.getWidth() * bounds.getHeight();
    // Only suppress large raster tiles. Large window/null panes are structural
    // parents for the dungeon-item and floor controls and must remain active.
    if (pane != keep && area >= 12000.0f && as_picture(pane) != nullptr) {
        pane->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_large_dmap_base_art(child, keep);
    }
}

void hide_dmap_picture_tree(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }
    if (as_picture(pane) != nullptr) {
        pane->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_dmap_picture_tree(child);
    }
}

void style_dmap_floor_rows(dMenu_DmapBg_c* map) {
    if (map == nullptr || map->mFloorScreen == nullptr) {
        return;
    }
    constexpr u64 floorTags[] = {
        MULTI_CHAR('floor7_n'), MULTI_CHAR('floor0_n'), MULTI_CHAR('floor1_n'),
        MULTI_CHAR('floor2_n'), MULTI_CHAR('floor3_n'), MULTI_CHAR('floor4_n'),
        MULTI_CHAR('floor5_n'), MULTI_CHAR('floor6_n'),
    };
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (frameTexture == nullptr) {
        return;
    }
    for (std::size_t index = 0; index < std::size(floorTags); ++index) {
        J2DPane* row = map->mFloorScreen->search(floorTags[index]);
        if (row == nullptr) {
            continue;
        }
        const u64 number = index == 0 ? 7 : index - 1;
        auto* frame = as_picture(row->search(MULTI_CHAR('f_base_0') + number));
        if (frame == nullptr || (frame->getTexture(0) != nullptr &&
            frame->getTexture(0)->getTexInfo() == frameTexture)) continue;
        if (auto* labelGroup = row->search(MULTI_CHAR('f_tex_0n') | (number << 8))) {
            labelGroup->translate(21.0f, 0.0f);
        }
        for (u64 prefix : {MULTI_CHAR('floor0_0'), MULTI_CHAR('ffoor0_0')}) {
            for (u64 layer = 1; layer <= 3; ++layer) {
                if (auto* text = static_cast<J2DTextBox*>(
                        row->search(prefix | (number << 16) | layer))) {
                    text->setFont(mDoExt_getMesgFont());
                    text->setFontSize(17.0f, 17.0f);
                }
            }
        }
        // Preserve the native child count and order: floor alpha/size animation
        // caches are allocated before this draw hook runs.
        frame->changeTexture(frameTexture, 0);
        frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
        set_neutral_picture_colors(frame);
    }
}

void style_dmap_frame(dMenu_DmapBg_c* map) {
    const auto* texture = resource_texture(s_dmapFrameResource);
    if (map == nullptr || map->mMapScreen[0] == nullptr || texture == nullptr) return;
    auto* center = map->mMapScreen[0]->search(MULTI_CHAR('center_n'));
    auto* parent = center != nullptr ? center->getParentPane() : nullptr;
    auto* frame = as_picture(map->mMapScreen[0]->search(MULTI_CHAR('bs_00_0')));
    if (parent == nullptr || frame == nullptr || frame->getParentPane() != parent) return;
    if (frame->getTexture(0) == nullptr || frame->getTexture(0)->getTexInfo() != texture) {
        const auto bounds = center->getBounds();
        const auto box = dungeon_map_layout::frame_around(
            {bounds.i.x, bounds.i.y, bounds.getWidth(), bounds.getHeight()});
        frame->changeTexture(texture, 0);
        frame->resize(box.width, box.height);
        frame->move(box.x, box.y);
        frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
        set_neutral_picture_colors(frame);
        // Reuse a sibling of center_n without changing the cached native
        // hierarchy. Neither center/scissor bounds nor map geometry changes.
    }
    for (auto* screen : {map->mMapScreen[0], map->mMapScreen[1]}) {
        if (screen == nullptr) continue;
        const auto hide = [screen](u64 tag) {
            if (auto* pane = screen->search(tag)) pane->hide();
        };
        for (u64 tag : {MULTI_CHAR('gold00_0'), MULTI_CHAR('gold00_1'),
                 MULTI_CHAR('bs_00_1')}) hide(tag);
        for (u64 i = 0; i < 12; ++i)
            hide(MULTI_CHAR('mam_k000') + ((i / 10) << 8) + i % 10);
        for (u64 i = 0; i < 8; ++i) {
            hide(MULTI_CHAR('hl_00') + i);
            hide(MULTI_CHAR('sha_00') + i);
        }
    }
}

void add_dmap_edge_rules(dMenu_DmapBg_c* map) {
    // Outside ROOT: its native fade cache has a fixed, preallocated child
    // count. New children there can read past that cache and get wrong alpha.
    auto* root = map->mBaseScreen;
    const auto* texture = resource_texture(s_collectMenuButtonResource);
    if (root == nullptr || texture == nullptr) return;
    using namespace dungeon_map_layout;
    for (int band = 0; band < 2; ++band) {
        const f32 y[] = {band_top[band], band_top[band] + band_edge,
            band_top[band] + band_height - band_edge};
        const f32 heights[] = {band_edge, band_height - 2 * band_edge, band_edge};
        for (int part = 0; part < 3; ++part) {
            const u64 tag = MULTI_CHAR('hd_drl0') + band * 3 + part;
            auto* rule = as_picture(root->search(tag));
            if (rule == nullptr) {
                rule = JKR_NEW J2DPicture(tag,
                    JGeometry::TBox2<f32>(0, y[part], 608, y[part] + heights[part]), texture, nullptr);
                configure_hd_picture(rule);
                const auto color = part == 1 ? JUtility::TColor(105, 107, 74, 255) :
                    JUtility::TColor(197, 196, 145, 255);
                rule->setBlackWhite(color, color);
                root->appendChild(rule);
            }
            rule->setAlpha(static_cast<u8>(255 * std::clamp(map->field_0xd9c, 0.0f, 1.0f)));
        }
    }
}

void position_dmap_native_groups(dMenu_DmapBg_c* map) {
    if (map == nullptr) {
        return;
    }
    for (J2DScreen* screen : {map->mMapScreen[0], map->mMapScreen[1]}) {
        if (screen != nullptr) {
            if (J2DPane* root = screen->search(MULTI_CHAR('ROOT'))) {
                root->translate(kDmapMapOffsetX, kDmapMapOffsetY);
            }
        }
    }
    if (map->mFloorScreen != nullptr) {
        if (J2DPane* root = map->mFloorScreen->search(MULTI_CHAR('ROOT'))) {
            root->scale(kDmapFloorScaleX, 1.0f);
            root->translate(kDmapFloorOffsetX, kDmapFloorOffsetY);
        }
        for (u64 tag : {MULTI_CHAR('rink'), MULTI_CHAR('wolf')}) {
            if (auto* icon = map->mFloorScreen->search(tag)) icon->scale(0.55f, 0.55f);
        }
        auto* menu = dMenu_Dmap_c::myclass;
        if (menu != nullptr && menu->mpDrawBg == map) {
            // Native iconMoveCalc feeds a global delta into a local transform.
            // Re-anchor after the new floor-root transform, retaining native
            // ownership/visibility and the actual player/boss floor indices.
            const int floors[] = {menu->field_0x172, menu->field_0x173};
            CPaneMgr manager;
            for (int i = 0; i < 2; ++i) {
                const int index = floors[i] - menu->mBottomFloor;
                if (index < 0 || index >= 8 || menu->mStayIcon[i] == nullptr) continue;
                auto* target = i == 0 ? menu->mIconLinkPos[index] : menu->mIconBossPos[index];
                if (target == nullptr) continue;
                const Vec position = manager.getGlobalVtxCenter(target->getPanePtr(), false, 0);
                position_dmap_global_center(menu->mStayIcon[i]->getPanePtr(), position.x, position.y);
            }
        }
    }
    if (map->mBaseScreen != nullptr) {
        // Establish the same widescreen transforms that native draw applies.
        // item_s_n is a selection marker, NOT the parent of these three cells.
        map->dMapBgWide();
        constexpr u64 items[] = {MULTI_CHAR('map_n'), MULTI_CHAR('con_n'), MULTI_CHAR('key_n')};
        for (std::size_t i = 0; i < std::size(items); ++i) {
            position_dmap_global_center(map->mBaseScreen->search(items[i]),
                mDoGph_gInf_c::getSafeMinXF() +
                    608.0f * mDoGph_gInf_c::hudAspectScaleUp * 0.872f,
                156.0f + i * 87.0f);
        }
        for (u64 tag : {MULTI_CHAR('gray_map'), MULTI_CHAR('gray_con'),
                 MULTI_CHAR('gray_key')}) {
            if (auto* frame = as_picture(map->mBaseScreen->search(tag))) {
                if (ResTIMG const* texture =
                        resource_texture(s_collectEquipmentFrameResource)) {
                    // Do not call configure_hd_picture on native panes: it
                    // forces show()/alpha=255, exposing unowned items.
                    if (frame->getTexture(0) == nullptr ||
                        frame->getTexture(0)->getTexInfo() != texture) {
                        frame->changeTexture(texture, 0);
                        frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
                        frame->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                            JUtility::TColor(153, 149, 112, 255));
                        frame->setCornerColor(JUtility::TColor(255, 255, 255, 255));
                    }
                }
            }
        }
        for (u64 tag : {MULTI_CHAR('c_btn2'), MULTI_CHAR('w_3dbas1'),
                 MULTI_CHAR('kazari01'), MULTI_CHAR('kazari02'), MULTI_CHAR('spork'),
                 MULTI_CHAR('w_sen01'), MULTI_CHAR('w_sen02'), MULTI_CHAR('w_sen03')}) {
            if (J2DPane* pane = map->mBaseScreen->search(tag)) {
                pane->hide();
            }
        }
    }
    if (map->mButtonScreen != nullptr) {
        for (u64 tag : {MULTI_CHAR('c_btn'), MULTI_CHAR('c_n'),
                 MULTI_CHAR('c_text_s'), MULTI_CHAR('c_text'),
                 MULTI_CHAR('f_text_s'), MULTI_CHAR('f_text')}) {
            if (J2DPane* pane = map->mButtonScreen->search(tag)) {
                pane->hide();
            }
        }
    }
}

void add_dmap_back_hint(dMenu_DmapBg_c* map, float size = 1.0f) {
    if (!feature_enabled(Feature::DpadShortcuts)) return;
    auto* root = map->mBaseScreen; // Outside native fixed-size alpha caches.
    auto* group = root->search(MULTI_CHAR('hd_dbak'));
    if (group == nullptr) {
        const auto* texture = resource_texture(s_dmapBackDpadResource);
        if (texture == nullptr) return;
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_dbak'), JGeometry::TBox2<f32>(0, 0, 110, 24));
        root->appendChild(group);
        auto* icon = JKR_NEW J2DPicture(MULTI_CHAR('hd_ddup'),
            JGeometry::TBox2<f32>(0, 0, 24, 24), texture, nullptr);
        configure_hd_picture(icon);
        group->appendChild(icon);
        auto* text = JKR_NEW J2DTextBox(MULTI_CHAR('hd_dbtx2'),
            JGeometry::TBox2<f32>(31, 0, 110, 24), nullptr, localized_label(MenuLabel::Back).c_str(), 512, HBIND_LEFT, VBIND_CENTER);
        text->setFont(mDoExt_getMesgFont());
        text->setFontSize(12.5f, 12.5f);
        text->setFontColor(JUtility::TColor(238, 238, 232, 255),
            JUtility::TColor(255, 255, 255, 255));
        group->appendChild(text);
    }
    auto* frame = map->mMapScreen[0]->search(MULTI_CHAR('bs_00_0'));
    if (frame == nullptr) { group->hide(); return; }
    CPaneMgr manager;
    Mtx matrix;
    const Vec tl = manager.getGlobalVtx(frame, &matrix, 0, false, 0);
    const Vec br = manager.getGlobalVtx(frame, &matrix, 3, false, 0);
    const auto hint = dungeon_map_layout::back_hint({tl.x, tl.y, br.x - tl.x, br.y - tl.y}, size);
    group->scale(mDoGph_gInf_c::hudAspectScaleDown * size, size);
    position_dmap_global_center(group, hint.x + hint.width / 2, hint.y + hint.height / 2);
    // Match the map fade; dim the shortcut while native loading/dialogue locks input.
    auto* menu = dMenu_Dmap_c::myclass;
    const bool locked = menu != nullptr && menu->isKeyCheck();
    group->setAlpha(static_cast<u8>((locked ? 100 : 255) * std::clamp(map->field_0xd9c, 0.0f, 1.0f)));
    group->show();
}

void apply_dmap_hd_layout(dMenu_DmapBg_c* map) {
    if (map == nullptr || map->mBaseScreen == nullptr ||
        map->mButtonScreen == nullptr) {
        return;
    }

    J2DPane* backgroundPane = map->mBaseScreen->search(MULTI_CHAR('hd_dbg0'));
    if (backgroundPane == nullptr) {
        ResTIMG const* background =
            resource_texture(s_fileSelectBackgroundResource);
        if (background != nullptr) {
            auto* picture = JKR_NEW J2DPicture(MULTI_CHAR('hd_dbg0'),
                JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f),
                background, nullptr);
            configure_hd_picture(picture);
            J2DPane* first = map->mBaseScreen->getFirstChildPane();
            if (first != nullptr) {
                map->mBaseScreen->insertChild(first, picture);
            } else {
                map->mBaseScreen->appendChild(picture);
            }
            backgroundPane = picture;
        }
    }
    hide_large_dmap_base_art(map->mBaseScreen, backgroundPane);
    style_dmap_frame(map);
    add_dmap_edge_rules(map);
    style_dmap_floor_rows(map);
    position_dmap_native_groups(map);
    add_dmap_back_hint(map);

    if (map->mDecorateScreen != nullptr) {
        map->mDecorateScreen->hide();
    }
    if (J2DPane* decoration = map->mButtonScreen->search(MULTI_CHAR('kazari_n'))) {
        decoration->hide();
    }
    if (J2DPane* nativePrompts = map->mButtonScreen->search(MULTI_CHAR('cont_n'))) {
        nativePrompts->hide();
    }

    J2DPane* titleGroup = map->mBaseScreen->search(MULTI_CHAR('hd_dttl'));
    J2DPane* promptGroup = map->mButtonScreen->search(MULTI_CHAR('hd_dbtn'));
    auto* titleSource = static_cast<J2DTextBox*>(
        map->mBaseScreen->search(MULTI_CHAR('f_t_00')));
    if (titleSource == nullptr) {
        titleSource = static_cast<J2DTextBox*>(
            map->mBaseScreen->search(MULTI_CHAR('t_t00')));
    }
    if (titleSource != nullptr) {
        // The native dungeon heading is a wide, shallow picture group. Hide
        // its green panel and stone end caps while retaining the rest of the
        // base screen's structural panes and dungeon-item cells.
        for (J2DPane* ancestor = titleSource->getParentPane();
             ancestor != nullptr; ancestor = ancestor->getParentPane()) {
            const JGeometry::TBox2<f32> bounds = ancestor->getBounds();
            if (bounds.getWidth() >= 260.0f && bounds.getHeight() <= 110.0f) {
                hide_dmap_picture_tree(ancestor);
                break;
            }
        }
    }

    if (titleGroup == nullptr && titleSource != nullptr) {
        titleGroup = JKR_NEW J2DPane(MULTI_CHAR('hd_dttl'),
            JGeometry::TBox2<f32>(0.0f, 0.0f, 270.0f, 58.0f));
        map->mBaseScreen->appendChild(titleGroup);
        if (ResTIMG const* frameTexture =
                resource_texture(s_collectBannerResource)) {
            auto* frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_ddfr'),
                JGeometry::TBox2<f32>(dungeon_map_layout::banner.x, dungeon_map_layout::banner.y,
                    dungeon_map_layout::banner.x + dungeon_map_layout::banner.width,
                    dungeon_map_layout::banner.y + dungeon_map_layout::banner.height),
                frameTexture, nullptr);
            configure_hd_picture(frame);
            titleGroup->appendChild(frame);
            // The same crest used by Collection is also in the dungeon-map
            // archive. Resolve it here so opening Collection first is unnecessary.
            // The old moyou pane supplies a different, streak-like background.
            auto* archive = dComIfGp_getDmapResArchive();
            const auto* pattern = archive != nullptr ? static_cast<ResTIMG const*>(
                archive->getResource('TIMG', "tt_kazari_kani_00.bti")) : nullptr;
            if (pattern != nullptr) {
                for (int tile = 0; tile < dungeon_map_layout::banner_motif_count; ++tile) {
                    const auto bounds = dungeon_map_layout::banner_motif(tile);
                    auto* decoration = JKR_NEW J2DPicture(MULTI_CHAR('hd_ddpa0') + tile,
                        JGeometry::TBox2<f32>(bounds.x, bounds.y,
                            bounds.x + bounds.width, bounds.y + bounds.height), pattern, nullptr);
                    configure_hd_picture(decoration);
                    // White texture background matches the banner's olive fill;
                    // tile edges blend into it rather than forming a second panel.
                    decoration->setBlackWhite(JUtility::TColor(58, 63, 30, 255),
                        JUtility::TColor(30, 34, 13, 255));
                    titleGroup->appendChild(decoration);
                }
            }
        }
        auto* title = JKR_NEW J2DTextBox(MULTI_CHAR('hd_ddtx'),
            JGeometry::TBox2<f32>(30.0f, 23.0f, 238.0f, 49.0f), nullptr,
            "", 64, HBIND_CENTER, VBIND_CENTER);
        title->setFont(mDoExt_getMesgFont());
        title->setFontSize(17.0f, 17.0f);
        title->setCharSpace(0.0f);
        title->setFontColor(JUtility::TColor(242, 242, 235, 255),
            JUtility::TColor(255, 255, 255, 255));
        titleGroup->appendChild(title);
    }

    if (promptGroup == nullptr && titleSource != nullptr) {
        promptGroup = JKR_NEW J2DPane(MULTI_CHAR('hd_dbtn'),
            JGeometry::TBox2<f32>(438.0f, 4.0f, 602.0f, 42.0f));
        map->mButtonScreen->appendChild(promptGroup);
        constexpr const char* labels[] = {"Zoom in", "Back"};
        constexpr u64 textTags[] = {
            MULTI_CHAR('hd_datx'), MULTI_CHAR('hd_dbtx'),
        };
        constexpr u64 iconTags[] = {
            MULTI_CHAR('hd_daic'), MULTI_CHAR('hd_dbic'),
        };
        ResTIMG const* icons[] = {
            menu_face_button_texture(true),
            menu_face_button_texture(false),
        };
        for (std::size_t index = 0; index < 2; ++index) {
            const auto anchor = dungeon_map_layout::prompt_icon(index);
            auto* text = JKR_NEW J2DTextBox(textTags[index],
                JGeometry::TBox2<f32>(0.0f, anchor.y - 11.0f,
                    anchor.x - 15.0f, anchor.y + 11.0f), nullptr, labels[index], 32,
                HBIND_RIGHT, VBIND_CENTER);
            text->setFont(mDoExt_getMesgFont());
            text->setFontSize(12.5f, 12.5f);
            text->setFontColor(JUtility::TColor(238, 238, 232, 255),
                JUtility::TColor(255, 255, 255, 255));
            promptGroup->appendChild(text);
            if (icons[index] != nullptr) {
                auto* icon = JKR_NEW J2DPicture(iconTags[index],
                    JGeometry::TBox2<f32>(anchor.x - 10.0f, anchor.y - 10.0f,
                        anchor.x + 10.0f, anchor.y + 10.0f), icons[index], nullptr);
                configure_hd_picture(icon);
                promptGroup->appendChild(icon);
            }
        }
    }

    if (titleSource != nullptr) {
        if (auto* title = static_cast<J2DTextBox*>(
                map->mBaseScreen->search(MULTI_CHAR('hd_ddtx')))) {
            title->setString(64, text_box_string(titleSource));
        }
        titleSource->hide();
    }
    if (titleGroup != nullptr) {
        titleGroup->move(20.0f, 6.5f);
        titleGroup->scale(mDoGph_gInf_c::hudAspectScaleDown, 1.0f);
        titleGroup->setAlpha(static_cast<u8>(255 * std::clamp(map->field_0xd9c, 0.0f, 1.0f)));
    }
    if (promptGroup != nullptr) {
        update_menu_face_button(map->mButtonScreen, MULTI_CHAR('hd_daic'), true);
        update_menu_face_button(map->mButtonScreen, MULTI_CHAR('hd_dbic'), false);
        // Follow native zoom/info/back text, including a blank disabled
        // action, instead of leaving a hard-coded Zoom in prompt in every mode.
        constexpr u64 promptTags[] = {MULTI_CHAR('hd_datx'), MULTI_CHAR('hd_dbtx')};
        constexpr u64 iconTags[] = {MULTI_CHAR('hd_daic'), MULTI_CHAR('hd_dbic')};
        constexpr u64 nativeTags[][2] = {
            {MULTI_CHAR('font_at'), MULTI_CHAR('cont_at')},
            {MULTI_CHAR('font_bt'), MULTI_CHAR('cont_bt')},
        };
        for (std::size_t i = 0; i < 2; ++i) {
            for (u64 tag : nativeTags[i]) {
                auto* source = static_cast<J2DTextBox*>(map->mButtonScreen->search(tag));
                if (source == nullptr || !source->isVisible()) continue;
                const char* label = text_box_string(source);
                if (auto* text = static_cast<J2DTextBox*>(promptGroup->search(promptTags[i]))) {
                    text->setString(32, label != nullptr ? label : "");
                }
                if (auto* icon = promptGroup->search(iconTags[i])) {
                    if (label != nullptr && label[0] != '\0') icon->show();
                    else icon->hide();
                }
                break;
            }
        }
        promptGroup->move(dungeon_map_layout::prompt_group_x, dungeon_map_layout::prompt_group_y);
        promptGroup->scale(mDoGph_gInf_c::hudAspectScaleDown, 1.0f);
        promptGroup->setAlpha(static_cast<u8>(255 * std::clamp(map->field_0xd9c, 0.0f, 1.0f)));
    }
}

// Field map -----------------------------------------------------------------
