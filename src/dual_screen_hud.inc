void prepare_dual_screen_full_hud(dMeter2Draw_c* meter);
void prepare_dual_screen_compact_hud(dMeter2Draw_c* meter);
void align_dual_screen_left_hud(dMeter2Draw_c* meter);

bool dual_screen_full_hud_ready() {
    return dual_screen_full_diamond();
}

void prepare_dual_screen_top_hud(dMeter2Draw_c* meter) {
    align_dual_screen_left_hud(meter);
    if (meter->mpButtonCrossParent != nullptr && !dual_screen_show_dpad())
        meter->mpButtonCrossParent->hide();
    if (meter->mpItemB != nullptr && dComIfGs_getBButtonItemKey() == dItemNo_NONE_e) {
        if (auto* sword = first_picture_pane(meter->mpItemB->getPanePtr()))
            sword->setMirror(MIRROR0);
        meter->mpItemB->getPanePtr()->rotate(meter->mpItemB->getSizeX() * 0.5f,
            meter->mpItemB->getSizeY() * 0.5f, ROTATE_Z, 360.0f);
    }
    if (dual_screen_full_hud_ready() && meter->mpButtonParent != nullptr &&
        !touch_controls_active() && !z_item_menu_or_pause_context())
        prepare_dual_screen_full_hud(meter);
    else if (meter->mpButtonParent != nullptr && !touch_controls_active() &&
        !z_item_menu_or_pause_context())
        prepare_dual_screen_compact_hud(meter);
}

// Draw the same archive panes as the single-screen HUD, then restore the
// fork's pose before the companion screen composites those panes.
void align_dual_screen_left_hud(dMeter2Draw_c* meter) {
    apply_hud_pane_transform(HudPaneSlot::Hearts, meter->mpLifeParent, true,
        -22.0f, -19.0f, hud_scales().hearts);
    if (!feature_enabled(Feature::DpadShortcuts) || meter->mpButtonCrossParent == nullptr ||
        meter->mpLifeTexture[0][0] == nullptr) return;
    auto* heart = first_picture_pane(meter->mpLifeTexture[0][0]->getPanePtr());
    if (heart == nullptr) return;
    CPaneMgr coordinates;
    Mtx matrix;
    const f32 heartLeft = coordinates.getGlobalVtx(heart, &matrix, 0, false, 0).x;
    constexpr u64 pieces[] = {MULTI_CHAR('juji_001'), MULTI_CHAR('juji_002'),
        MULTI_CHAR('juji_003'), MULTI_CHAR('juji_004')};
    f32 crossLeft = 0.0f;
    bool haveCross = false;
    for (const u64 tag : pieces) {
        auto* piece = meter->mpScreen->search(tag);
        if (piece == nullptr) continue;
        const f32 left = coordinates.getGlobalVtx(piece, &matrix, 0, false, 0).x;
        crossLeft = haveCross ? std::min(crossLeft, left) : left;
        haveCross = true;
    }
    if (!haveCross) return;
    auto* cross = meter->mpButtonCrossParent->getPanePtr();
    const f32 before = cross->getTranslateX();
    offset_diamond_group<J2DPane, 1>({cross}, heartLeft - crossLeft, 0.0f);
    auto& state = hud_pane_state(HudPaneSlot::DPad);
    state.offsetX += cross->getTranslateX() - before;
    state.appliedX = cross->getTranslateX();
}

struct DualHudPanePose {
    J2DPane* pane;
    J2DPane* parent;
    J2DPane* next;
    JGeometry::TBox2<f32> bounds, globalBounds, clip;
    Mtx localMatrix, globalMatrix;
    f32 sx, sy, tx, ty, rotation, pivotX, pivotY;
    u8 alpha, colorAlpha;
    char rotationAxis;
    bool visible, influencedAlpha;

