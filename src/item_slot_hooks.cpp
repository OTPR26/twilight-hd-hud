#include "config.hpp"
#include "hud_layout.hpp"
#include "service_imports.hpp"

#include "global.h"
#include "Z2AudioLib/Z2AudioMgr.h"
#include "Z2AudioLib/Z2SeMgr.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_kantera_icon_meter.h"
#include "d/d_item.h"
#include "d/d_item_data.h"
#include "d/d_meter_HIO.h"
#include "d/d_meter2.h"
#include "d/d_meter2_info.h"
#include "d/d_menu_window.h"
#include "d/d_menu_item_explain.h"
#include "d/d_pane_class.h"
#include "d/d_msg_object.h"
#include "JSystem/J2DGraph/J2DPane.h"
#include "JSystem/J2DGraph/J2DScreen.h"
#include "JSystem/J2DGraph/J2DPicture.h"
#include "JSystem/J2DGraph/J2DTextBox.h"
#define private public
#include "d/d_menu_ring.h"
#include "d/d_meter_map.h"
#include "d/d_meter2_draw.h"
#undef private
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_ext.h"
#include "mods/hook.hpp"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/resource.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace twilight_hd_hud {
namespace {

constexpr u8 kZItemSlot = SELECT_ITEM_DOWN;
constexpr int kExtendedSelectItemCount = 3;
constexpr int kSelectItemNotFound = 3;
constexpr int kItemProcBootsEquip = 1;
constexpr u32 kFirstNativeFaceButton = 0;
constexpr u32 kLastNativeFaceButton = 3;

struct WolfIconLayout {
    f32 offsetX;
    f32 offsetY;
    f32 size;
    f32 opacity;
};

constexpr WolfIconLayout kDigIconLayout = {-21.0f, 8.0f, 20.0f, 255.0f};
constexpr WolfIconLayout kSenseIconLayout = {-25.0f, -22.0f, 16.0f, 255.0f};
constexpr WolfIconLayout kAttackIconLayout = {3.0f, -37.0f, 22.0f, 220.0f};

DEFINE_HOOK(&dComIfGp_getSelectItem, GetSelectItemHook);
DEFINE_HOOK(&dComIfGp_setSelectItem, SetSelectItemHook);
DEFINE_HOOK(&dMenu_Ring_c::_create, RingCreateHook);
DEFINE_HOOK(&dMenu_Ring_c::_delete, RingDeleteHook);
DEFINE_HOOK(&dMenu_Ring_c::_draw, RingDrawHook);
DEFINE_HOOK(&dMenu_Ring_c::setActiveCursor, RingSetActiveCursorHook);
DEFINE_HOOK(&dMenu_Ring_c::isMixItemOn, RingIsMixItemOnHook);
DEFINE_HOOK(&dMenu_Ring_c::isMixItemOff, RingIsMixItemOffHook);
DEFINE_HOOK(&dMeter2Draw_c::draw, MeterDrawHook);
DEFINE_HOOK(&dMeter2_c::moveButtonCross, MeterMoveButtonCrossHook);
DEFINE_HOOK(&dMeter2Draw_c::drawKantera, MeterDrawKanteraHook);
DEFINE_HOOK(&dMeter2Draw_c::drawOxygen, MeterDrawOxygenHook);
DEFINE_HOOK(&dMeter2Draw_c::setButtonIconMidonaAlpha, MeterMidnaAlphaHook);
DEFINE_HOOK(&dMeterMap_c::draw, MeterMapDrawHook);
DEFINE_HOOK(&daAlink_c::midnaTalkTrigger, MidnaTalkTriggerHook);
DEFINE_HOOK(&mDoCPd_c::read, PadReadHook);
DEFINE_HOOK(&daAlink_c::checkItemButtonChange, CheckItemButtonChangeHook);
DEFINE_HOOK(&daAlink_c::checkItemChangeFromButton, CheckItemChangeFromButtonHook);
DEFINE_HOOK(&daAlink_c::checkSetItemTrigger, CheckSetItemTriggerHook);
DEFINE_HOOK(&daAlink_c::checkItemSetButton, CheckItemSetButtonHook);
DEFINE_HOOK(&daAlink_c::setHeavyBoots, SetHeavyBootsHook);
DEFINE_HOOK(&daAlink_c::execute, PlayerExecuteHook);

struct PendingAssign {
    dMenu_Ring_c* ring = nullptr;
    u8 targetSlot = dItemNo_NONE_e;
    u8 selectedSlot = dItemNo_NONE_e;
    u8 oldTargetSlot = dItemNo_NONE_e;
    u8 oldTargetMix = dItemNo_NONE_e;
    bool active = false;
};

PendingAssign s_pendingAssign;

struct RingZButtonPrompt {
    dMenu_Ring_c* ring = nullptr;
    J2DScreen* screen = nullptr;
    CPaneMgr* button = nullptr;
};

RingZButtonPrompt s_ringZPrompt;
alignas(32) u8 s_zHudItemTexBuf[2][2][0xC00];
u8 s_zHudItemTexPage = 0;
u8 s_zHudLastItem = dItemNo_NONE_e;
dMeter2Draw_c* s_zHudItemMeter = nullptr;
J2DPicture* s_zItemNumTex[3] = {};
J2DPicture* s_rupeeDigitTex[4] = {};
J2DPicture* s_wiiURButtonPicture = nullptr;
ResourceBuffer s_senseIconResource = RESOURCE_BUFFER_INIT;
J2DPicture* s_senseIconPicture = nullptr;
ResourceBuffer s_digIconResource = RESOURCE_BUFFER_INIT;
J2DPicture* s_digIconPicture = nullptr;
ResourceBuffer s_attackIconResource = RESOURCE_BUFFER_INIT;
J2DPicture* s_attackIconPicture = nullptr;
ResourceBuffer s_faceButtonAResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_faceButtonBResource = RESOURCE_BUFFER_INIT;
dMeter2Draw_c* s_wiiURButtonMeter = nullptr;
dKantera_icon_c* s_zOilMeter = nullptr;
daAlink_c* s_zHeavyBootsGuardLink = nullptr;
bool s_zHeavyBootsManualToggleOff = false;
bool s_zHeavyBootsWaitRelease = false;
u8 s_zHeavyBootsGuardFrames = 0;
bool s_dpadMidnaHeld = false;
bool s_dpadMidnaTrig = false;
u32 s_checkedControllerVid = 0;
u32 s_checkedControllerPid = 0;
bool s_checkedControllerBindings = false;

struct HudPaneTransformState {
    J2DPane* pane = nullptr;
    f32 offsetX = 0.0f;
    f32 offsetY = 0.0f;
    f32 scale = 1.0f;
    f32 appliedX = 0.0f;
    f32 appliedY = 0.0f;
    f32 appliedScaleX = 1.0f;
    f32 appliedScaleY = 1.0f;
    bool active = false;
};

enum class HudPaneSlot : std::size_t {
    ButtonZ,
    TextZ,
    DPad,
    Hearts,
    RupeeKey,
    Rupee0,
    Rupee1,
    Rupee2,
    Count,
};

std::array<HudPaneTransformState, static_cast<std::size_t>(HudPaneSlot::Count)>
    s_wiiUHudPaneTransforms;

struct MinimapTransformState {
    dMeterMap_c* map = nullptr;
    f32 drawPosX = 0.0f;
    f32 drawPosY = 0.0f;
    f32 sizeW = 0.0f;
    f32 sizeH = 0.0f;
    bool active = false;
};

MinimapTransformState s_wiiUMinimapTransform;

struct ItemNumTransformState {
    dMeter2Draw_c* meter = nullptr;
    f32 posX[2] = {};
    f32 posY[2] = {};
    f32 scale[2] = {};
    bool active = false;
};

ItemNumTransformState s_wiiUItemNumTransform;

bool consume_touch_midna_trigger() {
    return false;
}

bool z_item_menu_or_pause_context();
bool midna_unlocked();

ResTIMG* z_hud_item_tex(const u8 page, const u8 layer) {
    return reinterpret_cast<ResTIMG*>(s_zHudItemTexBuf[page][layer]);
}

u8 hud_texture_item(u8 itemNo) {
    return itemNo == dItemNo_LIGHT_ARROW_e ? dItemNo_BOW_e : itemNo;
}

u8 hud_layout_item(u8 itemNo) {
    return itemNo == dItemNo_HAWK_ARROW_e ? dItemNo_BOW_e : hud_texture_item(itemNo);
}

u8 clamp_hud_alpha(const f32 alpha) {
    if (alpha <= 0.0f) {
        return 0;
    }
    if (alpha >= 255.0f) {
        return 255;
    }
    return static_cast<u8>(alpha);
}

void hide_pane_tree(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    pane->hide();
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        hide_pane_tree(child);
    }
}

void show_pane_tree(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    pane->show();
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        show_pane_tree(child);
    }
}

void set_pane_tree_alpha_visible(J2DPane* pane, const bool visible, const u8 alpha) {
    if (pane == nullptr) {
        return;
    }

    pane->setAlpha(alpha);
    if (visible) {
        pane->show();
    } else {
        pane->hide();
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        set_pane_tree_alpha_visible(child, visible, alpha);
    }
}

void set_pane_influenced_alpha_tree(J2DPane* pane, const bool influenced) {
    if (pane == nullptr) {
        return;
    }

    pane->setInfluencedAlpha(influenced, true);
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        set_pane_influenced_alpha_tree(child, influenced);
    }
}

void show_pane_parents(J2DPane* pane) {
    for (J2DPane* parent = pane; parent != nullptr; parent = parent->getParentPane()) {
        parent->show();
    }
}

J2DPicture* as_picture(J2DPane* pane) {
    if (pane == nullptr || pane->getTypeID() != 18) {
        return nullptr;
    }

    return static_cast<J2DPicture*>(pane);
}

J2DPicture* first_picture_pane(J2DPane* pane) {
    if (J2DPicture* picture = as_picture(pane)) {
        return picture;
    }

    if (pane == nullptr) {
        return nullptr;
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        if (J2DPicture* picture = first_picture_pane(child)) {
            return picture;
        }
    }

    return nullptr;
}

void resize_pane_around_center(J2DPane* pane, const f32 width, const f32 height) {
    const JGeometry::TBox2<f32> bounds = pane->getBounds();
    const f32 centerX = bounds.i.x + bounds.getWidth() * 0.5f;
    const f32 centerY = bounds.i.y + bounds.getHeight() * 0.5f;

    pane->resize(width, height);
    pane->move(centerX - width * 0.5f, centerY - height * 0.5f);
}

bool nearly_equal(const f32 lhs, const f32 rhs) {
    return std::fabs(lhs - rhs) < 0.01f;
}

HudPaneTransformState& hud_pane_state(const HudPaneSlot slot) {
    return s_wiiUHudPaneTransforms[static_cast<std::size_t>(slot)];
}

J2DPane* pane_ptr(CPaneMgrAlpha* pane) {
    return pane != nullptr ? pane->getPanePtr() : nullptr;
}

void apply_hud_pane_transform(HudPaneTransformState& state, J2DPane* pane, const bool enabled,
    const f32 offsetX, const f32 offsetY, const f32 scale) {
    if (pane == nullptr || scale <= 0.0f) {
        state = {};
        return;
    }

    if (state.active && state.pane == pane &&
        nearly_equal(pane->getTranslateX(), state.appliedX) &&
        nearly_equal(pane->getTranslateY(), state.appliedY) &&
        nearly_equal(pane->getScaleX(), state.appliedScaleX) &&
        nearly_equal(pane->getScaleY(), state.appliedScaleY))
    {
        const f32 appliedScale = state.scale > 0.0f ? state.scale : 1.0f;
        pane->translate(pane->getTranslateX() - state.offsetX,
            pane->getTranslateY() - state.offsetY);
        pane->scale(pane->getScaleX() / appliedScale, pane->getScaleY() / appliedScale);
    }

    if (!enabled) {
        state = {};
        return;
    }

    const f32 baseX = pane->getTranslateX();
    const f32 baseY = pane->getTranslateY();
    const f32 baseScaleX = pane->getScaleX();
    const f32 baseScaleY = pane->getScaleY();
    pane->translate(baseX + offsetX, baseY + offsetY);
    pane->scale(baseScaleX * scale, baseScaleY * scale);

    state = {
        .pane = pane,
        .offsetX = offsetX,
        .offsetY = offsetY,
        .scale = scale,
        .appliedX = pane->getTranslateX(),
        .appliedY = pane->getTranslateY(),
        .appliedScaleX = pane->getScaleX(),
        .appliedScaleY = pane->getScaleY(),
        .active = true,
    };
}

void apply_hud_pane_transform(const HudPaneSlot slot, CPaneMgr* pane, const bool enabled,
    const f32 offsetX, const f32 offsetY, const f32 scale) {
    apply_hud_pane_transform(hud_pane_state(slot), pane_ptr(pane), enabled, offsetX, offsetY,
        scale);
}

void apply_hud_pane_transform(const HudPaneSlot slot, CPaneMgrAlpha* pane, const bool enabled,
    const f32 offsetX, const f32 offsetY, const f32 scale) {
    apply_hud_pane_transform(hud_pane_state(slot), pane_ptr(pane), enabled, offsetX, offsetY,
        scale);
}

void restore_archive_pane(CPaneMgr* pane) {
    if (pane == nullptr) {
        return;
    }
    pane->scale(pane->getInitScaleX(), pane->getInitScaleY());
    // Preserve the archive-authored Wii U diamond while moving the complete
    // face-button group toward the right safe-area edge.
    pane->paneTrans(30.0f, 0.0f);
}

