// Overworld styling is separate from the approved dungeon-map layout.
dMenu_Fmap2DTop_c* s_fmapTopDrawing = nullptr;
J2DTextBox::TFontSize s_fmapPoeFont{};
float s_fmapPoeSpacing = 0;
bool s_fmapPoeScaled = false;

void style_fmap_portal(dSelect_cursor_c* cursor) {
    auto* map = dMenu_Fmap_c::MyClass;
    const auto* texture = resource_texture(s_fmapPortalResource);
    if (!map || !map->mpDraw2DBack || !cursor || !texture ||
        cursor != map->mpDraw2DBack->mpPortalIcon || !cursor->mpScreen ||
        !cursor->mpPaneMgr) return;
    // The native object still owns position, visibility, zoom and animation.
    // Replace only its artwork, outside the fixed native pane/alpha caches.
    hide_dmap_picture_tree(cursor->mpPaneMgr->getPanePtr());
    auto* picture = as_picture(cursor->mpScreen->search(MULTI_CHAR('hd_fport')));
    if (!picture) {
        picture = JKR_NEW J2DPicture(MULTI_CHAR('hd_fport'),
            JGeometry::TBox2<f32>(0, 0, 44, 44), texture, nullptr);
        configure_hd_picture(picture);
        cursor->mpScreen->appendChild(picture);
    }
    const float width = 58 * cursor->mpPaneMgr->getScaleX();
    const float height = 58 * cursor->mpPaneMgr->getScaleY();
    picture->resize(width, height);
    picture->move(cursor->mPositionX - width * .5f, cursor->mPositionY - height * .5f);
    picture->setAlpha(cursor->mpPaneMgr->getPanePtr()->getAlpha());
    if (cursor->mpPaneMgr->getPanePtr()->isVisible()) picture->show();
    else picture->hide();
}

struct FmapPictureState {
    J2DPicture* picture = nullptr;
    float scaleX = 1, scaleY = 1;
    JUtility::TColor black, white;
} s_fmapPicture;

void before_fmap_picture(J2DPicture* picture) {
    auto* menu = dMenu_Fmap_c::MyClass;
    if (!picture || !menu || !menu->mpDraw2DBack || s_fmapPicture.picture) return;
    auto* map = menu->mpDraw2DBack;
    const bool player = picture == map->mPictures[ICON_LINK_e];
    bool parchment = false;
    for (auto* region : map->mpAreaTex) parchment |= picture == region;
    if (!player && !parchment) return;
    s_fmapPicture = {picture, picture->getScaleX(), picture->getScaleY(),
        picture->getBlack(), picture->getWhite()};
    if (player) {
        // The native draw has already set its rotation pivot to the icon's
        // center. Scale around that pivot, retaining position and zoom.
        picture->scale(s_fmapPicture.scaleX * .5f, s_fmapPicture.scaleY * .5f);
    } else {
        auto black = s_fmapPicture.black, white = s_fmapPicture.white;
        // Preserve unreachable black regions and all native fade/alpha values.
        if (white.r + white.g + white.b > 0) {
            // Keep native black points: lifting them created a blue-gray
            // veil over the unselected map. Cool only the brighter tones.
            white.r = white.r * 96 / 100;
            white.g = white.g * 98 / 100;
            picture->setBlackWhite(black, white);
        }
    }
}

void after_fmap_picture(ModContext*, void* args, void*, void*) {
    if (s_fmapPicture.picture != mods::arg<J2DPicture*>(args, 0)) return;
    if (auto* picture = s_fmapPicture.picture) {
        picture->scale(s_fmapPicture.scaleX, s_fmapPicture.scaleY);
        picture->setBlackWhite(s_fmapPicture.black, s_fmapPicture.white);
    }
    s_fmapPicture = {};
}

// Native animation has already selected its colors. Keep the recolored palette
// for the later CI-texture draw; the transform is idempotent for static entries.
HookAction before_fmap_palette(ModContext*, void* args, void*, void*) {
    auto* base = mods::arg<renderingFmap_c*>(args, 0);
    auto* map = dynamic_cast<dMenu_FmapMap_c*>(base);
    if (!map || !map->m_palette) return HOOK_CONTINUE;
    for (int i = 0; i < 27; ++i) {
        auto& p = map->m_palette[i];
        for (auto* c : {&p.field_0x0, &p.field_0x2, &p.field_0x4, &p.field_0x6})
            c->color = map_palette::muted(c->color);
    }
    return HOOK_CONTINUE;
}