    explicit DualHudPanePose(J2DPane* p) : pane(p), parent(p->getParentPane()),
        next(p->getNextChildPane()), bounds(p->mBounds),
        globalBounds(p->mGlobalBounds), clip(p->mClipRect),
        sx(p->mScaleX), sy(p->mScaleY), tx(p->mTranslateX), ty(p->mTranslateY),
        rotation(p->mRotateZ), pivotX(p->mRotateOffsetX), pivotY(p->mRotateOffsetY),
        alpha(p->mAlpha), colorAlpha(p->mColorAlpha), rotationAxis(p->mRotAxis), visible(p->mVisible), influencedAlpha(p->mIsInfluencedAlpha) {
        std::memcpy(localMatrix, p->mPositionMtx, sizeof(Mtx));
        std::memcpy(globalMatrix, p->mGlobalMtx, sizeof(Mtx));
    }
    void restore_parent() const {
        if (parent != nullptr && (pane->getParentPane() != parent ||
            pane->getNextChildPane() != next)) parent->insertChild(next, pane);
    }
    void restore() const {
        pane->mBounds = bounds;
        pane->mGlobalBounds = globalBounds;
        pane->mClipRect = clip;
        pane->mScaleX = sx; pane->mScaleY = sy;
        pane->mTranslateX = tx; pane->mTranslateY = ty;
        pane->mRotateZ = rotation;
        pane->mRotAxis = rotationAxis;
        pane->mRotateOffsetX = pivotX; pane->mRotateOffsetY = pivotY;
        pane->mAlpha = alpha; pane->mColorAlpha = colorAlpha;
        pane->mVisible = visible;
        pane->mIsInfluencedAlpha = influencedAlpha;
        std::memcpy(pane->mPositionMtx, localMatrix, sizeof(Mtx));
        std::memcpy(pane->mGlobalMtx, globalMatrix, sizeof(Mtx));
    }
};

struct DualHudPoseScope {
    std::vector<DualHudPanePose> panes;
    explicit DualHudPoseScope(J2DPane* root) { capture(root); }
    void capture(J2DPane* pane) {
        if (pane == nullptr) return;
        panes.emplace_back(pane);
        for (auto* child = pane->getFirstChildPane(); child != nullptr;
             child = child->getNextChildPane()) capture(child);
    }
    ~DualHudPoseScope() {
        for (auto it = panes.rbegin(); it != panes.rend(); ++it) it->restore_parent();
        for (const auto& pose : panes) pose.restore();
    }
};

struct DualRItemScope {
    dMeter2Draw_c* meter;
    dMeter2Draw_c::item_params params;
    std::array<f32, 4> dimensions;
    std::array<const ResTIMG*, 2> textures{};
    explicit DualRItemScope(dMeter2Draw_c* m) : meter(m), params(m->mItemParams[2]),
        dimensions{m->field_0x6ac[2], m->field_0x6b8[2], m->field_0x6c4[2], m->field_0x6d0[2]} {
        auto* primary = as_picture(m->mpItemR->getPanePtr());
        J2DPicture* pictures[] = {primary, m->mpItemXYPane[2]};
        for (int i = 0; i < 2; ++i)
            if (pictures[i] != nullptr && pictures[i]->getTexture(0) != nullptr)
                textures[i] = pictures[i]->getTexture(0)->getTexInfo();
    }
    ~DualRItemScope() {
        meter->mItemParams[2] = params;
        meter->field_0x6ac[2] = dimensions[0]; meter->field_0x6b8[2] = dimensions[1];
        meter->field_0x6c4[2] = dimensions[2]; meter->field_0x6d0[2] = dimensions[3];
        J2DPicture* pictures[] = {as_picture(meter->mpItemR->getPanePtr()), meter->mpItemXYPane[2]};
        for (int i = 0; i < 2; ++i)
            if (textures[i] != nullptr && pictures[i] != nullptr)
                pictures[i]->changeTexture(textures[i], 0);
    }
};

std::unique_ptr<DualHudPoseScope> s_dualHudPose;
std::unique_ptr<DualRItemScope> s_dualRItemScope;

void restore_dual_screen_top_pose() {
    s_dualRItemScope.reset();
    s_dualHudPose.reset();
}