void enable_linear_filtering(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane)) {
        for (u8 index = 0; index < picture->getTextureCount(); ++index) {
            JUTTexture* texture = picture->getTexture(index);
            if (texture != nullptr && texture->getTexInfo() != nullptr) {
                ResTIMG* info = const_cast<ResTIMG*>(texture->getTexInfo());
                info->minFilter = GX_LINEAR;
                info->magFilter = GX_LINEAR;
            }
        }
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        enable_linear_filtering(child);
    }
}

void align_action_text_shadow_group(dMeter2Draw_c* meter, const u64 foregroundTag,
    const std::array<u64, 4>& shadowTags) {
    if (meter == nullptr || meter->mpScreen == nullptr) {
        return;
    }

    J2DPane* foreground = meter->mpScreen->search(foregroundTag);
    if (foreground == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> foregroundBounds = foreground->getBounds();
    constexpr std::array<Vec, 4> offsets = {{
        {-0.5f, -0.5f, 0.0f},
        { 0.5f, -0.5f, 0.0f},
        {-0.5f,  0.5f, 0.0f},
        { 0.5f,  0.5f, 0.0f},
    }};
    const bool foregroundVisible = foreground->isVisible() && foreground->getAlpha() != 0;
    for (std::size_t index = 0; index < shadowTags.size(); ++index) {
        if (J2DPane* shadow = meter->mpScreen->search(shadowTags[index])) {
            shadow->move(foregroundBounds.i.x + offsets[index].x,
                foregroundBounds.i.y + offsets[index].y);
            shadow->setAlpha(foreground->getAlpha());
            if (foregroundVisible) {
                shadow->show();
            } else {
                shadow->hide();
            }
        }
    }
}

void align_action_text_shadow_layers(dMeter2Draw_c* meter) {
    // TPHD uses a tight black stroke around the glyph rather than an offset
    // drop shadow. Four half-pixel copies reproduce that thin outline.
    align_action_text_shadow_group(meter, MULTI_CHAR('cont_at'), {{
        MULTI_CHAR('cont_at1'), MULTI_CHAR('cont_at2'),
        MULTI_CHAR('cont_at3'), MULTI_CHAR('cont_at4'),
    }});
    align_action_text_shadow_group(meter, MULTI_CHAR('cont_bt'), {{
        MULTI_CHAR('cont_bt1'), MULTI_CHAR('cont_bt2'),
        MULTI_CHAR('cont_bt3'), MULTI_CHAR('cont_bt4'),
    }});
    align_action_text_shadow_group(meter, MULTI_CHAR('cont_xt5'), {{
        MULTI_CHAR('cont_xt1'), MULTI_CHAR('cont_xt2'),
        MULTI_CHAR('cont_xt3'), MULTI_CHAR('cont_xt4'),
    }});
    align_action_text_shadow_group(meter, MULTI_CHAR('cont_yt5'), {{
        MULTI_CHAR('cont_yt1'), MULTI_CHAR('cont_yt2'),
        MULTI_CHAR('cont_yt3'), MULTI_CHAR('cont_yt4'),
    }});
    align_action_text_shadow_group(meter, MULTI_CHAR('cont_rt'), {{
        MULTI_CHAR('cont_zt1'), MULTI_CHAR('cont_zt2'),
        MULTI_CHAR('cont_zt3'), MULTI_CHAR('cont_zt4'),
    }});
}

ResTIMG const* archive_texture(const char* textureName) {
    return textureName != nullptr ? static_cast<ResTIMG const*>(
        dComIfGp_getMain2DArchive()->getResource('TIMG', textureName)) : nullptr;
}

ResTIMG const* resource_texture(const ResourceBuffer& resource) {
    return resource.data != nullptr && resource.size >= sizeof(ResTIMG) ?
        static_cast<ResTIMG const*>(resource.data) : nullptr;
}

void set_face_button_texture(dMeter2Draw_c* meter, const u64 pictureTag,
    ResTIMG const* texture) {
    if (meter == nullptr || meter->mpScreen == nullptr || texture == nullptr) {
        return;
    }
    J2DPicture* picture = as_picture(meter->mpScreen->search(pictureTag));
    if (picture == nullptr) {
        return;
    }
    const JGeometry::TBox2<f32> originalBounds = picture->getBounds();
    picture->changeTexture(texture, 0);
    // Preserve the archive-authored pane geometry. The replacement uses the
    // same native dimensions. Explicitly restore a single full-image texture
    // span as some archive panes retain atlas-style coordinates when their
    // texture is replaced at runtime.
    picture->resize(originalBounds.getWidth(), originalBounds.getHeight());
    picture->move(originalBounds.i.x, originalBounds.i.y);
    picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
    // Texture selection must not override the game's visibility or alpha.
    // Those values drive the wolf/human transformation timing and prompt
    // fades. Replacing the texture can also restore its archive filtering, so
    // explicitly keep the small button lettering bilinear-filtered.
    if (JUTTexture* appliedTexture = picture->getTexture(0);
        appliedTexture != nullptr && appliedTexture->getTexInfo() != nullptr)
    {
        ResTIMG* info = const_cast<ResTIMG*>(appliedTexture->getTexInfo());
        info->minFilter = GX_LINEAR;
        info->magFilter = GX_LINEAR;
    }
}

void apply_button_layout_preference(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    const ButtonLayout layout = button_layout();
    ResTIMG const* buttonA = resource_texture(s_faceButtonAResource);
    ResTIMG const* buttonB = resource_texture(s_faceButtonBResource);
    if (buttonA == nullptr) {
        buttonA = archive_texture("wiiu_a.bti");
    }
    if (buttonB == nullptr) {
        buttonB = archive_texture("wiiu_b.bti");
    }

    if (layout == ButtonLayout::Nintendo) {
        set_face_button_texture(meter, MULTI_CHAR('a_btn'), buttonA);
        set_face_button_texture(meter, MULTI_CHAR('b_btn'), buttonB);
        set_face_button_texture(meter, MULTI_CHAR('x_btn'), archive_texture("wiiu_x.bti"));
        set_face_button_texture(meter, MULTI_CHAR('y_btn'), archive_texture("wiiu_y.bti"));
        return;
    }

    if (layout == ButtonLayout::Universal) {
        ResTIMG const* blank = archive_texture("tt_zelda_button_ab_maru.bti");
        set_face_button_texture(meter, MULTI_CHAR('a_btn'), blank);
        set_face_button_texture(meter, MULTI_CHAR('b_btn'), blank);
        set_face_button_texture(meter, MULTI_CHAR('x_btn'), blank);
        set_face_button_texture(meter, MULTI_CHAR('y_btn'), blank);
    } else if (layout == ButtonLayout::Xbox) {
        // Keep every action group in its authored location and change only the
        // printed letters: south Attack becomes A, east action becomes B,
        // north becomes Y, and west becomes X.
        set_face_button_texture(meter, MULTI_CHAR('a_btn'), buttonB);
        set_face_button_texture(meter, MULTI_CHAR('b_btn'), buttonA);
        set_face_button_texture(meter, MULTI_CHAR('x_btn'), archive_texture("wiiu_y.bti"));
        set_face_button_texture(meter, MULTI_CHAR('y_btn'), archive_texture("wiiu_x.bti"));
    }
}

void restore_archive_face_button_diamond(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    restore_archive_pane(meter->mpButtonA);
    restore_archive_pane(meter->mpTextA);
    if (meter->mpTextA != nullptr) {
        meter->mpTextA->paneTrans(10.0f, -4.0f);
        meter->mpTextA->scale(meter->mpTextA->getInitScaleX() * 1.25f,
            meter->mpTextA->getInitScaleY() * 1.25f);
    }
    restore_archive_pane(meter->mpButtonB);
    restore_archive_pane(meter->mpItemB);
    restore_archive_pane(meter->mpLightB);
    restore_archive_pane(meter->mpTextB);
    // These small circular textures are enlarged by the widescreen HUD. Force
    // bilinear sampling on every A/B layer so their alpha edges stay smooth.
    enable_linear_filtering(pane_ptr(meter->mpButtonA));
    enable_linear_filtering(pane_ptr(meter->mpButtonB));

    // The archive's separate A/B light panes form long offset streaks once the
    // clean Wii U discs are repositioned. The base discs already include their
    // intended edge shading, so suppress only these obsolete glow layers.
    if (meter->mpScreen != nullptr) {
        if (J2DPane* aLight = meter->mpScreen->search(MULTI_CHAR('a_btn_l'))) {
            aLight->hide();
        }
        if (J2DPane* bLight = meter->mpScreen->search(MULTI_CHAR('b_btn_l'))) {
            bLight->hide();
        }
    }

    // Stock drawing updates leave the B-item art at the former Y-slot anchor.
    // Move only the item layers onto the archive-authored B prompt; the button
    // diamond itself must remain untouched.
    if (meter->mpItemB != nullptr) {
        meter->mpItemB->paneTrans(50.0f, 19.0f);
        meter->mpItemB->scale(meter->mpItemB->getInitScaleX() * 1.9f,
            meter->mpItemB->getInitScaleY() * 1.9f);
        if (J2DPicture* swordPicture = first_picture_pane(meter->mpItemB->getPanePtr())) {
            swordPicture->setMirror(MIRROR0);
        }
        meter->mpItemB->getPanePtr()->rotate(
            meter->mpItemB->getSizeX() * 0.5f,
            meter->mpItemB->getSizeY() * 0.5f,
            ROTATE_Z, 360.0f);
    }
    if (meter->mpLightB != nullptr) {
        meter->mpLightB->paneTrans(50.0f, 19.0f);
    }

    for (int i = 0; i < 2; ++i) {
        restore_archive_pane(meter->mpButtonXY[i]);
        restore_archive_pane(meter->mpItemXY[i]);
        restore_archive_pane(meter->mpLightXY[i]);
        restore_archive_pane(meter->mpTextXY[i]);
    }

    // Wolf actions use labels instead of equipped-item artwork. The stock
    // GameCube text anchors place Sense and Dig to the left of their prompts;
    // Twilight Princess HD centers each label along the bottom of its prompt.
    if (daPy_py_c::checkNowWolf()) {
        if (meter->mpTextB != nullptr) {
            meter->mpTextB->paneTrans(40.0f, 0.0f);
        }
        if (meter->mpTextXY[0] != nullptr) {
            meter->mpTextXY[0]->paneTrans(50.0f, 7.0f);
            meter->mpTextXY[0]->scale(
                meter->mpTextXY[0]->getInitScaleX() * 0.75f,
                meter->mpTextXY[0]->getInitScaleY() * 0.75f);
        }
        if (meter->mpTextXY[1] != nullptr) {
            meter->mpTextXY[1]->paneTrans(62.0f, 3.0f);
            meter->mpTextXY[1]->scale(
                meter->mpTextXY[1]->getInitScaleX() * 1.30f,
                meter->mpTextXY[1]->getInitScaleY() * 1.30f);
        }
    }

    // The archive leaves the X item art left of its prompt. Recenter only the
    // item pane; the X button and authored diamond geometry stay fixed.
    if (meter->mpItemXY[0] != nullptr) {
        meter->mpItemXY[0]->paneTrans(42.0f, -15.0f);
        meter->mpItemXY[0]->scale(meter->mpItemXY[0]->getInitScaleX() * 1.25f,
            meter->mpItemXY[0]->getInitScaleY() * 1.25f);
    }
    if (meter->mpItemXY[1] != nullptr) {
        meter->mpItemXY[1]->paneTrans(42.0f, -7.0f);
        meter->mpItemXY[1]->scale(meter->mpItemXY[1]->getInitScaleX() * 1.25f,
            meter->mpItemXY[1]->getInitScaleY() * 1.25f);
    }

    apply_button_layout_preference(meter);
}

void apply_wii_u_item_num_layout(dMeter2Draw_c* meter) {
    if (meter == nullptr || s_wiiUItemNumTransform.active) {
        return;
    }

    s_wiiUItemNumTransform.meter = meter;
    for (int i = 0; i < 2; ++i) {
        s_wiiUItemNumTransform.posX[i] = meter->mItemParams[i].num_pos_x;
        s_wiiUItemNumTransform.posY[i] = meter->mItemParams[i].num_pos_y;
        s_wiiUItemNumTransform.scale[i] = meter->mItemParams[i].num_scale;
        // Both assignable item slots place their artwork and ammunition above
        // their corresponding face-button prompt.
        meter->mItemParams[i].num_pos_x += i == 0 ? 7.0f : -1.0f;
        // Wii U places the ammunition count along the lower edge of the item
        // artwork (for example, the bow count overlaps the bottom of the bow).
        meter->mItemParams[i].num_pos_y += i == 0 ? 1.0f : 6.0f;
        meter->mItemParams[i].num_scale *= 0.55f;
    }
    s_wiiUItemNumTransform.active = true;
}

void restore_wii_u_item_num_layout(dMeter2Draw_c* meter) {
    if (!s_wiiUItemNumTransform.active || s_wiiUItemNumTransform.meter != meter) {
        return;
    }

    for (int i = 0; i < 2; ++i) {
        meter->mItemParams[i].num_pos_x = s_wiiUItemNumTransform.posX[i];
        meter->mItemParams[i].num_pos_y = s_wiiUItemNumTransform.posY[i];
        meter->mItemParams[i].num_scale = s_wiiUItemNumTransform.scale[i];
    }
    s_wiiUItemNumTransform = {};
}

void apply_wii_u_r_button_art(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpScreen == nullptr) {
        return;
    }

    if (s_wiiURButtonMeter != meter) {
        s_wiiURButtonMeter = meter;
        s_wiiURButtonPicture = nullptr;
    }

    if (s_wiiURButtonPicture == nullptr) {
        auto* texture = static_cast<ResTIMG const*>(
            dComIfGp_getMain2DArchive()->getResource('TIMG', "wiiu_r.bti"));
        s_wiiURButtonPicture = as_picture(meter->mpScreen->search(MULTI_CHAR('zbtn')));
        if (texture != nullptr && s_wiiURButtonPicture != nullptr) {
            s_wiiURButtonPicture->changeTexture(texture, 0);
            if (s_wiiURButtonPicture->getTexture(0) != nullptr) {
                s_wiiURButtonPicture->setTexCoord(
                    s_wiiURButtonPicture->getTexture(0), BIND15, MIRROR0, false);
            }
            resize_pane_around_center(s_wiiURButtonPicture, 64.0f, 64.0f);
        }
    }

    const u8 itemNo = dComIfGp_getSelectItem(kZItemSlot);
    const bool rHudVisible = meter->mpButtonParent != nullptr &&
        meter->mpButtonParent->getPanePtr()->isVisible() &&
        meter->mpButtonParent->getAlphaRate() > 0.01f &&
        itemNo != dItemNo_NONE_e && itemNo != 0 &&
        !daPy_py_c::checkNowWolf() && !z_item_menu_or_pause_context();

    if (rHudVisible) {
        meter->mpButtonXY[2]->show();
        if (s_wiiURButtonPicture != nullptr) {
            s_wiiURButtonPicture->show();
            s_wiiURButtonPicture->setAlpha(255);
        }
    } else {
        meter->mpButtonXY[2]->hide();
        meter->mpItemR->hide();
        meter->mpLightXY[2]->hide();
        if (s_wiiURButtonPicture != nullptr) {
            s_wiiURButtonPicture->hide();
        }
    }

    // The Wii U badge includes its own R glyph, so suppress the GameCube Z
    // glyph that otherwise draws separately at the edge of the screen.
    if (J2DPane* zGlyph = meter->mpScreen->search(MULTI_CHAR('z_btn_t'))) {
        zGlyph->hide();
    }
}

