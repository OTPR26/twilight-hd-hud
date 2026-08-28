// Draw-only fitting: leave native map projection, animation caches and input
// geometry untouched between draws, including after a live window resize.
struct CompactPaneState {
    J2DPane* pane = nullptr;
    float x = 0, y = 0, scaleX = 1, scaleY = 1;
};
struct CompactDmapState {
    bool active = false;
    float scale = 1, left = 0, width = 608;
    std::array<CompactPaneState, 24> panes{};
    std::size_t count = 0;
    J2DPicture* background = nullptr;
    JGeometry::TBox2<f32> backgroundBounds{};
    std::array<dMenuMapCommon_c::IconInfo_s, 128> icons{};
    float mirrorCenter = 0;
    J2DTextBox* poeText = nullptr;
    J2DTextBox::TFontSize poeFont{};
    float poeSpacing = 0;
};
CompactDmapState s_compactDmap{};

CompactPaneState& remember_compact_pane(J2DPane* pane) {
    auto& state = s_compactDmap;
    for (std::size_t i = 0; i < state.count; ++i)
        if (state.panes[i].pane == pane) return state.panes[i];
    auto& saved = state.panes.at(state.count++);
    saved = {pane, pane->getTranslateX(), pane->getTranslateY(), pane->getScaleX(), pane->getScaleY()};
    return saved;
}

float compact_dmap_x(float x) {
    return s_compactDmap.active ? map_responsive_layout::x(x, s_compactDmap.left, s_compactDmap.width) : x;
}
float compact_dmap_y(float y) {
    return s_compactDmap.active ? map_responsive_layout::y(y, s_compactDmap.width) : y;
}

void compact_dmap_pane(J2DPane* pane, bool fitX = true) {
    if (pane == nullptr) return;
    CPaneMgr manager;
    const Vec center = manager.getGlobalVtxCenter(pane, false, 0);
    auto& saved = remember_compact_pane(pane);
    const float size = s_compactDmap.scale;
    pane->scale(saved.scaleX * size, saved.scaleY * size);
    position_dmap_global_center(pane, fitX ? compact_dmap_x(center.x) : center.x,
        compact_dmap_y(center.y));
}

void begin_compact_dmap(dMenu_DmapBg_c* map) {
    s_compactDmap = {};
    const float width = 608 * mDoGph_gInf_c::hudAspectScaleUp;
    const float size = map_responsive_layout::scale(width);
    if (map == nullptr || size >= 1) return;
    auto& state = s_compactDmap;
    state.active = true;
    state.scale = size;
    state.width = width;
    state.left = mDoGph_gInf_c::getSafeMinXF();
    for (auto* screen : {map->mMapScreen[0], map->mMapScreen[1], map->mFloorScreen})
        if (screen != nullptr) compact_dmap_pane(screen->search(MULTI_CHAR('ROOT')));

    // The parchment and markers are drawn separately from their J2D frame.
    // Fit their geometry with the identical transform; native scissoring
    // already reads the newly transformed center_n pane.
    state.background = map->mpBackTexture;
    if (state.background != nullptr) {
        state.backgroundBounds = state.background->getBounds();
        const auto& bounds = state.backgroundBounds;
        state.background->move(compact_dmap_x(bounds.i.x), compact_dmap_y(bounds.i.y));
        state.background->resize(bounds.getWidth() * size, bounds.getHeight() * size);
    }
    for (std::size_t i = 0; i < state.icons.size(); ++i) {
        state.icons[i] = map->mIconInfo[i];
        map->mIconInfo[i].pos_x *= size;
        map->mIconInfo[i].pos_y *= size;
        map->mIconInfo[i].scale *= size;
    }
    state.mirrorCenter = map->mCenterPosX;
    map->mCenterPosX = compact_dmap_x(map->mCenterPosX);
    state.poeText = map->mpPoeCountPane;
    if (state.poeText != nullptr) {
        state.poeText->getFontSize(state.poeFont);
        state.poeSpacing = state.poeText->getCharSpace();
        state.poeText->setFontSize(state.poeFont.mSizeX * size, state.poeFont.mSizeY * size);
        state.poeText->setCharSpace(state.poeSpacing * size);
    }
}

void compact_dmap_attachments(dMenu_DmapBg_c* map) {
    const float size = s_compactDmap.scale;
    for (u64 tag : {MULTI_CHAR('map_n'), MULTI_CHAR('con_n'), MULTI_CHAR('key_n')})
        compact_dmap_pane(map->mBaseScreen->search(tag), false); // Already width-anchored.

    CPaneMgr manager;
    Mtx matrix;
    if (auto* title = map->mBaseScreen->search(MULTI_CHAR('hd_dttl'))) {
        const Vec tl = manager.getGlobalVtx(title, &matrix, 0, false, 0);
        auto& saved = remember_compact_pane(title);
        title->scale(saved.scaleX * size, saved.scaleY * size);
        const Vec nowTL = manager.getGlobalVtx(title, &matrix, 0, false, 0);
        const Vec center = manager.getGlobalVtxCenter(title, false, 0);
        position_dmap_global_center(title, center.x + tl.x - nowTL.x, center.y + tl.y - nowTL.y);
    }
    if (auto* prompts = map->mButtonScreen->search(MULTI_CHAR('hd_dbtn'))) {
        const Vec tl = manager.getGlobalVtx(prompts, &matrix, 0, false, 0);
        auto& saved = remember_compact_pane(prompts);
        prompts->scale(saved.scaleX * size, saved.scaleY * size);
        const Vec nowTL = manager.getGlobalVtx(prompts, &matrix, 0, false, 0);
        if (auto* icon = prompts->search(MULTI_CHAR('hd_daic'))) {
            const Vec br = manager.getGlobalVtx(icon, &matrix, 3, false, 0);
            const Vec center = manager.getGlobalVtxCenter(prompts, false, 0);
            const float right = s_compactDmap.left + s_compactDmap.width * 0.963f;
            position_dmap_global_center(prompts, center.x + right - br.x, center.y + tl.y - nowTL.y);
        }
    }
}

void restore_compact_dmap() {
    auto& state = s_compactDmap;
    if (!state.active) return;
    for (std::size_t i = state.count; i > 0; --i) {
        const auto& saved = state.panes[i - 1];
        saved.pane->scale(saved.scaleX, saved.scaleY);
        saved.pane->translate(saved.x, saved.y);
    }
    if (state.background != nullptr) {
        state.background->move(state.backgroundBounds.i.x, state.backgroundBounds.i.y);
        state.background->resize(state.backgroundBounds.getWidth(), state.backgroundBounds.getHeight());
    }
    if (s_dmapDrawing != nullptr) {
        for (std::size_t i = 0; i < state.icons.size(); ++i) {
            s_dmapDrawing->mIconInfo[i].pos_x = state.icons[i].pos_x;
            s_dmapDrawing->mIconInfo[i].pos_y = state.icons[i].pos_y;
            s_dmapDrawing->mIconInfo[i].scale = state.icons[i].scale;
        }
        s_dmapDrawing->mCenterPosX = state.mirrorCenter;
    }
    if (state.poeText != nullptr) {
        state.poeText->setFontSize(state.poeFont);
        state.poeText->setCharSpace(state.poeSpacing);
    }
    state = {};
}