void prepare_dual_screen_compact_hud(dMeter2Draw_c* meter) {
    if (daPy_py_c::checkNowWolf() || meter->mpButtonB == nullptr || meter->mpItemB == nullptr ||
        dComIfGs_getBButtonItemKey() != dItemNo_NONE_e) return;
    auto* button = as_picture(meter->mpScreen->search(MULTI_CHAR('b_btn')));
    auto* sword = first_picture_pane(meter->mpItemB->getPanePtr());
    if (button == nullptr || sword == nullptr) return;
    CPaneMgr coordinates;
    const Vec compactB = coordinates.getGlobalVtxCenter(button, false, 0);
    f32 left, top, right, bottom;
    if (!pane_current_global_bounds(button, left, top, right, bottom)) return;
    const f32 compactWidth = right - left;
    f32 width = 0.0f, height = 0.0f, fullWidth = 0.0f, dx = 0.0f, dy = 0.0f;
    {
        DualHudPoseScope reference(meter->mpScreen);
        restore_archive_face_button_diamond(meter);
        meter->mpButtonParent->scale(meter->mpButtonParent->getInitScaleX() * hud_scales().controllerDiamond,
            meter->mpButtonParent->getInitScaleY() * hud_scales().controllerDiamond);
        const Vec fullB = coordinates.getGlobalVtxCenter(button, false, 0);
        const Vec fullSword = coordinates.getGlobalVtxCenter(sword, false, 0);
        if (!pane_current_global_bounds(button, left, top, right, bottom)) return;
        fullWidth = right - left;
        if (!pane_current_global_bounds(sword, left, top, right, bottom)) return;
        width = right - left;
        height = bottom - top;
        dx = fullSword.x - fullB.x; dy = fullSword.y - fullB.y;
    }
    if (fullWidth <= 0.0f || width <= 0.0f || compactWidth <= 0.0f) return;
    if (!pane_current_global_bounds(sword, left, top, right, bottom) || right <= left) return;
    s_dualHudPose = std::make_unique<DualHudPoseScope>(meter->mpScreen);
    const f32 ratio = compactWidth / fullWidth;
    const f32 scale = width * ratio / (right - left);
    meter->mpItemB->scale(meter->mpItemB->getScaleX() * scale,
        meter->mpItemB->getScaleY() * scale);
    const Vec center = coordinates.getGlobalVtxCenter(sword, false, 0);
    offset_diamond_group<J2DPane, 1>({meter->mpItemB->getPanePtr()},
        compactB.x + dx * ratio - center.x,
        compactB.y + (dy + height * 0.15f) * ratio - center.y);
    if (meter->mpItemBPane != nullptr) meter->mpItemBPane->hide();
    if (meter->mpLightB != nullptr) meter->mpLightB->hide();
}

void prepare_dual_screen_full_hud(dMeter2Draw_c* meter) {
    restore_dual_screen_top_pose();
    s_dualHudPose = std::make_unique<DualHudPoseScope>(meter->mpScreen);
    restore_archive_face_button_diamond(meter);
    fix_xy_hud_bow_combo_layering(meter);
    align_action_text_shadow_layers(meter);
    meter->mpButtonParent->scale(meter->mpButtonParent->getInitScaleX() * hud_scales().controllerDiamond,
        meter->mpButtonParent->getInitScaleY() * hud_scales().controllerDiamond);
    meter->mpButtonParent->show();
    for (int slot = 0; slot < 2; ++slot) {
        if (meter->mpButtonXY[slot] != nullptr) meter->mpButtonXY[slot]->show();
        if (meter->mpItemXY[slot] != nullptr) meter->mpItemXY[slot]->show();
        if (daPy_py_c::checkNowWolf() && meter->mpTextXY[slot] != nullptr)
            meter->mpTextXY[slot]->show();
    }
    if (meter->mpButtonMidona != nullptr) meter->mpButtonMidona->hide();
    const u8 item = dComIfGp_getSelectItem(2);
    if (!daPy_py_c::checkNowWolf() && item != 0 && item != dItemNo_NONE_e &&
        meter->mpItemR != nullptr && meter->mpItemXYPane[2] != nullptr) {
        s_dualRItemScope = std::make_unique<DualRItemScope>(meter);
        change_z_hud_item_texture(meter, item);
        layout_z_hud_item(meter, item);
        update_z_hud_item_alpha(meter,
            s_companionItemUsable == nullptr || s_companionItemUsable(meter, 2));
        meter->mpItemR->show();
        if (auto* parent = meter->mpScreen->search(MULTI_CHAR('item_r_n'))) parent->show();
    }
    apply_wii_u_r_button_art(meter, true);
    HudPaneTransformState shoulderPose;
    apply_hud_pane_transform(shoulderPose, pane_ptr(meter->mpButtonXY[2]), true,
        31.0f, -4.0f, 0.45f);
    if (meter->mpLightXY[2] != nullptr) meter->mpLightXY[2]->hide();
    apply_hud_backing_visibility(meter);
}