void apply_wii_u_archive_layout_corrections(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    const f32 scale = hud_scale();

    if (meter->mpButtonParent != nullptr) {
        meter->mpButtonParent->scale(meter->mpButtonParent->getInitScaleX() * scale,
            meter->mpButtonParent->getInitScaleY() * scale);
    }

    apply_hud_pane_transform(HudPaneSlot::ButtonZ, meter->mpButtonXY[2], true, 31.0f,
        -4.0f, 0.45f);
    apply_hud_pane_transform(HudPaneSlot::TextZ, meter->mpTextXY[2], true, 31.0f,
        -4.0f, 0.45f);

    apply_hud_pane_transform(HudPaneSlot::DPad, meter->mpButtonCrossParent, true,
        -24.0f, -307.0f, 1.30f * scale);
    apply_hud_pane_transform(HudPaneSlot::Hearts, meter->mpLifeParent, true, -22.0f,
        -19.0f, scale);
    // Keep the icon, frame, and digit layers on their common animated parent.
    // Moving the three child layers independently makes their stock animation
    // fight our transform and leaves the rupee icon behind.
    apply_hud_pane_transform(HudPaneSlot::Rupee0, meter->mpRupeeParent[0], false,
        0.0f, 0.0f, 1.0f);
    apply_hud_pane_transform(HudPaneSlot::Rupee1, meter->mpRupeeParent[1], false,
        0.0f, 0.0f, 1.0f);
    apply_hud_pane_transform(HudPaneSlot::Rupee2, meter->mpRupeeParent[2], false,
        0.0f, 0.0f, 1.0f);
    apply_hud_pane_transform(HudPaneSlot::RupeeKey, meter->mpRupeeKeyParent, true,
        20.0f, 8.0f, scale);

    for (CPaneMgr* pane : {meter->mpMagicParent, meter->mpLightDropParent,
             meter->mpSIParent[0], meter->mpSIParent[1], meter->mpKeyParent,
             meter->mpPikariParent})
    {
        if (pane != nullptr) {
            pane->scale(pane->getInitScaleX() * scale, pane->getInitScaleY() * scale);
        }
    }
    if (meter->mpRupeeKeyParent != nullptr) {
        meter->mpRupeeKeyParent->show();
    }
}

void stabilize_wii_u_rupee_counter(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpScreen == nullptr ||
        meter->mpRupeeKeyParent == nullptr || meter->mpRupeeParent[0] == nullptr)
    {
        return;
    }

    // The archive gives the hundreds pane different geometry and material
    // animation from the tens and ones panes. Hide all authored digit panes;
    // a uniform strip using the same native textures is drawn after the HUD.
    for (int digit = 0; digit < 4; ++digit) {
        if (meter->mpRupeeTexture[digit][0] != nullptr) {
            meter->mpRupeeTexture[digit][0]->hide();
            meter->mpRupeeTexture[digit][0]->setAlpha(0);
            meter->mpRupeeTexture[digit][0]->setAlphaRate(0.0f);
        }
        if (meter->mpRupeeTexture[digit][1] != nullptr) {
            meter->mpRupeeTexture[digit][1]->scale(0.65f, 0.65f);
            meter->mpRupeeTexture[digit][1]->paneTrans(
                g_drawHIO.mRupeeCountPosX,
                g_drawHIO.mRupeeCountPosY + 9.0f);
            meter->mpRupeeTexture[digit][1]->hide();
            meter->mpRupeeTexture[digit][1]->setAlpha(0);
            meter->mpRupeeTexture[digit][1]->setAlphaRate(0.0f);
        }
    }

    meter->mpRupeeKeyParent->show();
    meter->mpRupeeKeyParent->setAlphaRate(1.0f);
    meter->mpRupeeParent[0]->show();
    meter->mpRupeeParent[0]->setAlphaRate(1.0f);
    // This archive adds a decorative ring pane behind the rupee group. Unlike
    // the matching key pane, the stock initializer never hides it, producing
    // the dark oval to the right of the currency value.
    if (J2DPane* rupeeRing = meter->mpScreen->search(MULTI_CHAR('ju_ring6'))) {
        rupeeRing->hide();
        rupeeRing->setAlpha(0);
    }
    // The two decorative frame parents overlap the number in this archive and
    // can be animated back into view. Disable both decisively.
    for (int frame = 1; frame < 3; ++frame) {
        if (meter->mpRupeeParent[frame] != nullptr) {
            meter->mpRupeeParent[frame]->hide();
            meter->mpRupeeParent[frame]->setAlphaRate(0.0f);
        }
    }

    // The archive already assigns the correct HUD rupee artwork to `rupi`.
    // Do not replace it with the similarly named inventory-item texture.
    // Keep just the crisp foreground pane, matching the digit treatment above.
    J2DPicture* shadowRupeeIcon =
        as_picture(meter->mpScreen->search(MULTI_CHAR('rupi_s')));
    J2DPicture* visibleRupeeIcon =
        as_picture(meter->mpScreen->search(MULTI_CHAR('rupi')));
    if (shadowRupeeIcon != nullptr) {
        shadowRupeeIcon->hide();
    }

    if (visibleRupeeIcon != nullptr && meter->mpRupeeTexture[3][1] != nullptr) {
        J2DPane* counterParent = meter->mpRupeeKeyParent->getPanePtr();
        J2DPane* thousandsDigit = meter->mpRupeeTexture[3][1]->getPanePtr();
        if (visibleRupeeIcon->getParentPane() != counterParent) {
            counterParent->appendChild(visibleRupeeIcon);
        }
        // Preserve the texture's native aspect ratio. Forcing this pane into a
        // square visibly deforms the faceted rupee artwork.
        f32 iconWidth = 15.0f;
        constexpr f32 iconHeight = 18.0f;
        if (visibleRupeeIcon->getTexture(0) != nullptr &&
            visibleRupeeIcon->getTexture(0)->getHeight() > 0)
        {
            iconWidth = iconHeight *
                (static_cast<f32>(visibleRupeeIcon->getTexture(0)->getWidth()) /
                 static_cast<f32>(visibleRupeeIcon->getTexture(0)->getHeight()));
        }
        resize_pane_around_center(visibleRupeeIcon, iconWidth, iconHeight);
        // After reparenting, both values are in the counter parent's space.
        // The authored digit origin is shared by all four digits; these offsets
        // place the icon immediately to the left of the visible number.
        visibleRupeeIcon->move(thousandsDigit->getTranslateX() + 87.0f,
            thousandsDigit->getTranslateY() + 46.0f);
        visibleRupeeIcon->show();
        visibleRupeeIcon->setAlpha(255);
    }
}

bool ensure_rupee_digit_textures() {
    ResTIMG* zero = static_cast<ResTIMG*>(dComIfGp_getMain2DArchive()->getResource(
        'TIMG', dMeter2Info_getNumberTextureName(0)));
    if (zero == nullptr) {
        return false;
    }

    for (J2DPicture*& digit : s_rupeeDigitTex) {
        if (digit == nullptr) {
            digit = JKR_NEW J2DPicture(zero);
        }
        if (digit == nullptr) {
            return false;
        }
    }
    return true;
}

void draw_uniform_rupee_digits(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpScreen == nullptr ||
        !ensure_rupee_digit_textures())
    {
        return;
    }

    J2DPicture* icon = as_picture(meter->mpScreen->search(MULTI_CHAR('rupi')));
    if (icon == nullptr) {
        return;
    }

    const int value = static_cast<int>(dComIfGs_getRupee());
    const int digitCount = value >= 1000 ? 4 : 3;
    int divisor = digitCount == 4 ? 1000 : 100;

    JUtility::TColor black;
    black.set(0, 0, 0, 0);
    JUtility::TColor white;
    white.set(255, 255, 255, 255);
    JUtility::TColor cornerWhite;
    cornerWhite.set(255, 255, 255, 255);

    constexpr f32 digitSize = 11.0f;
    constexpr f32 digitStep = 13.0f;
    const Vec iconTopLeft = icon->getGlbVtx(0);
    const Vec iconBottomRight = icon->getGlbVtx(3);
    const f32 startX = iconBottomRight.x + 5.0f;
    const f32 startY = (iconTopLeft.y + iconBottomRight.y - digitSize) * 0.5f;

    int remainder = value;
    for (int index = 0; index < digitCount; ++index) {
        const int number = (remainder / divisor) % 10;
        ResTIMG* texture = static_cast<ResTIMG*>(dComIfGp_getMain2DArchive()->getResource(
            'TIMG', dMeter2Info_getNumberTextureName(number)));
        if (texture != nullptr) {
            J2DPicture* digit = s_rupeeDigitTex[index];
            digit->changeTexture(texture, 0);
            digit->setBlackWhite(black, white);
            digit->setCornerColor(cornerWhite);
            digit->setAlpha(255);
            digit->draw(startX + digitStep * index, startY, digitSize, digitSize,
                false, false, false);
        }
        remainder %= divisor;
        divisor = std::max(1, divisor / 10);
    }
}

void draw_wolf_icon(CPaneMgr* button, J2DPicture* icon, const WolfIconLayout& layout) {
    if (!daPy_py_c::checkNowWolf() || button == nullptr || icon == nullptr) {
        return;
    }

    J2DPane* pane = button->getPanePtr();
    const f32 alphaRate = button->getAlphaRate();
    if (pane == nullptr || !pane->isVisible() || alphaRate <= 0.01f) {
        return;
    }

    const Vec topLeft = pane->getGlbVtx(0);
    const Vec bottomRight = pane->getGlbVtx(3);
    const f32 centerX = (topLeft.x + bottomRight.x) * 0.5f;
    const f32 centerY = (topLeft.y + bottomRight.y) * 0.5f;
    const f32 scale = hud_scale();

    icon->setAlpha(static_cast<u8>(layout.opacity * alphaRate));
    icon->draw(centerX + layout.offsetX * scale, centerY + layout.offsetY * scale,
        layout.size * scale, layout.size * scale, false, false, false);
}

void draw_wolf_action_icons(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    draw_wolf_icon(meter->mpButtonXY[0], s_senseIconPicture, kSenseIconLayout);
    draw_wolf_icon(meter->mpButtonXY[1], s_digIconPicture, kDigIconLayout);
    draw_wolf_icon(meter->mpButtonB, s_attackIconPicture, kAttackIconLayout);
}