bool fmap_accepts_back(dMenu_Fmap_c* map) {
    return map != nullptr && map->isSync() &&
        (map->mProcess == dMenu_Fmap_c::PROC_ALL_MAP ||
         map->mProcess == dMenu_Fmap_c::PROC_REGION_MAP ||
         map->mProcess == dMenu_Fmap_c::PROC_SPOT_MAP ||
         map->mProcess == dMenu_Fmap_c::PROC_PORTAL_WARP_MAP);
}

void add_fmap_back_hint(dMenu_Fmap2DTop_c* map, J2DPane* overlay) {
    auto* group = overlay->search(MULTI_CHAR('hd_fbak'));
    if (group == nullptr) {
        const auto* texture = resource_texture(s_dmapBackDpadResource);
        if (texture == nullptr) return;
        group = JKR_NEW J2DPane(MULTI_CHAR('hd_fbak'), JGeometry::TBox2<f32>(0, 0, 110, 24));
        overlay->appendChild(group); // Outside the native fixed alpha cache.
        auto* icon = JKR_NEW J2DPicture(MULTI_CHAR('hd_fdup'),
            JGeometry::TBox2<f32>(0, 0, 24, 24), texture, nullptr);
        configure_hd_picture(icon);
        group->appendChild(icon);
        auto* text = JKR_NEW J2DTextBox(MULTI_CHAR('hd_fbtx'),
            JGeometry::TBox2<f32>(31, 0, 110, 24), nullptr, "Back", 32, HBIND_LEFT, VBIND_CENTER);
        text->setFont(mDoExt_getMesgFont());
        text->setFontSize(12.5f, 12.5f);
        text->setCharSpace(-0.6f);
        text->setFontColor(JUtility::TColor(238, 238, 232, 255),
            JUtility::TColor(255, 255, 255, 255));
        group->appendChild(text);
    }
    const auto hint = overworld_map_layout::back_hint(mDoGph_gInf_c::getSafeMinXF(),
        608 * mDoGph_gInf_c::hudAspectScaleUp);
    group->move(hint.x, hint.y);
    const float size = map_responsive_layout::scale(608 * mDoGph_gInf_c::hudAspectScaleUp);
    group->scale(size, size);
    // hd_ftop is unscaled; unlike the dungeon's wide base screen it needs
    // no aspect compensation. Both icons therefore render at the same size.
    const float nativeAlpha = map->mpDpadAlpha != nullptr ? map->mpDpadAlpha->getAlphaRate() : 1.0f;
    group->setAlpha(static_cast<u8>((s_inputGate.blocked() ? 100 : 255) *
        std::clamp(nativeAlpha, 0.0f, 1.0f)));
    group->show();
    // This parent owns both the old left/right Back hint and Move/Select area.
    if (auto* nativeHints = map->mpTitleScreen->search(MULTI_CHAR('btn_i_n'))) nativeHints->hide();
}

void position_fmap_viewport(dMenu_Fmap2DBack_c* map) {
    if (map == nullptr || map->mpBaseScreen == nullptr || map->mpMapArea == nullptr ||
        resource_texture(s_fmapFrameResource) == nullptr) return;
    using namespace overworld_map_layout;
    map->fMapBackWide();
    if (map->mpPointParent) {
        map->mpPointParent->scale(g_fmapHIO.mCursorScale * .65f,
            g_fmapHIO.mCursorScale * .65f);
    }
    for (auto* manager : {map->mpMapArea, map->mpMapBlack}) {
        if (manager == nullptr) continue;
        auto* pane = manager->getPanePtr();
        pane->resize(content.width, content.height);
        position_dmap_global_center(pane,
            mDoGph_gInf_c::getSafeMinXF() +
                (content.x + content.width * 0.5f) * mDoGph_gInf_c::hudAspectScaleUp,
            content.y + content.height * 0.5f);
    }
    // Native projection, scrolling and cursor calculations read these same bounds.
    dMeter2Info_set2DWidth(map->getMapScissorAreaSizeX());
    dMeter2Info_set2DHeight(map->getMapScissorAreaSizeY());
    dMeter2Info_set2DPosH(map->getMapScissorAreaLX());
    dMeter2Info_set2DPosV(map->getMapScissorAreaLY());
}