void draw_dual_screen_full_hud(dMeter2Draw_c* meter) {
    if (s_dualHudPose == nullptr) return;
    dComIfGp_getCurrentGrafPort()->setup2D();
    draw_wolf_action_icons(meter);
    if (!daPy_py_c::checkNowWolf()) {
        apply_wii_u_item_num_layout(meter);
        for (int slot = 0; slot < 2; ++slot) {
            if (meter->mpItemXY[slot] == nullptr) continue;
            const Vec p0 = meter->mpItemXY[slot]->getPanePtr()->getGlbVtx(0);
            const Vec p3 = meter->mpItemXY[slot]->getPanePtr()->getGlbVtx(3);
            const f32 digit = meter->mItemParams[slot].num_scale * 16.0f;
            for (int i = 0; i < 3; ++i) {
                if (meter->mpItemNumTex[slot][i] == nullptr) continue;
                meter->mpItemNumTex[slot][i]->draw(meter->mItemParams[slot].num_pos_x +
                    (p0.x + p3.x) * 0.5f + digit * i,
                    meter->mItemParams[slot].num_pos_y + (p0.y + p3.y) * 0.5f +
                    meter->mpItemXY[slot]->getSizeY(), digit, digit, false, false, false);
            }
        }
        restore_wii_u_item_num_layout(meter);
        if (s_dualRItemScope != nullptr) {
            const u8 item = dComIfGp_getSelectItem(2);
            const f32 rate = static_cast<f32>(meter->mpItemR->getAlpha()) / 255.0f;
            draw_z_ammo(meter, item, rate);
            draw_z_oil_meter(meter, item, rate);
        }
    }
}

void draw_dual_screen_midna(dMeter2Draw_c* meter) {
    if (!dual_screen_show_midna() || !midna_unlocked() || meter->mpButtonMidona == nullptr) return;
    auto* midna = meter->mpButtonMidona->getPanePtr();
    auto* parent = midna->getParentPane();
    auto* next = midna->getNextChildPane();
    if (parent == nullptr) return;
    DualHudPoseScope pose(meter->mpScreen);
    position_midna_hud(meter, true);
    const u8 alpha = meter->mpLifeParent != nullptr ? meter->mpLifeParent->getAlpha() : 255;
    set_pane_tree_alpha_visible(midna, alpha != 0, alpha);
    update_midna_shoulder_badge(meter, true);
    auto* graf = dComIfGp_getCurrentGrafPort();
    graf->setup2D();
    midna->draw(0.0f, 0.0f, graf, false, true);
    if (auto* badge = meter->mpScreen->search(MULTI_CHAR('hd_mbtn'))) badge->hide();
    if (next != nullptr) parent->insertChild(next, midna);
    else parent->appendChild(midna);
    graf->setup2D();
}

void draw_dual_screen_top_hud(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpButtonA == nullptr || meter->mpButtonParent == nullptr ||
        touch_controls_active() || z_item_menu_or_pause_context()) return;
    const u8 alpha = meter->mpButtonParent->getAlpha();
    if (alpha == 0) return;
    if (dual_screen_full_hud_ready()) draw_dual_screen_full_hud(meter);
    restore_dual_screen_top_pose();
    draw_dual_screen_midna(meter);
    dComIfGp_getCurrentGrafPort()->setup2D();
}