void apply_wii_u_dpad_style(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpScreen == nullptr) {
        return;
    }

    // These four panes are the GameCube orange direction markers. The Wii U
    // presentation uses the clean white cross without those surrounding arrows.
    constexpr u64 arrowTags[] = {
        MULTI_CHAR('yaji_00'), MULTI_CHAR('yaji_01'),
        MULTI_CHAR('yaji_02'), MULTI_CHAR('yaji_03'),
    };
    for (const u64 tag : arrowTags) {
        if (J2DPane* arrow = meter->mpScreen->search(tag)) {
            arrow->hide();
        }
    }

    // These are the separate GameCube backing/glow panes. They become an
    // oversized offset shadow when the clean white cross is enlarged.
    constexpr u64 dpadBackingTags[] = {
        MULTI_CHAR('ju_ring1'), MULTI_CHAR('ju_ring2'),
        MULTI_CHAR('ju_ring3'), MULTI_CHAR('ju_ring4'),
    };
    for (const u64 tag : dpadBackingTags) {
        if (J2DPane* backing = meter->mpScreen->search(tag)) {
            backing->hide();
        }
    }

    // Keep the font these layered text panes were authored for so their outline
    // copies stay registered. A compact group scale gives the lighter Wii U
    // presentation without creating separated black shadows.
    if (meter->mpTextI != nullptr) {
        meter->mpTextI->scale(0.42f, 0.42f);
        // Swap Items with Midna: Items now occupies the lower D-Pad row.
        meter->mpTextI->paneTrans(0.0f, 46.0f);
    }
    if (meter->mpTextM != nullptr) {
        meter->mpTextM->scale(0.42f, 0.42f);
        meter->mpTextM->paneTrans(-11.0f, 0.0f);
    }
    constexpr u64 textTags[] = {
        MULTI_CHAR('cont_ju0'), MULTI_CHAR('cont_ju1'), MULTI_CHAR('cont_ju2'),
        MULTI_CHAR('cont_ju3'), MULTI_CHAR('cont_ju4'), MULTI_CHAR('cont_ju5'),
        MULTI_CHAR('cont_ju6'), MULTI_CHAR('cont_ju7'), MULTI_CHAR('cont_ju8'),
        MULTI_CHAR('cont_ju9'),
    };
    constexpr u64 actionTextTags[] = {
        MULTI_CHAR('cont_at1'), MULTI_CHAR('cont_at2'), MULTI_CHAR('cont_at3'),
        MULTI_CHAR('cont_at4'), MULTI_CHAR('cont_at'),
    };
    for (int layer = 0; layer < 5; ++layer) {
        auto* actionText = static_cast<J2DTextBox*>(
            meter->mpScreen->search(actionTextTags[layer]));
        if (actionText == nullptr) {
            continue;
        }

        J2DTextBox::TFontSize actionFontSize;
        actionText->getFontSize(actionFontSize);
        for (const int destination : {layer, layer + 5}) {
            auto* text = static_cast<J2DTextBox*>(
                meter->mpScreen->search(textTags[destination]));
            if (text == nullptr) {
                continue;
            }
            text->setFont(actionText->getFont());
            text->setFontSize(actionFontSize);
            text->setCharSpace(actionText->getCharSpace());
            text->setLineSpace(actionText->getLineSpace());
            const char* label = destination < 5 ? "Items" : "Map";
            if (std::strcmp(text->getStringPtr(), label) != 0) {
                text->setString(0x40, label);
            }
        }
    }
}

void apply_hud_backing_visibility(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpUzu == nullptr) {
        return;
    }

    meter->mpUzu->hide();
}

void apply_wii_u_minimap_layout(dMeterMap_c* map) {
    if (map == nullptr) {
        return;
    }

    s_wiiUMinimapTransform = {
        .map = map,
        .drawPosX = map->mDrawPosX,
        .drawPosY = map->mDrawPosY,
        .sizeW = map->mSizeW,
        .sizeH = map->mSizeH,
        .active = true,
    };

    const f32 originalHeight = map->mSizeH;
    const f32 scale = 0.70f * hud_scale();
    map->mSizeW *= scale;
    map->mSizeH *= scale;
    // Preserve the original bottom-left anchor while shrinking the minimap.
    map->mDrawPosX -= 22.0f;
    map->mDrawPosY += originalHeight - map->mSizeH + 12.0f;
}

void restore_wii_u_minimap_layout(dMeterMap_c* map) {
    if (!s_wiiUMinimapTransform.active || s_wiiUMinimapTransform.map != map) {
        s_wiiUMinimapTransform = {};
        return;
    }

    map->mDrawPosX = s_wiiUMinimapTransform.drawPosX;
    map->mDrawPosY = s_wiiUMinimapTransform.drawPosY;
    map->mSizeW = s_wiiUMinimapTransform.sizeW;
    map->mSizeH = s_wiiUMinimapTransform.sizeH;
    s_wiiUMinimapTransform = {};
}

J2DPane* item_wheel_z_anchor(J2DScreen* screen) {
    return screen != nullptr ? screen->search(MULTI_CHAR('r_btn_n')) : nullptr;
}

void apply_item_wheel_z_offset(Vec& pos) {
    pos.x += 5.0f;
    pos.y -= 5.0f;
}

void destroy_ring_z_prompt(dMenu_Ring_c* ring) {
    if (s_ringZPrompt.ring != ring) {
        return;
    }

    JKR_DELETE(s_ringZPrompt.button);
    s_ringZPrompt.button = nullptr;
    JKR_DELETE(s_ringZPrompt.screen);
    s_ringZPrompt.screen = nullptr;
    s_ringZPrompt.ring = nullptr;
}

void create_ring_z_prompt(dMenu_Ring_c* ring) {
    destroy_ring_z_prompt(s_ringZPrompt.ring);
    if (ring == nullptr || ring->mPlayerIsWolf || ring->mpScreen == nullptr) {
        return;
    }

    J2DPane* anchor = item_wheel_z_anchor(ring->mpScreen);
    if (anchor != nullptr) {
        anchor->translate(anchor->getTranslateX() + 64.0f, anchor->getTranslateY());
        anchor->hide();
    }

    J2DScreen* screen = JKR_NEW J2DScreen();
    if (screen == nullptr) {
        return;
    }
    if (!screen->setPriority("zelda_game_image.blo", 0x20000, dComIfGp_getMain2DArchive())) {
        JKR_DELETE(screen);
        return;
    }

    dPaneClass_showNullPane(screen);
    hide_pane_tree(screen->search('ROOT'));

    J2DPane* zButtonPane = screen->search(MULTI_CHAR('zbtn_n'));
    if (zButtonPane == nullptr) {
        JKR_DELETE(screen);
        return;
    }

    show_pane_parents(zButtonPane);
    show_pane_tree(zButtonPane);

    CPaneMgr* button = JKR_NEW CPaneMgr(screen, MULTI_CHAR('zbtn_n'), 2, nullptr);
    if (button == nullptr) {
        JKR_DELETE(screen);
        return;
    }

    button->setAlphaRate(1.0f);
    button->show();
    s_ringZPrompt = {.ring = ring, .screen = screen, .button = button};
}

void draw_ring_z_prompt(dMenu_Ring_c* ring) {
    if (s_ringZPrompt.ring != ring ||
        s_ringZPrompt.screen == nullptr || s_ringZPrompt.button == nullptr ||
        ring == nullptr || ring->mpScreen == nullptr || ring->mPlayerIsWolf)
    {
        return;
    }

    J2DPane* anchor = item_wheel_z_anchor(ring->mpScreen);
    if (anchor == nullptr) {
        return;
    }

    CPaneMgr paneMgr;
    Vec pos = paneMgr.getGlobalVtxCenter(anchor, true, 0);
    pos.x += ring->mCenterPosX;
    pos.y += ring->mCenterPosY;
    apply_item_wheel_z_offset(pos);

    s_ringZPrompt.button->scale(0.9f, 0.9f);
    s_ringZPrompt.button->paneTrans(pos.x - s_ringZPrompt.button->getInitGlobalCenterPosX(),
                                    pos.y - s_ringZPrompt.button->getInitGlobalCenterPosY());
    s_ringZPrompt.button->setAlphaRate(ring->mAlphaRate);
    s_ringZPrompt.screen->draw(0.0f, 0.0f, dComIfGp_getCurrentGrafPort());
}

bool pane_current_global_bounds(CPaneMgr* pane, f32& left, f32& top, f32& right, f32& bottom) {
    if (pane == nullptr) {
        return false;
    }

    Mtx mtx;
    for (u8 i = 0; i < 4; ++i) {
        Vec vtx = pane->getGlobalVtx(&mtx, i, false, 0);
        if (i == 0) {
            left = right = vtx.x;
            top = bottom = vtx.y;
            continue;
        }
        if (vtx.x < left) {
            left = vtx.x;
        }
        if (vtx.x > right) {
            right = vtx.x;
        }
        if (vtx.y < top) {
            top = vtx.y;
        }
        if (vtx.y > bottom) {
            bottom = vtx.y;
        }
    }
    return true;
}

bool add_pane_current_global_bounds(CPaneMgr* pane, f32& left, f32& top, f32& right,
    f32& bottom, bool& hasBounds) {
    f32 paneLeft;
    f32 paneTop;
    f32 paneRight;
    f32 paneBottom;
    if (!pane_current_global_bounds(pane, paneLeft, paneTop, paneRight, paneBottom)) {
        return false;
    }

    if (!hasBounds) {
        left = paneLeft;
        top = paneTop;
        right = paneRight;
        bottom = paneBottom;
        hasBounds = true;
        return true;
    }

    left = std::min(left, paneLeft);
    right = std::max(right, paneRight);
    top = std::min(top, paneTop);
    bottom = std::max(bottom, paneBottom);
    return true;
}

void pane_trans_to_global_center(CPaneMgr* pane, const f32 targetX, const f32 targetY) {
    f32 transX = targetX - pane->getInitGlobalCenterPosX();
    f32 transY = targetY - pane->getInitGlobalCenterPosY();
    pane->paneTrans(transX, transY);

    f32 left;
    f32 top;
    f32 right;
    f32 bottom;
    if (!pane_current_global_bounds(pane, left, top, right, bottom)) {
        return;
    }

    const f32 centerX = (left + right) * 0.5f;
    const f32 centerY = (top + bottom) * 0.5f;
    const f32 localWidth = pane->getSizeX();
    const f32 localHeight = pane->getSizeY();
    const f32 globalScaleX = localWidth != 0.0f ? (right - left) / localWidth : 1.0f;
    const f32 globalScaleY = localHeight != 0.0f ? (bottom - top) / localHeight : 1.0f;

    if (globalScaleX != 0.0f) {
        transX += (targetX - centerX) / globalScaleX;
    }
    if (globalScaleY != 0.0f) {
        transY += (targetY - centerY) / globalScaleY;
    }
    pane->paneTrans(transX, transY);
}

void change_z_hud_item_texture(dMeter2Draw_c* meter, const u8 itemNo) {
    if (s_zHudItemMeter != meter) {
        s_zHudItemMeter = meter;
        s_zHudLastItem = dItemNo_NONE_e;
    }

    const u8 textureItem = hud_texture_item(itemNo);
    if (s_zHudLastItem == textureItem) {
        return;
    }

    s_zHudItemTexPage ^= 1;
    ResTIMG* primary = z_hud_item_tex(s_zHudItemTexPage, 0);
    ResTIMG* secondary = z_hud_item_tex(s_zHudItemTexPage, 1);
    const s32 textureCount =
        dMeter2Info_readItemTexture(textureItem, primary,
            static_cast<J2DPicture*>(meter->mpItemR->getPanePtr()), secondary,
            meter->mpItemXYPane[2], nullptr, nullptr, nullptr, nullptr, -1);
    if (textureCount <= 1) {
        meter->mpItemXYPane[2]->hide();
    } else {
        meter->mpItemXYPane[2]->show();
    }

    const f32 textureScale = g_drawHIO.mItemScaleAdjustON ?
        g_drawHIO.mItemScalePercent / 100.0f :
        dItem_data::getTexScale(textureItem) / 100.0f;
    meter->field_0x6c4[2] =
        textureScale * ((primary->width * meter->mpItemR->getInitSizeX()) / 48.0f);
    meter->field_0x6d0[2] =
        textureScale * ((primary->height * meter->mpItemR->getInitSizeY()) / 48.0f);
    meter->field_0x6ac[2] = (meter->mpItemR->getInitSizeX() - meter->field_0x6c4[2]) * 0.5f;
    meter->field_0x6b8[2] = (meter->mpItemR->getInitSizeY() - meter->field_0x6d0[2]) * 0.5f;
    meter->mpItemR->resize(meter->field_0x6c4[2], meter->field_0x6d0[2]);
    meter->mpItemXYPane[2]->resize(meter->field_0x6c4[2], meter->field_0x6d0[2]);
    s_zHudLastItem = textureItem;
}

void layout_z_hud_item(dMeter2Draw_c* meter, const u8 itemNo) {
    meter->setItemParamZ(hud_layout_item(itemNo));
    meter->mpItemR->getPanePtr()->rotate(meter->mpItemR->getSizeX() * 0.5f,
        meter->mpItemR->getSizeY() * 0.5f, ROTATE_Z,
        meter->mItemParams[dMeter2Draw_c::SELECT_Z_e].rotation);

    const HudTransform& hudTransform = kRItemTransform;
    const HudItemLayout& buttonLayout = kRItemLayout;
    const f32 hudScale = hudTransform.scale;
    const f32 itemScale = buttonLayout.itemScale;
    const f32 itemOffsetX = buttonLayout.itemOffsetX;
    const f32 itemOffsetY = buttonLayout.itemOffsetY;

    constexpr f32 iconScale = 1.0f;
    meter->mpItemR->scale(g_drawHIO.mButtonZItemScale * hudScale * itemScale * iconScale,
        g_drawHIO.mButtonZItemScale * hudScale * itemScale * iconScale);
    meter->mpItemR->paneTrans(g_drawHIO.mButtonZItemPosX + meter->field_0x6ac[2] +
            itemOffsetX + hudTransform.offsetX,
        g_drawHIO.mButtonZItemPosY + meter->field_0x6b8[2] + itemOffsetY +
            hudTransform.offsetY);

    meter->mpLightXY[2]->scale(
        g_drawHIO.mButtonZItemBaseScale * hudScale * itemScale * iconScale,
        g_drawHIO.mButtonZItemBaseScale * hudScale * itemScale * iconScale);
    meter->mpLightXY[2]->paneTrans(g_drawHIO.mButtonZItemBasePosX + itemOffsetX +
            hudTransform.offsetX,
        g_drawHIO.mButtonZItemBasePosY + itemOffsetY + hudTransform.offsetY);
}