void style_fmap_frame(dMenu_Fmap2DBack_c* map) {
    const auto* texture = resource_texture(s_fmapFrameResource);
    if (map == nullptr || map->mpBaseScreen == nullptr || texture == nullptr) return;
    using namespace overworld_map_layout;
    position_fmap_viewport(map);
    auto* screen = map->mpBaseScreen;
    // Hide just the native frame artwork, never the native hierarchy itself.
    hide_dmap_picture_tree(screen->search(MULTI_CHAR('ROOT')));
    auto* picture = as_picture(screen->search(MULTI_CHAR('hd_fmfr')));
    if (picture == nullptr) {
        picture = JKR_NEW J2DPicture(MULTI_CHAR('hd_fmfr'),
            JGeometry::TBox2<f32>(frame.x, frame.y, frame.x + frame.width,
                frame.y + frame.height), texture, nullptr);
        configure_hd_picture(picture);
        screen->appendChild(picture); // Outside mpBaseRoot's fixed alpha cache.
    }
    picture->setAlpha(static_cast<u8>(255 * std::clamp(map->mAlphaRate, 0.0f, 1.0f)));
}

void refine_fmap_top(dMenu_Fmap2DTop_c* map) {
    if (map == nullptr || map->mpTitleScreen == nullptr) return;
    using namespace overworld_map_layout;
    auto* screen = map->mpTitleScreen;
    auto* group = screen->search(MULTI_CHAR('hd_ftop'));
    if (group == nullptr) return;
    const float left = mDoGph_gInf_c::getSafeMinXF();
    const float width = 608 * mDoGph_gInf_c::hudAspectScaleUp;
    const float size = map_responsive_layout::scale(width);
    const float bannerX = safe_x(left, width, 0.05625f);
    if (auto* titleFrame = screen->search(MULTI_CHAR('hd_ftfr'))) {
        titleFrame->resize(bannerWidth * size, bannerHeight * size);
        titleFrame->move(bannerX, bannerY);
    }
    auto* title = static_cast<J2DTextBox*>(screen->search(MULTI_CHAR('hd_fttx')));
    if (title != nullptr) {
        title->setFont(mDoExt_getRubyFont());
        title->setFontSize(22 * size, 22 * size);
        title->setCharSpace(-0.35f * size);
        set_text_binding(title, HBIND_CENTER, VBIND_CENTER);
        title->resize((bannerWidth - 16) * size, bannerTitleHeight * size);
        title->move(bannerX + 8 * size, bannerY + bannerTitleTop * size);
    }
    // The native subtitle is separate from the province/title line. Keep its
    // localized area/portal name while replacing the old frame hierarchy.
    constexpr u64 areaTags[] = {
        MULTI_CHAR('f_name_1'), MULTI_CHAR('f_name3'), MULTI_CHAR('f_name2'),
        MULTI_CHAR('i_name_s'), MULTI_CHAR('i_name'), MULTI_CHAR('i_name1'),
    };
    J2DTextBox* areaSource = nullptr;
    for (u64 tag : areaTags) {
        auto* candidate = static_cast<J2DTextBox*>(screen->search(tag));
        // Native setup initializes only the active language's labels with
        // the Ruby font. Inactive layout placeholders can be nonempty too.
        if (candidate != nullptr && candidate->isVisible() &&
            candidate->getFont() == mDoExt_getRubyFont()) {
            areaSource = candidate;
            break; // Keep an empty active label empty; never use a placeholder.
        }
    }
    auto* area = static_cast<J2DTextBox*>(screen->search(MULTI_CHAR('hd_farea')));
    if (area == nullptr) {
        area = JKR_NEW J2DTextBox(MULTI_CHAR('hd_farea'),
            JGeometry::TBox2<f32>(0, 0, 426, 18), nullptr, "", 256,
            HBIND_LEFT, VBIND_CENTER);
        area->setFont(mDoExt_getRubyFont());
        area->setFontColor(JUtility::TColor(220, 215, 137, 255),
            JUtility::TColor(244, 237, 169, 255));
        group->appendChild(area);
    }
    const char* areaName = text_box_string(areaSource);
    if (areaSource != nullptr) area->setFont(areaSource->getFont());
    area->setString(256, areaName != nullptr ? areaName : "");
    area->setFontSize(areaNameFontSize * size, areaNameFontSize * size);
    area->setCharSpace(-0.25f * size);
    // Reserve the right side for the Poe counter rather than overlapping it.
    area->resize((content.width - 105) * size, 24 * size);
    area->move(safe_x(left, width, areaNameX / 608.0f), areaNameY);
    if (areaName != nullptr && areaName[0] != '\0') area->show();
    else area->hide();
    if (const auto* motif = resource_texture(s_fmapBannerPatternResource)) {
        for (int i = 0; i < 5; ++i) {
            auto* picture = as_picture(group->search(MULTI_CHAR('hd_fpat0') + i));
            if (picture == nullptr) {
                picture = JKR_NEW J2DPicture(MULTI_CHAR('hd_fpat0') + i,
                    JGeometry::TBox2<f32>(0, 0, 44, 22), motif, nullptr);
                configure_hd_picture(picture);
                picture->setBlackWhite(JUtility::TColor(58, 63, 30, 255),
                    JUtility::TColor(30, 34, 13, 255));
                if (title != nullptr) group->insertChild(title, picture);
                else group->appendChild(picture);
            }
            picture->resize(44 * size, 22 * size);
            picture->move(bannerX + (6 + 44 * i) * size, bannerY + 10 * size);
        }
    }
    if (auto* controls = screen->search(MULTI_CHAR('hd_fctl'))) controls->move(0, 0);
    constexpr u64 labels[] = {MULTI_CHAR('hd_fpt0'), MULTI_CHAR('hd_fpt1'), MULTI_CHAR('hd_fpt2')};
    constexpr u64 icons[] = {MULTI_CHAR('hd_fri0'), MULTI_CHAR('hd_fai1'), MULTI_CHAR('hd_fbi2')};
    for (int i = 0; i < 3; ++i) {
        const float x = safe_x(left, width, i == 0 ? 0.181f : i == 1 ? 0.946f : 0.922f);
        const float y = i == 0 ? portalY : i == 1 ? zoomAY : zoomBY;
        const float extent = (i == 0 ? 44 : 28) * size;
        if (auto* icon = screen->search(icons[i])) {
            icon->resize(extent, extent);
            icon->move(x - extent * 0.5f, y - extent * 0.5f);
        }
        if (auto* text = static_cast<J2DTextBox*>(screen->search(labels[i]))) {
            text->setFont(mDoExt_getMesgFont());
            text->setFontSize(13 * size, 13 * size);
            text->setCharSpace(-0.6f * size);
            text->resize(115 * size, 22 * size);
            text->move(i == 0 ? x + 24 * size : x - 130 * size, y - 11 * size);
        }
    }
    // Match the map-only physical L/LB/L1 portal input mapping.
    set_menu_face_button_texture(screen, MULTI_CHAR('hd_fri0'), styled_l_button_texture());
    add_fmap_back_hint(map, group);
    for (int band = 0; band < 2; ++band) {
        const float y = band == 0 ? topRule : bottomRule;
        for (int part = 0; part < 3; ++part) {
            auto* rule = as_picture(group->search(MULTI_CHAR('hd_frl0') + band * 3 + part));
            if (rule == nullptr) {
                const auto* texture = resource_texture(s_collectMenuButtonResource);
                if (texture == nullptr) continue;
                rule = JKR_NEW J2DPicture(MULTI_CHAR('hd_frl0') + band * 3 + part,
                    JGeometry::TBox2<f32>(0, 0, 608, 1), texture, nullptr);
                configure_hd_picture(rule);
                const auto color = part == 1 ? JUtility::TColor(105, 107, 74, 255) :
                    JUtility::TColor(197, 196, 145, 255);
                rule->setBlackWhite(color, color);
                group->insertChild(group->getFirstChildPane(), rule);
            }
            rule->resize(mDoGph_gInf_c::getWidthF(), part == 1 ? 2.6f : 0.6f);
            rule->move(mDoGph_gInf_c::getMinXF(), y + (part == 0 ? 0 : part == 1 ? 0.6f : 3.2f));
        }
    }
    group->setAlpha(static_cast<u8>(255 * std::clamp(map->mAlphaRate, 0.0f, 1.0f)));
}