bool is_z_lantern_item(const u8 itemNo) {
    return itemNo == dItemNo_KANTERA_e || itemNo == dItemNo_KANTERA2_e;
}

bool z_item_has_ammo(const u8 itemNo) {
    switch (itemNo) {
    case dItemNo_NORMAL_BOMB_e:
    case dItemNo_WATER_BOMB_e:
    case dItemNo_POKE_BOMB_e:
    case dItemNo_BOMB_ARROW_e:
    case dItemNo_BOW_e:
    case dItemNo_LIGHT_ARROW_e:
    case dItemNo_ARROW_LV1_e:
    case dItemNo_ARROW_LV2_e:
    case dItemNo_ARROW_LV3_e:
    case dItemNo_HAWK_ARROW_e:
    case dItemNo_PACHINKO_e:
    case dItemNo_BEE_CHILD_e:
        return true;
    default:
        return false;
    }
}

bool z_item_ammo_values(const u8 itemNo, u8& itemNum, u8& itemMax) {
    if (!z_item_has_ammo(itemNo)) {
        return false;
    }

    switch (itemNo) {
    case dItemNo_BOW_e:
    case dItemNo_LIGHT_ARROW_e:
    case dItemNo_ARROW_LV1_e:
    case dItemNo_ARROW_LV2_e:
    case dItemNo_ARROW_LV3_e:
    case dItemNo_HAWK_ARROW_e:
        itemNum = static_cast<u8>(dComIfGs_getArrowNum());
        itemMax = static_cast<u8>(dComIfGs_getArrowMax());
        return true;
    case dItemNo_BOMB_ARROW_e: {
        itemNum = static_cast<u8>(std::max<s16>(0, dComIfGp_getSelectItemNum(kZItemSlot)));
        itemMax = static_cast<u8>(std::max(0, dComIfGp_getSelectItemMaxNum(kZItemSlot)));
        itemNum = std::min(itemNum, static_cast<u8>(dComIfGs_getArrowNum()));
        itemMax = std::max(itemMax, static_cast<u8>(dComIfGs_getArrowMax()));
        return true;
    }
    case dItemNo_PACHINKO_e:
        itemNum = static_cast<u8>(dComIfGs_getPachinkoNum());
        itemMax = static_cast<u8>(dComIfGs_getPachinkoMax());
        return true;
    default:
        itemNum = static_cast<u8>(std::max<s16>(0, dComIfGp_getSelectItemNum(kZItemSlot)));
        itemMax = static_cast<u8>(std::max(0, dComIfGp_getSelectItemMaxNum(kZItemSlot)));
        return true;
    }
}

bool ensure_z_item_num_textures() {
    if (s_zItemNumTex[0] != nullptr && s_zItemNumTex[1] != nullptr &&
        s_zItemNumTex[2] != nullptr)
    {
        return true;
    }

    ResTIMG* timg = static_cast<ResTIMG*>(dComIfGp_getMain2DArchive()->getResource(
        'TIMG', dMeter2Info_getNumberTextureName(0)));
    if (timg == nullptr) {
        return false;
    }

    for (int i = 0; i < 3; ++i) {
        if (s_zItemNumTex[i] == nullptr) {
            s_zItemNumTex[i] = JKR_NEW J2DPicture(timg);
        }
        if (s_zItemNumTex[i] == nullptr) {
            return false;
        }
    }
    return true;
}

void set_z_item_num_textures(u8 itemNum, const u8 itemMax) {
    if (!ensure_z_item_num_textures()) {
        return;
    }

    if (itemNum > itemMax) {
        itemNum = itemMax;
    }

    JUtility::TColor black;
    JUtility::TColor white;
    if (itemNum == itemMax) {
        black.set(30, 30, 30, 0);
        white.set(255, 200, 50, 255);
    } else if (itemNum == 0) {
        black.set(30, 30, 30, 0);
        white.set(180, 180, 180, 255);
    } else {
        black.set(0, 0, 0, 0);
        white.set(255, 255, 255, 255);
    }

    for (J2DPicture* digit : s_zItemNumTex) {
        digit->setBlackWhite(black, white);
    }

    auto set_digit = [](const int index, const int digit) {
        ResTIMG* timg = static_cast<ResTIMG*>(dComIfGp_getMain2DArchive()->getResource(
            'TIMG', dMeter2Info_getNumberTextureName(digit)));
        if (timg != nullptr) {
            s_zItemNumTex[index]->changeTexture(timg, 0);
        }
    };

    if (itemNum < 100) {
        set_digit(0, itemNum / 10);
        set_digit(1, itemNum % 10);
        s_zItemNumTex[2]->hide();
    } else {
        set_digit(0, itemNum / 100);
        itemNum %= 100;
        set_digit(1, itemNum / 10);
        set_digit(2, itemNum % 10);
        s_zItemNumTex[2]->show();
    }
}

void update_z_hud_item_alpha(dMeter2Draw_c* meter) {
    const f32 buttonAlpha =
        g_drawHIO.mButtonZAlpha * (g_drawHIO.mParentAlpha * g_drawHIO.mMainHUDButtonsAlpha);
    const f32 parentAlpha = meter->mpButtonParent->getAlphaRate();
    u8 itemAlpha = meter->mpItemR->getInitAlpha();
    u8 itemBaseAlpha = clamp_hud_alpha(
        g_drawHIO.mButtonZItemBaseAlpha * (buttonAlpha * meter->mpLightXY[2]->getInitAlpha()));
    u8 buttonBaseAlpha = clamp_hud_alpha(255.0f * buttonAlpha);

    if (dComIfGp_getSelectItem(kZItemSlot) == dItemNo_NONE_e ||
        dComIfGp_getSelectItem(kZItemSlot) == 0)
    {
        itemAlpha = g_drawHIO.mButtonXYItemDimAlpha;
        itemBaseAlpha = g_drawHIO.mButtonXYItemDimAlpha;
        buttonBaseAlpha = g_drawHIO.mButtonXYBaseDimAlpha;
    }

    meter->mpItemR->setAlpha(clamp_hud_alpha(static_cast<f32>(itemAlpha) * parentAlpha));
    meter->mpLightXY[2]->setAlpha(clamp_hud_alpha(static_cast<f32>(itemBaseAlpha) * parentAlpha));
    meter->mpButtonXY[2]->setAlpha(clamp_hud_alpha(static_cast<f32>(buttonBaseAlpha) * parentAlpha));
}

void draw_z_ammo(dMeter2Draw_c* meter, const u8 itemNo, const f32 itemAlphaRate) {
    u8 itemNum = 0;
    u8 itemMax = 0;
    if (!z_item_ammo_values(itemNo, itemNum, itemMax) || itemMax == 0 ||
        !ensure_z_item_num_textures())
    {
        return;
    }

    set_z_item_num_textures(itemNum, itemMax);

    const HudTransform& hudTransform = kRItemTransform;
    const HudItemLayout& buttonLayout = kRItemLayout;
    const f32 itemScale = buttonLayout.itemScale;
    const f32 ammoScale = hudTransform.scale * itemScale * buttonLayout.ammoScale;
    const f32 digitSize = meter->mItemParams[dMeter2Draw_c::SELECT_Z_e].num_scale * 16.0f * ammoScale;

    Vec vtx0 = meter->mpItemR->getPanePtr()->getGlbVtx(0);
    Vec vtx3 = meter->mpItemR->getPanePtr()->getGlbVtx(3);
    const f32 centerX = (vtx0.x + vtx3.x) * 0.5f;
    const f32 centerY = (vtx0.y + vtx3.y) * 0.5f;
    const u8 alpha = clamp_hud_alpha(itemAlphaRate * 255.0f);

    for (int i = 0; i < 3; ++i) {
        if (i == 2 && itemNum < 100) {
            continue;
        }
        s_zItemNumTex[i]->setAlpha(alpha);
        s_zItemNumTex[i]->draw(meter->mItemParams[dMeter2Draw_c::SELECT_Z_e].num_pos_x +
                buttonLayout.ammoOffsetX + centerX + digitSize * i,
            meter->mItemParams[dMeter2Draw_c::SELECT_Z_e].num_pos_y +
                buttonLayout.ammoOffsetY + centerY + meter->mpItemR->getSizeY(),
            digitSize, digitSize, false, false, false);
    }
}

void draw_z_oil_meter(dMeter2Draw_c* meter, const u8 itemNo, const f32 itemAlphaRate) {
    if (!is_z_lantern_item(itemNo) || dComIfGs_getMaxOil() == 0) {
        return;
    }

    if (s_zOilMeter == nullptr) {
        s_zOilMeter = JKR_NEW dKantera_icon_c();
    }
    if (s_zOilMeter == nullptr) {
        return;
    }

    const HudTransform& hudTransform = kRItemTransform;
    const HudItemLayout& buttonLayout = kRItemLayout;
    const f32 itemScale = buttonLayout.itemScale;
    Vec vtx0 = meter->mpItemR->getPanePtr()->getGlbVtx(0);
    Vec vtx3 = meter->mpItemR->getPanePtr()->getGlbVtx(3);

    s_zOilMeter->setPos(((vtx0.x + vtx3.x) * 0.5f) + 9.0f * hudTransform.scale * itemScale,
        vtx3.y);
    s_zOilMeter->setScale(0.6f * hudTransform.scale * itemScale,
        0.6f * hudTransform.scale * itemScale);
    s_zOilMeter->setNowGauge(dComIfGs_getMaxOil(), dComIfGs_getOil());
    s_zOilMeter->setAlphaRate(itemAlphaRate);
    s_zOilMeter->drawSelf();
}

void draw_z_hud_item_meters(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpItemR == nullptr ||
        meter->mpButtonParent == nullptr || daPy_py_c::checkNowWolf())
    {
        return;
    }
    if (!meter->mpButtonParent->getPanePtr()->isVisible()) {
        return;
    }

    const u8 itemNo = dComIfGp_getSelectItem(kZItemSlot);
    if (itemNo == dItemNo_NONE_e || itemNo == 0 || !meter->mpItemR->isVisible()) {
        return;
    }

    const f32 itemAlphaRate = static_cast<f32>(meter->mpItemR->getAlpha()) / 255.0f;
    if (itemAlphaRate <= 0.0f) {
        return;
    }

    draw_z_ammo(meter, itemNo, itemAlphaRate);
    draw_z_oil_meter(meter, itemNo, itemAlphaRate);
}

void update_z_hud_item(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpItemR == nullptr ||
        meter->mpLightXY[2] == nullptr || meter->mpButtonXY[2] == nullptr ||
        meter->mpItemXYPane[2] == nullptr)
    {
        return;
    }

    if (daPy_py_c::checkNowWolf()) {
        // The stock HUD reuses these panes while Link transforms. Reload both
        // item layers when the human HUD returns, even if the selected item did
        // not change.
        s_zHudLastItem = dItemNo_NONE_e;
        return;
    }

    if (meter->mpTextXY[2] != nullptr) {
        meter->mpTextXY[2]->hide();
    }

    const u8 itemNo = dComIfGp_getSelectItem(kZItemSlot);
    J2DPane* itemParent = meter->mpScreen != nullptr ?
        meter->mpScreen->search(MULTI_CHAR('item_r_n')) : nullptr;
    if (itemNo == dItemNo_NONE_e || itemNo == 0) {
        if (itemParent != nullptr) {
            itemParent->hide();
        }
        meter->mpItemR->hide();
        meter->mpLightXY[2]->hide();
        return;
    }

    if (itemParent != nullptr) {
        itemParent->show();
    }
    meter->mpItemR->show();
    meter->mpLightXY[2]->show();
    dMeter2Info_onUseButton(METER2_USEBUTTON_Z);
    change_z_hud_item_texture(meter, itemNo);
    layout_z_hud_item(meter, itemNo);
    update_z_hud_item_alpha(meter);
}

void position_midna_hud(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpScreen == nullptr) {
        return;
    }

    J2DPane* midnaPane = meter->mpScreen->search(MULTI_CHAR('midona_n'));
    if (midnaPane == nullptr) {
        return;
    }

    if (z_item_menu_or_pause_context() || !midna_unlocked()) {
        set_pane_tree_alpha_visible(midnaPane, false, 0);
        return;
    }

    // The standalone L badge does not participate reliably in the stock HUD's
    // visibility path. Use the established portable layout instead: Midna is
    // attached to and displayed over D-Pad Up.
    J2DPane* anchorPane = meter->mpScreen->search(MULTI_CHAR('juji_n'));
    constexpr f32 positionX = 8.0f;
    // Midna now occupies the former Items row above the D-Pad while D-Pad
    // Down remains her input binding.
    constexpr f32 positionY = -21.0f;

    if (anchorPane == nullptr) {
        return;
    }

    if (midnaPane->getParentPane() != anchorPane) {
        anchorPane->appendChild(midnaPane);
        set_pane_influenced_alpha_tree(midnaPane, true);
    }

    // Counter the larger D-Pad parent so Midna retains her existing visual size.
    const f32 scale = g_drawHIO.mMidnaIconScale * (0.95f / 1.30f);
    midnaPane->scale(scale, scale);
    midnaPane->move(positionX, positionY);

    const u8 anchorAlpha = anchorPane->getAlpha();
    set_pane_tree_alpha_visible(
        midnaPane, anchorPane->isVisible() && anchorAlpha > 0, anchorAlpha);
}

bool z_item_menu_or_pause_context() {
    const u8 windowStatus = dMeter2Info_getWindowStatus();
    return windowStatus != 0 || dMeter2Info_getPauseStatus() != 0 || dComIfGp_isPauseFlag() ||
           dComIfGp_event_runCheck() || dMeter2Info_isShopTalkFlag() ||
           dMsgObject_isTalkNowCheck();
}

bool midna_unlocked() {
    const daAlink_c* link = daAlink_getAlinkActorClass();
    return (link != nullptr && link->checkWolf()) || dComIfGs_getTransformStatus() != 0 ||
           dComIfGs_isEventBit(0x0520) || dComIfGs_isEventBit(0x0510) ||
           dComIfGs_isEventBit(0x0501) || dComIfGs_isEventBit(0x0640) ||
           dComIfGs_isEventBit(0x0504) || dComIfGs_isEventBit(0x0502);
}

bool bow_mix_item(u8 itemNo) {
    switch (itemNo) {
    case dItemNo_NORMAL_BOMB_e:
    case dItemNo_WATER_BOMB_e:
    case dItemNo_POKE_BOMB_e:
    case dItemNo_HAWK_EYE_e:
        return true;
    default:
        return false;
    }
}

u8 combine_select_item(u8 playItem, u8 mixSlot) {
    if (mixSlot == dItemNo_NONE_e) {
        return playItem;
    }

    u8 saveItem = dComIfGs_getItem(mixSlot, false);
    if (saveItem == dItemNo_BOW_e) {
        saveItem = playItem;
        playItem = dItemNo_BOW_e;
    } else if (saveItem == dItemNo_FISHING_ROD_1_e) {
        saveItem = playItem;
        playItem = dItemNo_FISHING_ROD_1_e;
    }

    if (playItem == dItemNo_BOW_e) {
        switch (saveItem) {
        case dItemNo_NORMAL_BOMB_e:
        case dItemNo_WATER_BOMB_e:
        case dItemNo_POKE_BOMB_e:
            return dItemNo_BOMB_ARROW_e;
        case dItemNo_HAWK_EYE_e:
            return dItemNo_HAWK_ARROW_e;
        default:
            break;
        }
    } else if (playItem == dItemNo_FISHING_ROD_1_e) {
        switch (saveItem) {
        case dItemNo_BEE_CHILD_e:
            return dItemNo_BEE_ROD_e;
        case dItemNo_WORM_e:
            return dItemNo_WORM_ROD_e;
        case dItemNo_ZORAS_JEWEL_e:
            return dItemNo_JEWEL_ROD_e;
        default:
            break;
        }
    }

    return playItem;
}

u8 resolved_select_item(int index) {
    const u8 slot = dComIfGs_getSelectItemIndex(index);
    if (slot == dItemNo_NONE_e) {
        return dItemNo_NONE_e;
    }

    return combine_select_item(dComIfGs_getItem(slot, false), dComIfGs_getMixItemIndex(index));
}

void sync_play_select_item(int index) {
    if (index != kZItemSlot) {
        return;
    }

    g_dComIfG_gameInfo.play.setSelectItem(index, resolved_select_item(index));
}

int find_select_button(daAlink_c* link, int itemNo) {
    if (link == nullptr) {
        return kSelectItemNotFound;
    }

    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (link->checkGroupItem(itemNo, resolved_select_item(i))) {
            return i;
        }
    }
    return kSelectItemNotFound;
}

bool item_needs_z_valid_button(int itemNo) {
    return itemNo == dItemNo_HVY_BOOTS_e || itemNo == dItemNo_SPINNER_e;
}

bool z_heavy_boots_selected(daAlink_c* link) {
    return link != nullptr &&
           link->checkGroupItem(dItemNo_HVY_BOOTS_e, resolved_select_item(kZItemSlot));
}

bool z_heavy_boots_held(daAlink_c* link) {
    return link != nullptr && (link->mItemButton & daAlink_c::BTN_Z) != 0;
}

bool z_heavy_boots_input_locked(daAlink_c* link) {
    return s_zHeavyBootsGuardLink == link && s_zHeavyBootsWaitRelease;
}

bool z_heavy_boots_forced_off_context(daAlink_c* link) {
    if (link == nullptr) {
        return true;
    }

    if (link->checkWolf() || link->checkEventRun() || link->checkDeadHP() ||
        link->checkCanoeRide() || link->checkHorseRide() || link->checkBoardRide() ||
        link->checkSpinnerRide())
    {
        return true;
    }

    switch (link->mProcID) {
    case daAlink_c::PROC_DIVE_JUMP:
    case daAlink_c::PROC_SMALL_JUMP:
    case daAlink_c::PROC_CANOE_RIDE:
    case daAlink_c::PROC_CANOE_JUMP_RIDE:
    case daAlink_c::PROC_CANOE_GETOFF:
    case daAlink_c::PROC_HORSE_RIDE:
    case daAlink_c::PROC_HORSE_GETOFF:
    case daAlink_c::PROC_BOARD_RIDE:
    case daAlink_c::PROC_SPINNER_READY:
        return true;
    default:
        return false;
    }
}

void clear_z_heavy_boots_input_lock() {
    s_zHeavyBootsGuardLink = nullptr;
    s_zHeavyBootsManualToggleOff = false;
    s_zHeavyBootsWaitRelease = false;
    s_zHeavyBootsGuardFrames = 0;
}

void lock_z_heavy_boots_input(daAlink_c* link, bool manualToggleOff) {
    s_zHeavyBootsGuardLink = link;
    s_zHeavyBootsManualToggleOff = manualToggleOff;
    s_zHeavyBootsWaitRelease = true;
    s_zHeavyBootsGuardFrames = manualToggleOff ? 24 : 0;
}

void tick_z_heavy_boots_guard(daAlink_c* link) {
    if (s_zHeavyBootsGuardLink == nullptr) {
        s_zHeavyBootsWaitRelease = false;
        s_zHeavyBootsGuardFrames = 0;
        return;
    }

    if (s_zHeavyBootsGuardLink != link || !s_zHeavyBootsWaitRelease) {
        return;
    }

    if (s_zHeavyBootsManualToggleOff) {
        if (s_zHeavyBootsGuardFrames != 0) {
            --s_zHeavyBootsGuardFrames;
        } else {
            clear_z_heavy_boots_input_lock();
        }
        return;
    }

    if (!z_heavy_boots_held(link)) {
        clear_z_heavy_boots_input_lock();
    }
}

u8 cursor_for_slot(dMenu_Ring_c* ring, u8 slot) {
    return slot == dItemNo_NONE_e ? dItemNo_NONE_e : ring->getCursorPos(slot);
}

void sync_ring_fields(dMenu_Ring_c* ring) {
    ring->mXButtonSlot = cursor_for_slot(ring, dComIfGs_getSelectItemIndex(SELECT_ITEM_X));
    ring->mYButtonSlot = cursor_for_slot(ring, dComIfGs_getSelectItemIndex(SELECT_ITEM_Y));
    ring->field_0x6ac = cursor_for_slot(ring, dComIfGs_getSelectItemIndex(kZItemSlot));
    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        ring->field_0x6b4[i] = dComIfGs_getSelectItemIndex(i);
        ring->field_0x6b8[i] = dComIfGs_getMixItemIndex(i);
    }
}

void store_select_slots(const std::array<u8, kExtendedSelectItemCount>& slots,
    const std::array<u8, kExtendedSelectItemCount>& mixes) {
    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        dComIfGs_setMixItemIndex(i, mixes[i]);
        dComIfGs_setSelectItemIndex(i, slots[i]);
        sync_play_select_item(i);
    }
}

bool z_mix_item_on(dMenu_Ring_c* ring) {
    if (ring == nullptr || ring->mPlayerIsWolf ||
        dComIfGs_getItem(ring->mItemSlots[ring->mCurrentSlot], false) == dItemNo_NONE_e)
    {
        return false;
    }

    if (!bow_mix_item(dComIfGs_getItem(ring->mItemSlots[ring->mCurrentSlot], false))) {
        return false;
    }

    return (dComIfGs_getSelectItemIndex(kZItemSlot) == SLOT_4 &&
               dComIfGs_getMixItemIndex(kZItemSlot) == dItemNo_NONE_e) ||
           dComIfGs_getMixItemIndex(kZItemSlot) == SLOT_4;
}

bool z_mix_item_off(dMenu_Ring_c* ring) {
    return ring != nullptr && !ring->mPlayerIsWolf &&
           dComIfGs_getItem(ring->mItemSlots[ring->mCurrentSlot], false) != dItemNo_NONE_e &&
           dComIfGs_getMixItemIndex(kZItemSlot) == SLOT_4 &&
           ring->mItemSlots[ring->mCurrentSlot] == dComIfGs_getSelectItemIndex(kZItemSlot);
}

bool set_z_mix_item(dMenu_Ring_c* ring) {
    if (!z_mix_item_on(ring) && !z_mix_item_off(ring)) {
        return false;
    }

    for (int i = 0; i < MAX_SELECT_ITEM; ++i) {
        ring->setSelectItemForce(i);
    }

    std::array<u8, kExtendedSelectItemCount> slots = {
        dComIfGs_getSelectItemIndex(SELECT_ITEM_X),
        dComIfGs_getSelectItemIndex(SELECT_ITEM_Y),
        dComIfGs_getSelectItemIndex(kZItemSlot),
    };
    std::array<u8, kExtendedSelectItemCount> mixes = {
        dComIfGs_getMixItemIndex(SELECT_ITEM_X),
        dComIfGs_getMixItemIndex(SELECT_ITEM_Y),
        dComIfGs_getMixItemIndex(kZItemSlot),
    };

    if (z_mix_item_off(ring)) {
        Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_COMBINE_OFF, nullptr, 0, 0, 1.0f, 1.0f,
            -1.0f, -1.0f, 0);
        slots[kZItemSlot] = SLOT_4;
        mixes[kZItemSlot] = dItemNo_NONE_e;
        ring->field_0x6cb = dComIfGs_getSelectItemIndex(kZItemSlot);
        ring->field_0x6cd = kZItemSlot;
    } else {
        Z2GetAudioMgr()->seStart(Z2SE_SY_ITEM_COMBINE_ON, nullptr, 0, 0, 1.0f, 1.0f,
            -1.0f, -1.0f, 0);
        slots[kZItemSlot] = ring->mItemSlots[ring->mCurrentSlot];
        mixes[kZItemSlot] = SLOT_4;
        ring->field_0x6cd = dItemNo_NONE_e;

        for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
            if (i == kZItemSlot) {
                continue;
            }
            if (slots[i] == slots[kZItemSlot]) {
                slots[i] = dItemNo_NONE_e;
                mixes[i] = dItemNo_NONE_e;
            }
        }
    }

    store_select_slots(slots, mixes);
    sync_ring_fields(ring);
    ring->field_0x6ac = cursor_for_slot(ring, slots[kZItemSlot]);
    ring->field_0x6b3 = kZItemSlot;
    ring->field_0x674[kZItemSlot] = 1;
    ring->setJumpItem(false);
    return true;
}

void assign_current_item(dMenu_Ring_c* ring, u8 targetSlot) {
    const u8 selectedSlot = ring->mItemSlots[ring->mCurrentSlot];
    std::array<u8, kExtendedSelectItemCount> slots = {
        dComIfGs_getSelectItemIndex(SELECT_ITEM_X),
        dComIfGs_getSelectItemIndex(SELECT_ITEM_Y),
        dComIfGs_getSelectItemIndex(kZItemSlot),
    };
    std::array<u8, kExtendedSelectItemCount> mixes = {
        dComIfGs_getMixItemIndex(SELECT_ITEM_X),
        dComIfGs_getMixItemIndex(SELECT_ITEM_Y),
        dComIfGs_getMixItemIndex(kZItemSlot),
    };

    u8 sourceSlot = dItemNo_NONE_e;
    bool selectedWasMixItem = false;
    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (i == targetSlot) {
            continue;
        }
        if (slots[i] == selectedSlot) {
            sourceSlot = i;
            break;
        }
        if (mixes[i] == selectedSlot) {
            sourceSlot = i;
            selectedWasMixItem = true;
            break;
        }
    }

    const u8 oldTargetSlot = slots[targetSlot];
    const u8 oldTargetMix = mixes[targetSlot];
    const bool targetAlreadyHeldSelected = oldTargetSlot == selectedSlot;

    slots[targetSlot] = selectedSlot;
    mixes[targetSlot] = dItemNo_NONE_e;

    if (sourceSlot != dItemNo_NONE_e) {
        if (targetAlreadyHeldSelected) {
            if (selectedWasMixItem) {
                mixes[sourceSlot] = dItemNo_NONE_e;
            } else {
                slots[sourceSlot] = dItemNo_NONE_e;
                mixes[sourceSlot] = dItemNo_NONE_e;
            }
        } else {
            slots[sourceSlot] = oldTargetSlot;
            mixes[sourceSlot] = oldTargetSlot == dItemNo_NONE_e ? dItemNo_NONE_e : oldTargetMix;
        }
    }

    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (i == targetSlot || i == sourceSlot) {
            continue;
        }
        if (slots[i] == selectedSlot) {
            slots[i] = dItemNo_NONE_e;
            mixes[i] = dItemNo_NONE_e;
        } else if (mixes[i] == selectedSlot) {
            mixes[i] = dItemNo_NONE_e;
        }
    }

    store_select_slots(slots, mixes);
    sync_ring_fields(ring);
    ring->field_0x6b3 = targetSlot;
    ring->field_0x674[targetSlot] = 1;
    ring->setJumpItem(true);
}