HookAction before_fmap_top_draw(ModContext*, void* args, void*, void*) {
    begin_map_font();
    s_fmapTopDrawing = mods::arg<dMenu_Fmap2DTop_c*>(args, 0);
    const float size = map_responsive_layout::scale(608 * mDoGph_gInf_c::hudAspectScaleUp);
    s_fmapPoeScaled = size < 1 && s_fmapTopDrawing != nullptr && s_fmapTopDrawing->mpPoeCountPane != nullptr;
    if (s_fmapPoeScaled) {
        auto* text = s_fmapTopDrawing->mpPoeCountPane;
        text->getFontSize(s_fmapPoeFont);
        s_fmapPoeSpacing = text->getCharSpace();
        text->setFontSize(s_fmapPoeFont.mSizeX * size, s_fmapPoeFont.mSizeY * size);
        text->setCharSpace(s_fmapPoeSpacing * size);
    }
    return HOOK_CONTINUE;
}
void after_fmap_top_draw(ModContext*, void*, void*, void*) {
    end_map_font();
    if (s_fmapPoeScaled) {
        s_fmapTopDrawing->mpPoeCountPane->setFontSize(s_fmapPoeFont);
        s_fmapTopDrawing->mpPoeCountPane->setCharSpace(s_fmapPoeSpacing);
    }
    s_fmapPoeScaled = false;
    s_fmapTopDrawing = nullptr;
}