bool item_assign_allowed(dMenu_Ring_c* ring) {
    if (ring == nullptr) {
        return false;
    }

    const u8 item = dComIfGs_getItem(ring->mItemSlots[ring->mCurrentSlot], false);
    return ring->mStatus == dMenu_Ring_c::STATUS_WAIT &&
           ring->mOldStatus != dMenu_Ring_c::STATUS_EXPLAIN_FORCE &&
           ring->mOldStatus != dMenu_Ring_c::STATUS_EXPLAIN &&
           ring->mpItemExplain->getStatus() == 0 &&
           !ring->mPlayerIsWolf &&
           item != dItemNo_NONE_e;
}

u8 vanilla_assign_target() {
    if (mDoCPd_c::getTrigX(PAD_1)) {
        return SELECT_ITEM_X;
    }
    if (mDoCPd_c::getTrigY(PAD_1)) {
        return SELECT_ITEM_Y;
    }
    return dItemNo_NONE_e;
}

void capture_vanilla_assign(dMenu_Ring_c* ring) {
    s_pendingAssign = {};
    const u8 targetSlot = vanilla_assign_target();
    if (!item_assign_allowed(ring) || targetSlot == dItemNo_NONE_e) {
        return;
    }

    s_pendingAssign = {
        .ring = ring,
        .targetSlot = targetSlot,
        .selectedSlot = ring->mItemSlots[ring->mCurrentSlot],
        .oldTargetSlot = dComIfGs_getSelectItemIndex(targetSlot),
        .oldTargetMix = dComIfGs_getMixItemIndex(targetSlot),
        .active = true,
    };
}

void rotate_pending_duplicate(dMenu_Ring_c* ring) {
    if (!s_pendingAssign.active || s_pendingAssign.ring != ring) {
        s_pendingAssign = {};
        return;
    }

    std::array<u8, kExtendedSelectItemCount> slots = {
        ring->field_0x6b4[SELECT_ITEM_X],
        ring->field_0x6b4[SELECT_ITEM_Y],
        dComIfGs_getSelectItemIndex(kZItemSlot),
    };
    std::array<u8, kExtendedSelectItemCount> mixes = {
        ring->field_0x6b8[SELECT_ITEM_X],
        ring->field_0x6b8[SELECT_ITEM_Y],
        dComIfGs_getMixItemIndex(kZItemSlot),
    };

    const u8 targetSlot = s_pendingAssign.targetSlot;
    const u8 selectedSlot = s_pendingAssign.selectedSlot;
    u8 sourceSlot = dItemNo_NONE_e;
    bool selectedWasMixItem = false;
    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (i == targetSlot) {
            continue;
        }
        if (slots[i] == selectedSlot) {
            sourceSlot = i;
            break;
        }
        if (mixes[i] == selectedSlot) {
            sourceSlot = i;
            selectedWasMixItem = true;
            break;
        }
    }

    if (sourceSlot != dItemNo_NONE_e) {
        if (s_pendingAssign.oldTargetSlot == selectedSlot) {
            if (selectedWasMixItem) {
                mixes[sourceSlot] = dItemNo_NONE_e;
            } else {
                slots[sourceSlot] = dItemNo_NONE_e;
                mixes[sourceSlot] = dItemNo_NONE_e;
            }
        } else {
            slots[sourceSlot] = s_pendingAssign.oldTargetSlot;
            mixes[sourceSlot] =
                s_pendingAssign.oldTargetSlot == dItemNo_NONE_e ? dItemNo_NONE_e :
                                                                  s_pendingAssign.oldTargetMix;
        }
    }

    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (i == targetSlot || i == sourceSlot) {
            continue;
        }
        if (slots[i] == selectedSlot) {
            slots[i] = dItemNo_NONE_e;
            mixes[i] = dItemNo_NONE_e;
        } else if (mixes[i] == selectedSlot) {
            mixes[i] = dItemNo_NONE_e;
        }
    }

    store_select_slots(slots, mixes);
    sync_ring_fields(ring);
    ring->field_0x674[targetSlot] = 1;
    if (sourceSlot != dItemNo_NONE_e) {
        ring->field_0x674[sourceSlot] = 1;
    }
    s_pendingAssign = {};
}

HookAction before_get_select_item(ModContext*, void* args, void* retval, void*) {
    const int index = mods::arg<int>(args, 0);
    if (index != kZItemSlot) {
        return HOOK_CONTINUE;
    }

    *static_cast<u8*>(retval) = resolved_select_item(index);
    return HOOK_SKIP_ORIGINAL;
}

void after_set_select_item(ModContext*, void* args, void*, void*) {
    sync_play_select_item(mods::arg<int>(args, 0));
}

void repair_missing_y_item_binding() {
    u32 mappingCount = 0;
    PADButtonMapping* mappings = PADGetButtonMappings(PAD_1, &mappingCount);
    if (mappings == nullptr || mappingCount == 0) {
        s_checkedControllerBindings = false;
        return;
    }

    u32 vid = 0;
    u32 pid = 0;
    PADGetVidPid(PAD_1, &vid, &pid);
    if (s_checkedControllerBindings && vid == s_checkedControllerVid &&
        pid == s_checkedControllerPid)
    {
        return;
    }

    s_checkedControllerBindings = true;
    s_checkedControllerVid = vid;
    s_checkedControllerPid = pid;

    PADButtonMapping* yMapping = nullptr;
    std::array<bool, 4> usedFaceButtons = {};
    for (u32 i = 0; i < mappingCount; ++i) {
        PADButtonMapping& mapping = mappings[i];
        if (mapping.padButton == PAD_BUTTON_Y) {
            yMapping = &mapping;
        }
        if ((mapping.padButton == PAD_BUTTON_A || mapping.padButton == PAD_BUTTON_B ||
             mapping.padButton == PAD_BUTTON_X || mapping.padButton == PAD_BUTTON_Y) &&
            mapping.nativeButton <= kLastNativeFaceButton)
        {
            usedFaceButtons[mapping.nativeButton] = true;
        }
    }

    if (yMapping == nullptr || yMapping->nativeButton != PAD_NATIVE_BUTTON_INVALID) {
        return;
    }

    // A partially configured Nintendo-style profile can leave Y unbound. Use
    // the one unused face button so the repair also respects swapped A/B and X/Y layouts.
    for (u32 nativeButton = kFirstNativeFaceButton;
         nativeButton <= kLastNativeFaceButton; ++nativeButton)
    {
        if (!usedFaceButtons[nativeButton]) {
            PADSetButtonMapping(PAD_1, {nativeButton, PAD_BUTTON_Y});
            break;
        }
    }
}

void after_pad_read(ModContext*, void*, void*, void*) {
    repair_missing_y_item_binding();

    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);
    if (z_item_menu_or_pause_context()) {
        s_dpadMidnaHeld = false;
        s_dpadMidnaTrig = false;
        return;
    }

    // The Wii U presentation uses the physical R shoulder for the third item.
    // Internally that slot remains GameCube Z, so translate a profile's digital
    // R binding into Z while gameplay is active.
    if ((pad.mButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mButtonFlags = (pad.mButtonFlags & ~PAD_TRIGGER_R) | PAD_TRIGGER_Z;
    }
    if ((pad.mPressedButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mPressedButtonFlags =
            (pad.mPressedButtonFlags & ~PAD_TRIGGER_R) | PAD_TRIGGER_Z;
    }

    s_dpadMidnaHeld = (pad.mButtonFlags & PAD_BUTTON_UP) != 0;
    s_dpadMidnaTrig = (pad.mPressedButtonFlags & PAD_BUTTON_UP) != 0;
    if (s_dpadMidnaHeld) {
        pad.mButtonFlags &= ~PAD_BUTTON_UP;
    }
    if (s_dpadMidnaTrig) {
        pad.mPressedButtonFlags &= ~PAD_BUTTON_UP;
    }
}

void after_meter_move_button_cross(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2_c*>(args, 0);
    if (meter == nullptr || meter->getMeterDrawPtr() == nullptr) {
        return;
    }

    // The original GameCube HUD animates the D-Pad toward the minimap whenever the map is
    // visible. Redrawing at the stationary base coordinates reproduces Twilight HD HUD's independent
    // D-Pad layout while preserving alpha animation and any additional pane transforms.
    meter->getMeterDrawPtr()->drawButtonCross(g_drawHIO.mButtonCrossOFFPosX, 0.0f);
}

void after_ring_create(ModContext*, void* args, void*, void*) {
    create_ring_z_prompt(mods::arg<dMenu_Ring_c*>(args, 0));
}

HookAction before_ring_delete(ModContext*, void* args, void*, void*) {
    destroy_ring_z_prompt(mods::arg<dMenu_Ring_c*>(args, 0));
    return HOOK_CONTINUE;
}

void after_ring_draw(ModContext*, void* args, void*, void*) {
    draw_ring_z_prompt(mods::arg<dMenu_Ring_c*>(args, 0));
}

HookAction before_meter_draw(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
    update_z_hud_item(meter);
    restore_archive_face_button_diamond(meter);
    align_action_text_shadow_layers(meter);
    apply_wii_u_r_button_art(meter);
    apply_wii_u_item_num_layout(meter);
    // The bundled main2D.arc supplies the Wii U prompt art and its native pane
    // geometry. Applying the older synthetic round-button and relocation pass
    // here would overwrite those resources and move the panes twice.
    apply_wii_u_archive_layout_corrections(meter);
    stabilize_wii_u_rupee_counter(meter);
    apply_wii_u_dpad_style(meter);
    apply_hud_backing_visibility(meter);
    return HOOK_CONTINUE;
}

void after_meter_draw(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
    restore_wii_u_item_num_layout(meter);
    draw_z_hud_item_meters(meter);
    draw_uniform_rupee_digits(meter);
    draw_wolf_action_icons(meter);
}

HookAction before_meter_draw_kantera(ModContext*, void* args, void*, void*) {
    mods::arg_ref<f32>(args, 3) += 100.0f;
    return HOOK_CONTINUE;
}

HookAction before_meter_draw_oxygen(ModContext*, void* args, void*, void*) {
    mods::arg_ref<f32>(args, 3) += 100.0f;
    return HOOK_CONTINUE;
}

void after_meter_midna_alpha(ModContext*, void* args, void*, void*) {
    position_midna_hud(mods::arg<dMeter2Draw_c*>(args, 0));
}

HookAction before_meter_map_draw(ModContext*, void* args, void*, void*) {
    apply_wii_u_minimap_layout(mods::arg<dMeterMap_c*>(args, 0));
    return HOOK_CONTINUE;
}

void after_meter_map_draw(ModContext*, void* args, void*, void*) {
    restore_wii_u_minimap_layout(mods::arg<dMeterMap_c*>(args, 0));
}

HookAction before_ring_set_active_cursor(ModContext*, void* args, void*, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (ring == nullptr) {
        s_pendingAssign = {};
        return HOOK_CONTINUE;
    }

    if (!mDoCPd_c::getTrigZ(PAD_1) && !mDoCPd_c::getTrigR(PAD_1)) {
        capture_vanilla_assign(ring);
        return HOOK_CONTINUE;
    }

    s_pendingAssign = {};
    if (item_assign_allowed(ring)) {
        assign_current_item(ring, kZItemSlot);
        if (ring->mpItemExplain->getStatus() == 0) {
            ring->setStatus(dMenu_Ring_c::STATUS_WAIT);
            ring->stick_wait_init();
        }
    } else {
        Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, nullptr, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0);
    }

    return HOOK_SKIP_ORIGINAL;
}

void after_ring_set_active_cursor(ModContext*, void* args, void*, void*) {
    rotate_pending_duplicate(mods::arg<dMenu_Ring_c*>(args, 0));
}

HookAction before_ring_is_mix_item_on(ModContext*, void* args, void* retval, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (!z_mix_item_on(ring)) {
        return HOOK_CONTINUE;
    }

    *static_cast<bool*>(retval) = true;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_ring_is_mix_item_off(ModContext*, void* args, void* retval, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    if (!z_mix_item_off(ring)) {
        return HOOK_CONTINUE;
    }

    *static_cast<bool*>(retval) = true;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_midna_talk_trigger(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<const daAlink_c*>(args, 0);
    if (link == nullptr) {
        return HOOK_CONTINUE;
    }

    *static_cast<BOOL*>(retval) = s_dpadMidnaTrig || consume_touch_midna_trigger();
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_check_item_button_change(ModContext*, void* args, void*, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    if (link == nullptr) {
        return HOOK_CONTINUE;
    }

    if (link->mProcID != daAlink_c::PROC_CANOE_PADDLE_PUT &&
        link->mEquipItem != dItemNo_NONE_e &&
        !link->checkEquipAnime())
    {
        for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
            const u8 next = (i + 1) % kExtendedSelectItemCount;
            if (link->mEquipItem == resolved_select_item(i) &&
                (link->mEquipItem != resolved_select_item(next) || link->mSelectItemId != next))
            {
                link->mSelectItemId = i;
            }
        }
    }
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_check_item_change_from_button(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    if (link == nullptr) {
        return HOOK_CONTINUE;
    }

    BOOL result = FALSE;
    if (link->checkModeFlg(4) &&
        !link->checkEquipAnime() &&
        !link->checkBoomerangThrowAnime() &&
        !link->checkCopyRodThrowAnime() &&
        !link->checkKandelaarSwingAnime())
    {
        if (
#if PLATFORM_GCN
            dComIfGs_getSelectEquipSword() != dItemNo_NONE_e &&
#endif
            !link->checkNotBattleStage() &&
            !link->checkCanoeRide() &&
            (!link->checkModeFlg(0x40000) || link->checkEquipHeavyBoots()) &&
            link->mEquipItem != 0x103 &&
            link->swordTrigger())
        {
            if (!link->checkEndResetFlg1(daPy_py_c::ERFLG1_SWORD_TRIGGER_NON)) {
                link->swordEquip(TRUE);
            }
        } else if (link->checkCanoeRide() &&
                   !link->checkStageName("F_SP103") &&
                   !link->checkCanoeSlider() &&
                   !link->checkFisingRodLure() &&
                   link->swordTrigger())
        {
            link->itemEquip(0x105);
        } else {
            for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
                const int procType = link->checkNewItemChange(i);
                if (procType != 0 && link->itemTriggerCheck(1 << i)) {
                    if (i == kZItemSlot &&
                        link->checkGroupItem(dItemNo_HVY_BOOTS_e, resolved_select_item(i)))
                    {
                        if (z_heavy_boots_input_locked(link)) {
                            continue;
                        }
                        lock_z_heavy_boots_input(link, link->checkEquipHeavyBoots());
                    }
                    result = link->changeItemTriggerKeepProc(i, procType);
                    *static_cast<BOOL*>(retval) = result;
                    return HOOK_SKIP_ORIGINAL;
                }
            }

            if (link->doTrigger() && dComIfGp_getDoStatus() == BUTTON_STATUS_PUT_AWAY) {
                if (link->mEquipItem != dItemNo_KANTERA_e && link->checkNoResetFlg2(daPy_py_c::FLG2_UNK_1)) {
                    link->offKandelaarModel();
                } else if (link->mSwordFlourishTimer != 0 && link->mEquipItem == 0x103 &&
                           !link->checkWoodSwordEquip() && !link->checkModeFlg(0x402))
                {
                    result = link->procSwordUnequipSpInit();
                } else {
                    link->allUnequip(TRUE);
                }
            } else if (link->mEquipItem == dItemNo_NONE_e &&
                       link->mThrowBoomerangAcKeep.getActor() == nullptr &&
                       !link->checkCanoeRide() &&
                       link->checkNoUpperAnime() &&
                       link->checkNoResetFlg2(daPy_py_c::FLG2_UNK_1))
            {
                for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
                    if (resolved_select_item(i) == dItemNo_KANTERA_e) {
                        link->mSelectItemId = i;
                    }
                }
                link->itemEquip(dItemNo_KANTERA_e);
                link->onNoResetFlg1(daPy_py_c::FLG1_UNK_40);
            } else if (link->mEquipItem != 0x103 &&
                       link->mEquipItem != dItemNo_NONE_e &&
                       link->mEquipItem != 0x10B &&
                       link->mEquipItem != 0x102 &&
                       (!link->checkCanoeRide() || !link->checkFisingRodLure()))
            {
                if (!link->checkEventRun() ||
                    std::strcmp(dComIfGp_getEventManager().getRunEventName(), "ANGER") != 0)
                {
                    if (std::strcmp(dComIfGp_getEventManager().getRunEventName(), "ANGER2") != 0 &&
                        find_select_button(link, link->mEquipItem) == kSelectItemNotFound)
                    {
                        link->allUnequip(TRUE);
                    }
                }
            }
        }
    }

    *static_cast<BOOL*>(retval) = result;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_check_set_item_trigger(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    const int itemNo = mods::arg<int>(args, 1);
    if (link == nullptr) {
        return HOOK_CONTINUE;
    }

    for (u8 i = 0; i < kExtendedSelectItemCount; ++i) {
        if (!link->checkGroupItem(itemNo, resolved_select_item(i)) || !link->itemTriggerCheck(1 << i)) {
            continue;
        }

        if (itemNo == dItemNo_HVY_BOOTS_e) {
            if (i == kZItemSlot) {
                if (link->checkEquipHeavyBoots()) {
                    if (!z_heavy_boots_input_locked(link) &&
                        link->checkNewItemChange(i) == kItemProcBootsEquip)
                    {
                        lock_z_heavy_boots_input(link, true);
                        link->changeItemTriggerKeepProc(i, kItemProcBootsEquip);
                    }
                    *static_cast<int*>(retval) = 0;
                    return HOOK_SKIP_ORIGINAL;
                }
                if (z_heavy_boots_input_locked(link)) {
                    *static_cast<int*>(retval) = 0;
                    return HOOK_SKIP_ORIGINAL;
                }
                lock_z_heavy_boots_input(link, link->checkEquipHeavyBoots());
            }
        } else {
            link->mSelectItemId = i;
        }

        *static_cast<int*>(retval) = 1;
        return HOOK_SKIP_ORIGINAL;
    }

    *static_cast<int*>(retval) = 0;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_check_item_set_button(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    const int itemNo = mods::arg<int>(args, 1);
    if (link == nullptr || !item_needs_z_valid_button(itemNo)) {
        return HOOK_CONTINUE;
    }

    if (!link->checkGroupItem(itemNo, resolved_select_item(kZItemSlot))) {
        return HOOK_CONTINUE;
    }

    *static_cast<int*>(retval) = SELECT_ITEM_X;
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_set_heavy_boots(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    const int enable = mods::arg<int>(args, 1);
    if (link == nullptr || !link->checkEquipHeavyBoots() ||
        link->checkNotHeavyBootsStage() || !z_heavy_boots_selected(link))
    {
        return HOOK_CONTINUE;
    }

    if (enable != 0 && s_zHeavyBootsGuardLink == link && s_zHeavyBootsManualToggleOff) {
        clear_z_heavy_boots_input_lock();
        return HOOK_CONTINUE;
    }

    if (enable != 0 && z_heavy_boots_input_locked(link)) {
        *static_cast<int*>(retval) = 0;
        return HOOK_SKIP_ORIGINAL;
    }

    if (enable == 0 && z_heavy_boots_forced_off_context(link)) {
        clear_z_heavy_boots_input_lock();
        return HOOK_CONTINUE;
    }

    if (!link->checkEquipHeavyBoots()) {
        return HOOK_CONTINUE;
    }

    *static_cast<int*>(retval) = 0;
    return HOOK_SKIP_ORIGINAL;
}

void after_player_execute(ModContext*, void* args, void*, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    if (link == nullptr || link->checkWolf()) {
        return;
    }

    tick_z_heavy_boots_guard(link);
    sync_play_select_item(kZItemSlot);
    if (resolved_select_item(kZItemSlot) != dItemNo_NONE_e) {
        dMeter2Info_onUseButton(METER2_USEBUTTON_Z);
    }
}

template <class Hook>
ModResult add_pre_hook(const char* name, HookPreFn callback, ModError* error) {
    const ModResult result = mods::hook_add_pre<Hook>(svc_hook, callback);
    if (result == MOD_OK) {
        return MOD_OK;
    }

    char message[192];
    std::snprintf(message, sizeof(message),
        "failed to install Twilight HD HUD hook: %s (error %d)",
        name, static_cast<int>(result));
    return mods::set_error(error, result, message);
}

template <class Hook>
ModResult add_post_hook(const char* name, HookPostFn callback, ModError* error) {
    const ModResult result = mods::hook_add_post<Hook>(svc_hook, callback);
    if (result == MOD_OK) {
        return MOD_OK;
    }

    char message[192];
    std::snprintf(message, sizeof(message),
        "failed to install Twilight HD HUD hook: %s (error %d)",
        name, static_cast<int>(result));
    return mods::set_error(error, result, message);
}

void free_resource(ResourceBuffer& resource) {
    if (resource.data != nullptr) {
        svc_resource->free(mod_ctx, &resource);
        resource = RESOURCE_BUFFER_INIT;
    }
}

bool load_picture(const char* path, const char* errorMessage,
    ResourceBuffer& resource, J2DPicture*& picture) {
    if (picture != nullptr) {
        return true;
    }

    free_resource(resource);
    if (svc_resource->load(mod_ctx, path, &resource) != MOD_OK ||
        resource.data == nullptr || resource.size < sizeof(ResTIMG))
    {
        free_resource(resource);
        svc_log->warn(mod_ctx, errorMessage);
        return false;
    }

    picture = JKR_NEW J2DPicture(static_cast<const ResTIMG*>(resource.data));
    if (picture == nullptr) {
        free_resource(resource);
        svc_log->warn(mod_ctx, errorMessage);
        return false;
    }
    return true;
}

void free_picture(ResourceBuffer& resource, J2DPicture*& picture) {
    JKR_DELETE(picture);
    picture = nullptr;
    free_resource(resource);
}

}  // namespace

void initialize_wolf_action_icons() {
    load_picture("hud/wolf_actions/sense.bti", "Unable to load the Sense HUD icon",
        s_senseIconResource, s_senseIconPicture);
    load_picture("hud/wolf_actions/dig-left.bti", "Unable to load the Dig HUD icon",
        s_digIconResource, s_digIconPicture);
    load_picture("hud/wolf_actions/attack.bti", "Unable to load the Attack HUD icon",
        s_attackIconResource, s_attackIconPicture);
}

void initialize_face_button_textures() {
    if (svc_resource->load(mod_ctx, "hud/face-button-a.bti", &s_faceButtonAResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the smooth A button texture");
    }
    if (svc_resource->load(mod_ctx, "hud/face-button-b.bti", &s_faceButtonBResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the smooth B button texture");
    }
}

void shutdown_face_button_textures() {
    free_resource(s_faceButtonAResource);
    free_resource(s_faceButtonBResource);
}

void shutdown_item_slot_resources() {
    destroy_ring_z_prompt(s_ringZPrompt.ring);
}

void shutdown_wolf_action_icons() {
    free_picture(s_senseIconResource, s_senseIconPicture);
    free_picture(s_digIconResource, s_digIconPicture);
    free_picture(s_attackIconResource, s_attackIconPicture);
}

ModResult install_item_slot_hooks(ModError* error) {
#define ADD_PRE(type, callback, name) \
    if (const ModResult result = add_pre_hook<type>(name, callback, error); result != MOD_OK) { \
        return result; \
    }
#define ADD_POST(type, callback, name) \
    if (const ModResult result = add_post_hook<type>(name, callback, error); result != MOD_OK) { \
        return result; \
    }

    ADD_PRE(GetSelectItemHook, before_get_select_item, "get selected item");
    ADD_POST(SetSelectItemHook, after_set_select_item, "set selected item");
    ADD_POST(PadReadHook, after_pad_read, "controller read");
    ADD_POST(RingCreateHook, after_ring_create, "item ring create");
#if !defined(_WIN32)
    ADD_PRE(RingDeleteHook, before_ring_delete, "item ring delete");
#endif
    ADD_POST(RingDrawHook, after_ring_draw, "item ring draw");
    ADD_PRE(MeterDrawHook, before_meter_draw, "HUD draw (before)");
    ADD_POST(MeterDrawHook, after_meter_draw, "HUD draw (after)");
    ADD_POST(MeterMoveButtonCrossHook, after_meter_move_button_cross, "D-pad update");
    ADD_PRE(MeterDrawKanteraHook, before_meter_draw_kantera, "lantern meter draw");
    ADD_PRE(MeterDrawOxygenHook, before_meter_draw_oxygen, "oxygen meter draw");
    ADD_POST(MeterMidnaAlphaHook, after_meter_midna_alpha, "Midna icon opacity");
    ADD_PRE(MeterMapDrawHook, before_meter_map_draw, "minimap draw (before)");
    ADD_POST(MeterMapDrawHook, after_meter_map_draw, "minimap draw (after)");
    ADD_PRE(RingSetActiveCursorHook, before_ring_set_active_cursor, "item ring cursor (before)");
    ADD_POST(RingSetActiveCursorHook, after_ring_set_active_cursor, "item ring cursor (after)");
    ADD_PRE(RingIsMixItemOnHook, before_ring_is_mix_item_on, "item combination enable");
    ADD_PRE(RingIsMixItemOffHook, before_ring_is_mix_item_off, "item combination disable");
    ADD_PRE(MidnaTalkTriggerHook, before_midna_talk_trigger, "Midna input");
    ADD_PRE(CheckItemButtonChangeHook, before_check_item_button_change, "item button change");
    ADD_PRE(CheckItemChangeFromButtonHook, before_check_item_change_from_button,
        "item change from button");
    ADD_PRE(CheckSetItemTriggerHook, before_check_set_item_trigger, "item trigger");
    ADD_PRE(CheckItemSetButtonHook, before_check_item_set_button, "item button lookup");
    ADD_PRE(SetHeavyBootsHook, before_set_heavy_boots, "heavy boots toggle");
    ADD_POST(PlayerExecuteHook, after_player_execute, "player update");

#undef ADD_PRE
#undef ADD_POST
    return MOD_OK;
}

}  // namespace twilight_hd_hud
