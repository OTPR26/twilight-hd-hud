#include "config.hpp"
#include "hud_layout.hpp"
#include "service_imports.hpp"

#include "global.h"
#include "Z2AudioLib/Z2AudioMgr.h"
#include "Z2AudioLib/Z2SeMgr.h"
#include "d/actor/d_a_alink.h"
#include "d/d_com_inf_game.h"
#include "d/d_bright_check.h"
#include "d/d_kantera_icon_meter.h"
#include "d/d_item.h"
#include "d/d_item_data.h"
#include "d/d_meter_HIO.h"
#include "d/d_meter_haihai.h"
#include "d/d_meter2.h"
#include "d/d_meter2_info.h"
#include "d/d_menu_window.h"
#include "d/d_pane_class.h"
#include "d/d_msg_object.h"
#include "JSystem/J2DGraph/J2DPane.h"
#include "JSystem/J2DGraph/J2DScreen.h"
#include "JSystem/J2DGraph/J2DPicture.h"
#include "JSystem/J2DGraph/J2DTextBox.h"
#include "JSystem/JKernel/JKRExpHeap.h"
#include "JSystem/JUtility/JUTFont.h"
#include "JSystem/JUtility/JUTResFont.h"
#define private public
#include "d/d_menu_item_explain.h"
#include "d/d_msg_out_font.h"
#include "d/d_msg_string.h"
#include "d/d_select_cursor.h"
#define PaneCache FileSelectPaneCache
#include "d/d_file_select.h"
#undef PaneCache
#include "d/d_file_sel_info.h"
#include "d/d_menu_save.h"
#include "d/d_menu_collect.h"
#include "d/d_menu_letter.h"
#include "d/d_menu_fishing.h"
#include "d/d_menu_skill.h"
#include "d/d_menu_insect.h"
#include "d/d_menu_fmap.h"
#include "d/d_menu_fmap2D.h"
#include "d/d_menu_dmap.h"
#include "d/d_menu_option.h"
#include "d/d_menu_ring.h"
#include "d/d_meter_map.h"
#include "d/d_meter_button.h"
#include "d/d_meter2_draw.h"
#undef private
#include "m_Do/m_Do_controller_pad.h"
#include "m_Do/m_Do_ext.h"
#include "m_Do/m_Do_graphic.h"
#include "mods/service.hpp"
#include "mods/svc/hook.h"
#include "mods/svc/hook.hpp"
#include "mods/svc/resource.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace twilight_hd_hud {
namespace {

constexpr int kSdlLeftTriggerAxis = 4;

constexpr u8 kZItemSlot = SELECT_ITEM_DOWN;
constexpr int kExtendedSelectItemCount = 3;
constexpr int kSelectItemNotFound = 3;
constexpr int kItemProcBootsEquip = 1;
int s_descenderCorrectionDrawDepth = 0;
dFile_select_c* s_activeFileSelect = nullptr;
bool s_fileSelectYesNoLayoutReady = false;
f32 s_fileSelectYesNoX[2] = {};
f32 s_fileSelectYesNoY[2] = {};

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
DEFINE_HOOK(&dMw_c::dMw_ring_delete, MenuRingDeleteHook);
DEFINE_HOOK(&dMenu_Ring_c::_draw, RingDrawHook);
DEFINE_HOOK(&dMenu_Ring_c::setActiveCursor, RingSetActiveCursorHook);
DEFINE_HOOK(&dMenu_Ring_c::setMixMessage, RingSetMixMessageHook);
DEFINE_HOOK(&dMenu_Ring_c::isMixItemOn, RingIsMixItemOnHook);
DEFINE_HOOK(&dMenu_Ring_c::isMixItemOff, RingIsMixItemOffHook);
DEFINE_HOOK(&dMenu_ItemExplain_c::draw, ItemExplainDrawHook);
DEFINE_HOOK(&dMeterButton_c::_execute, MeterButtonExecuteHook);
DEFINE_HOOK(&dMeterButton_c::draw, MeterButtonDrawHook);
DEFINE_HOOK(&dMeter2Draw_c::draw, MeterDrawHook);
DEFINE_HOOK(&dMeter2Draw_c::drawButtonZ, MeterDrawButtonZHook);
DEFINE_HOOK(&dMeter2_c::moveButtonCross, MeterMoveButtonCrossHook);
DEFINE_HOOK(&dMeter2Draw_c::drawKantera, MeterDrawKanteraHook);
DEFINE_HOOK(&dMeter2Draw_c::drawOxygen, MeterDrawOxygenHook);
DEFINE_HOOK(&dMeter2Draw_c::setButtonIconMidonaAlpha, MeterMidnaAlphaHook);
DEFINE_HOOK(&dMeterMap_c::draw, MeterMapDrawHook);
DEFINE_HOOK(&dMenu_Collect2D_c::_create, CollectCreateHook);
DEFINE_HOOK(&dMenu_Collect2D_c::_move, CollectMoveHook);
DEFINE_HOOK(&dMenu_Collect2D_c::_draw, CollectDrawHook);
DEFINE_HOOK(&dMenu_Collect2D_c::_delete, CollectDeleteHook);
#if defined(_WIN32)
DEFINE_HOOK(&dMenu_Collect2D_c::menuCollectWide, CollectWideHook);
#endif
DEFINE_HOOK(&dMenu_Letter_c::_create, LetterCreateHook);
DEFINE_HOOK(&dMenu_Letter_c::_draw, LetterDrawHook);
DEFINE_HOOK(&dMenu_Fishing_c::_create, FishingCreateHook);
DEFINE_HOOK(&dMenu_Fishing_c::_draw, FishingDrawHook);
DEFINE_HOOK(&dMenu_Skill_c::_create, SkillCreateHook);
DEFINE_HOOK(&dMenu_Skill_c::_draw, SkillDrawHook);
DEFINE_HOOK(&dMenu_Insect_c::_create, InsectCreateHook);
DEFINE_HOOK(&dMenu_Insect_c::_draw, InsectDrawHook);
DEFINE_HOOK(&dSelect_cursor_c::draw, SelectCursorDrawHook);
DEFINE_HOOK(&dSelect_cursor_c::update, SelectCursorUpdateHook);
DEFINE_HOOK(&dMenu_Fmap_c::_move, FmapMoveHook);
DEFINE_HOOK(&dMenu_Fmap_c::_draw, FmapDrawHook);
DEFINE_HOOK(&dMenu_Dmap_c::_draw, DmapDrawHook);
DEFINE_HOOK(&dMenu_Option_c::_create, OptionCreateHook);
DEFINE_HOOK(&dMenu_Option_c::_move, OptionMoveHook);
DEFINE_HOOK(&dMenu_Option_c::_draw, OptionDrawHook);
DEFINE_HOOK(&dMenu_Option_c::drawHaihai, OptionDrawArrowsHook);
DEFINE_HOOK(&dBrightCheck_c::screenSet, BrightCheckScreenSetHook);
DEFINE_HOOK(&dBrightCheck_c::_draw, BrightCheckDrawHook);
DEFINE_HOOK(&JUTResFont::drawChar_scale, ResFontDrawCharHook);
DEFINE_HOOK(&dFile_select_c::_create, FileSelectCreateHook);
DEFINE_HOOK(&dFile_select_c::_move, FileSelectMoveHook);
DEFINE_HOOK(&dFile_select_c::_draw, FileSelectDrawHook);
DEFINE_HOOK(&dDlst_FileSel_c::draw, FileSelectMainDrawHook);
DEFINE_HOOK(&dMenu_save_c::screenSet, SaveMenuScreenSetHook);
DEFINE_HOOK(&dMenu_save_c::_delete, SaveMenuDeleteHook);
DEFINE_HOOK(&dMenu_save_c::_draw2, SaveMenuDrawHook);
DEFINE_HOOK(&dMenu_save_c::menuSaveWide, SaveMenuWideHook);
DEFINE_HOOK(&dDlst_MenuSave_c::draw, SaveDlstDrawHook);
DEFINE_HOOK(&dMw_c::_execute, MenuWindowExecuteHook);
DEFINE_HOOK(&daAlink_c::midnaTalkTrigger, MidnaTalkTriggerHook);
DEFINE_HOOK(&daAlink_c::itemActionTrigger, ItemActionTriggerHook);
DEFINE_HOOK(&daAlink_c::setStickData, SetStickDataHook);
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
    J2DPicture* buttonX = nullptr;
    J2DPicture* buttonY = nullptr;
    J2DPicture* buttonR = nullptr;
    J2DTextBox* comboZL = nullptr;
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
ResourceBuffer s_blackProFaceButtonResources[4] = {
    RESOURCE_BUFFER_INIT, RESOURCE_BUFFER_INIT,
    RESOURCE_BUFFER_INIT, RESOURCE_BUFFER_INIT,
};
ResourceBuffer s_blackProBlankFaceButtonResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_blackProShoulderButtonResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_zlShoulderButtonResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_blackProZlShoulderButtonResource = RESOURCE_BUFFER_INIT;
J2DScreen* s_contextRButtonScreen = nullptr;
ResTIMG const* s_contextRButtonTexture = nullptr;
bool s_contextRButtonUsesZl = false;
struct ContextRPictureState {
    J2DPane* pane = nullptr;
    bool visible = false;
};
std::array<ContextRPictureState, 8> s_contextRPictureStates = {};
std::size_t s_contextRPictureStateCount = 0;
ResourceBuffer s_collectBackgroundResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_collectMenuButtonResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectBackgroundResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectRowResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectSelectedRowResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectClearRowResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectTitleRulesResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectBackLabelResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectPromptFlourishResource = RESOURCE_BUFFER_INIT;
ResourceBuffer s_fileSelectNumberResources[3] = {
    RESOURCE_BUFFER_INIT, RESOURCE_BUFFER_INIT, RESOURCE_BUFFER_INIT,
};
dMenu_save_c* s_activeSaveMenu = nullptr;
dMenu_Collect2D_c* s_activeCollectMenu = nullptr;
J2DPicture* s_collectTitleFrame = nullptr;
J2DPicture* s_collectSaveFrame = nullptr;
J2DPicture* s_collectOptionsFrame = nullptr;
J2DPicture* s_collectTopRule = nullptr;
J2DPicture* s_collectBottomRule = nullptr;
J2DPicture* s_collectTopRuleInner = nullptr;
J2DPicture* s_collectBottomRuleInner = nullptr;
J2DTextBox* s_collectTitleLabel = nullptr;
struct CollectPromptWidthState {
    J2DScreen* screen = nullptr;
    f32 rightOffset = 0.0f;
};
std::array<CollectPromptWidthState, 8> s_collectPromptWidthStates = {};
std::size_t s_nextCollectPromptWidthState = 0;
bool s_collectRailDiagnosticsLogged = false;
bool s_collectCursorDiagnosticsLogged = false;
bool s_collectLayoutReady = false;
dMeter2Draw_c* s_wiiURButtonMeter = nullptr;
dKantera_icon_c* s_zOilMeter = nullptr;
daAlink_c* s_zHeavyBootsGuardLink = nullptr;
bool s_zHeavyBootsManualToggleOff = false;
bool s_zHeavyBootsWaitRelease = false;
u8 s_zHeavyBootsGuardFrames = 0;
bool s_dpadMidnaHeld = false;
bool s_dpadMidnaTrig = false;
bool s_fixedZlHeld = false;
bool s_fixedZlTrig = false;
u32 s_menuWindowSuppressedHeld = 0;
u32 s_menuWindowSuppressedTrig = 0;

enum class DusklightActionBind {
    FirstPersonCamera,
    CallMidna,
};

using GetActionBindTrigFn = bool (*)(DusklightActionBind, u32);
GetActionBindTrigFn s_getActionBindTrig = nullptr;
using GetActionBindButtonFn = int (*)(DusklightActionBind, u32);
GetActionBindButtonFn s_getActionBindButton = nullptr;

bool midna_action_triggered() {
    return controller_compatibility() == ControllerCompatibility::FollowDusklight &&
        s_getActionBindTrig != nullptr &&
        s_getActionBindTrig(DusklightActionBind::CallMidna, PAD_1);
}

u32 midna_game_button_mask() {
    if (controller_compatibility() != ControllerCompatibility::FollowDusklight ||
        s_getActionBindButton == nullptr)
    {
        return 0;
    }

    const int nativeButton =
        s_getActionBindButton(DusklightActionBind::CallMidna, PAD_1);
    if (nativeButton == PAD_NATIVE_BUTTON_INVALID) {
        return 0;
    }

    u32 mappingCount = 0;
    PADButtonMapping* mappings = PADGetButtonMappings(PAD_1, &mappingCount);
    u32 gameButtonMask = 0;
    for (u32 index = 0; mappings != nullptr && index < mappingCount; ++index) {
        if (mappings[index].nativeButton == nativeButton) {
            gameButtonMask |= mappings[index].padButton;
        }
    }
    return gameButtonMask;
}

bool midna_uses_dpad_down() {
    return controller_compatibility() == ControllerCompatibility::FollowDusklight &&
        (midna_game_button_mask() & PAD_BUTTON_DOWN) != 0;
}

void resolve_action_binding_functions() {
    void* address = nullptr;
    HookSymbolFlags flags = {};
    const ModResult result = svc_hook->resolve(
        mod_ctx, "dusk::getActionBindTrig", &address, &flags);
    if (result == MOD_OK && address != nullptr) {
        s_getActionBindTrig = reinterpret_cast<GetActionBindTrigFn>(address);
    } else {
        s_getActionBindTrig = nullptr;
        svc_log->warn(mod_ctx,
            "Unable to read Dusklight's Call Midna trigger; D-pad Up remains available");
    }

    address = nullptr;
    const ModResult buttonResult = svc_hook->resolve(
        mod_ctx, "dusk::getActionBindButton", &address, &flags);
    if (buttonResult == MOD_OK && address != nullptr) {
        s_getActionBindButton = reinterpret_cast<GetActionBindButtonFn>(address);
    } else {
        s_getActionBindButton = nullptr;
        svc_log->warn(mod_ctx,
            "Unable to isolate Dusklight's Call Midna button from its normal game action");
    }
}

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
void position_cursor_outside_frame(dSelect_cursor_c*, J2DPane*,
    f32 = 7.0f, f32 = 5.0f);
void move_file_select_pane_center(J2DPane*, f32, f32);
void apply_file_select_row_texture(J2DPicture*, ResTIMG const*);
void set_copy_file_info_visible(dFile_select_c*, std::size_t, bool);

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

const char* text_box_string(const J2DTextBox* text) {
    if (text == nullptr) {
        return nullptr;
    }
#if TARGET_PC
    return text->getStringPtr().buffer;
#else
    return text->getStringPtr();
#endif
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

ResTIMG const* styled_face_button_texture(const char letter) {
    if (button_style() == ButtonStyle::BlackPro) {
        const int index = letter == 'A' ? 0 : letter == 'B' ? 1 :
            letter == 'X' ? 2 : letter == 'Y' ? 3 : -1;
        if (index >= 0) {
            if (ResTIMG const* texture = resource_texture(s_blackProFaceButtonResources[index])) {
                return texture;
            }
        }
    }

    switch (letter) {
    case 'A':
        if (ResTIMG const* texture = resource_texture(s_faceButtonAResource)) return texture;
        return archive_texture("wiiu_a.bti");
    case 'B':
        if (ResTIMG const* texture = resource_texture(s_faceButtonBResource)) return texture;
        return archive_texture("wiiu_b.bti");
    case 'X': return archive_texture("wiiu_x.bti");
    case 'Y': return archive_texture("wiiu_y.bti");
    default: return nullptr;
    }
}

ResTIMG const* styled_r_button_texture() {
    if (button_style() == ButtonStyle::BlackPro) {
        if (ResTIMG const* texture = resource_texture(s_blackProShoulderButtonResource)) {
            return texture;
        }
    }
    return archive_texture("wiiu_r.bti");
}

ResTIMG const* styled_zl_button_texture() {
    if (button_style() == ButtonStyle::BlackPro) {
        if (ResTIMG const* texture = resource_texture(s_blackProZlShoulderButtonResource)) {
            return texture;
        }
    }
    return resource_texture(s_zlShoulderButtonResource);
}

ResTIMG const* styled_blank_face_button_texture() {
    if (button_style() == ButtonStyle::BlackPro) {
        if (ResTIMG const* texture = resource_texture(s_blackProBlankFaceButtonResource)) {
            return texture;
        }
    }
    return archive_texture("tt_zelda_button_ab_maru.bti");
}

ResTIMG const* menu_face_button_texture(const bool nativeAAction) {
    ResTIMG const* buttonA = styled_face_button_texture('A');
    ResTIMG const* buttonB = styled_face_button_texture('B');

    switch (button_layout()) {
    case ButtonLayout::Nintendo:
        return nativeAAction ? buttonA : buttonB;
    case ButtonLayout::Xbox:
        // The actions retain their original positions. Only the printed
        // letters change to match the Xbox south/east face-button layout.
        return nativeAAction ? buttonB : buttonA;
    case ButtonLayout::Universal:
        return styled_blank_face_button_texture();
    }
    return nativeAAction ? buttonA : buttonB;
}

ResTIMG const* item_assignment_button_texture(const bool nativeXButton) {
    switch (button_layout()) {
    case ButtonLayout::Nintendo:
        return styled_face_button_texture(nativeXButton ? 'X' : 'Y');
    case ButtonLayout::Xbox:
        return styled_face_button_texture(nativeXButton ? 'Y' : 'X');
    case ButtonLayout::Universal:
        return styled_blank_face_button_texture();
    }
    return styled_face_button_texture(nativeXButton ? 'X' : 'Y');
}

void set_neutral_picture_colors(J2DPicture* picture) {
    if (picture == nullptr) {
        return;
    }
    picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
}

void replace_item_assignment_buttons(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    ResTIMG const* xBase = archive_texture("tt_zelda_button_x_base.bti");
    ResTIMG const* yBase = archive_texture("tt_zelda_button_y_base.bti");
    ResTIMG const* xText = archive_texture("tt_zelda_button_x_text.bti");
    ResTIMG const* yText = archive_texture("tt_zelda_button_y_text.bti");
    ResTIMG const* xFull = archive_texture("WiiU_zelda_button_x.bti");
    ResTIMG const* yFull = archive_texture("WiiU_zelda_button_y.bti");

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr) {
        ResTIMG const* texture = picture->getTexture(0)->getTexInfo();
        if (texture == xBase || texture == yBase) {
            picture->hide();
        } else if (texture == xText || texture == xFull) {
            if (ResTIMG const* replacement = item_assignment_button_texture(true)) {
                picture->changeTexture(replacement, 0);
                set_neutral_picture_colors(picture);
                picture->show();
            }
        } else if (texture == yText || texture == yFull) {
            if (ResTIMG const* replacement = item_assignment_button_texture(false)) {
                picture->changeTexture(replacement, 0);
                set_neutral_picture_colors(picture);
                picture->show();
            }
        }
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        replace_item_assignment_buttons(child);
    }
}

void apply_item_explain_button_layout(dMenu_ItemExplain_c* menu) {
    if (menu == nullptr) {
        return;
    }

    replace_item_assignment_buttons(menu->mpInfoScreen);

    if (menu->mpInfoString == nullptr || menu->mpInfoString->mpOutFont == nullptr) {
        return;
    }

    // Item descriptions render inline X/Y controls through their own out-font
    // instance. Types 5 and 6 are the native X and Y glyphs respectively.
    COutFont_c* outFont = menu->mpInfoString->mpOutFont;
    const struct {
        int type;
        bool nativeXButton;
    } inlineButtons[] = {
        {5, true},
        {6, false},
    };
    for (const auto& button : inlineButtons) {
        J2DPicture* picture = outFont->mpPane[button.type];
        ResTIMG const* replacement = item_assignment_button_texture(button.nativeXButton);
        if (picture != nullptr && replacement != nullptr) {
            picture->changeTexture(replacement, 0);
            set_neutral_picture_colors(picture);
        }
    }

    // Hawkeye's stock description labels the bow-combination control as R.
    // In this HUD, R is the third item slot and ZL is the combination control,
    // so replace only this description's inline R glyph. Other item-help R
    // glyphs must remain R because they can legitimately refer to that slot.
    if (menu->field_0xe1 == dItemNo_HAWK_EYE_e) {
        constexpr int kInlineRType = 4;
        J2DPicture* picture = outFont->mpPane[kInlineRType];
        if (ResTIMG const* replacement = styled_zl_button_texture();
            picture != nullptr && replacement != nullptr) {
            picture->changeTexture(replacement, 0);
            set_neutral_picture_colors(picture);
        }
    }
}

void update_menu_face_button(J2DScreen* screen, const u64 tag,
    const bool nativeAAction) {
    auto* picture = screen != nullptr ?
        as_picture(screen->search(tag)) : nullptr;
    ResTIMG const* texture = menu_face_button_texture(nativeAAction);
    if (picture != nullptr && texture != nullptr) {
        picture->changeTexture(texture, 0);
    }
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

void set_menu_face_button_texture(J2DScreen* screen, const u64 pictureTag,
    ResTIMG const* texture) {
    if (screen == nullptr || texture == nullptr) {
        return;
    }

    J2DPicture* picture = as_picture(screen->search(pictureTag));
    if (picture == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> originalBounds = picture->getBounds();
    picture->changeTexture(texture, 0);
    picture->resize(originalBounds.getWidth(), originalBounds.getHeight());
    picture->move(originalBounds.i.x, originalBounds.i.y);
    picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
}

// Collection menu -----------------------------------------------------------

ResTIMG const* collect_archive_texture(const char* textureName) {
    JKRArchive* archive = dComIfGp_getCollectResArchive();
    return archive != nullptr && textureName != nullptr ? static_cast<ResTIMG const*>(
        archive->getResource('TIMG', textureName)) : nullptr;
}

f32 collection_right_edge_offset() {
#if TARGET_PC
    // Dusklight's desktop Collection implementation already translates the
    // icon screen by -safeMinX on every widescreen refresh. Adding the same
    // safe-edge delta here a second time pushes the A/B discs beyond the
    // viewport (most visibly on Windows).
    return 0.0f;
#else
    return mDoGph_gInf_c::getMaxXF() - 608.0f;
#endif
}

f32 collection_left_edge() {
#if TARGET_PC
    return mDoGph_gInf_c::getSafeMinXF();
#else
    return mDoGph_gInf_c::getMinXF();
#endif
}

void remember_collect_prompt_width(J2DScreen* screen, const f32 rightOffset) {
    if (screen == nullptr) {
        return;
    }
    for (CollectPromptWidthState& state : s_collectPromptWidthStates) {
        if (state.screen == screen) {
            state.rightOffset = rightOffset;
            return;
        }
    }
    CollectPromptWidthState& state = s_collectPromptWidthStates[
        s_nextCollectPromptWidthState++ % s_collectPromptWidthStates.size()];
    state.screen = screen;
    state.rightOffset = rightOffset;
}

void refresh_collect_prompt_width(J2DScreen* screen) {
    if (screen == nullptr) {
        return;
    }
    for (CollectPromptWidthState& state : s_collectPromptWidthStates) {
        if (state.screen != screen) {
            continue;
        }
        const f32 nextOffset = collection_right_edge_offset();
        const f32 delta = nextOffset - state.rightOffset;
        if (std::fabs(delta) > 0.01f) {
            for (const u64 tag : {MULTI_CHAR('abtn_n'), MULTI_CHAR('a_text_n'),
                     MULTI_CHAR('bbtn_n'), MULTI_CHAR('b_text_n')}) {
                if (J2DPane* pane = screen->search(tag)) {
                    pane->add(delta, 0.0f);
                }
            }
            state.rightOffset = nextOffset;
        }
        return;
    }
}

void simplify_collect_button_decoration(J2DScreen* screen) {
    if (screen == nullptr) {
        return;
    }

    // TPHD retains one shared gold flourish woven behind the A/B diagonal.
    // Keep only the primary wire layer, then reduce and position the group so
    // it supports both discs instead of forming the oversized GameCube halo.
    if (J2DPane* shadow = screen->search(MULTI_CHAR('wire_s'))) {
        shadow->hide();
    }
    if (J2DPane* shadow = screen->search(MULTI_CHAR('wire_s1'))) {
        shadow->hide();
    }
    if (J2DPane* flourish = screen->search(MULTI_CHAR('uzu_n'))) {
        flourish->hide();
    }

    // Anchor to Dusklight's current safe right edge. Unlike a fixed 4:3/16:9
    // branch, this follows arbitrary desktop window sizes and platform safe
    // areas continuously.
    const f32 widescreenOffset = collection_right_edge_offset();

    // Keep both neutral discs compact and place the two rows in the upper-right
    // margin, matching TPHD's right-aligned prompt cluster.
    if (J2DPane* buttonA = screen->search(MULTI_CHAR('abtn_n'))) {
        buttonA->add(widescreenOffset, -6.0f);
        buttonA->scale(0.88f, 0.88f);
    }

    if (J2DPane* actionText = screen->search(MULTI_CHAR('a_text_n'))) {
        // Lift the A action caption slightly so Confirm shares the A disc's
        // visual baseline rather than sitting below it. Move the containing
        // pane so all five native outline/shadow text layers stay registered.
        actionText->add(widescreenOffset - 10.0f, -12.0f);
    }

    // The original B prompt is authored considerably smaller than A. Match
    // their apparent disc sizes while retaining TPHD's tighter second row.
    if (J2DPane* buttonB = screen->search(MULTI_CHAR('bbtn_n'))) {
        buttonB->add(widescreenOffset + 2.0f, -12.0f);
        buttonB->scale(1.25f, 1.25f);
    }
    if (J2DPane* backText = screen->search(MULTI_CHAR('b_text_n'))) {
        backText->add(widescreenOffset - 16.0f, -12.0f);
    }
    remember_collect_prompt_width(screen, widescreenOffset);
}

void replace_collect_background(J2DScreen* screen) {
    ResTIMG const* replacement = resource_texture(s_collectBackgroundResource);
    if (screen == nullptr || replacement == nullptr) {
        return;
    }

    // w_bas05 is the large tiled window-base picture that supplies the brick
    // field behind the Collection UI. Its tag is stable even when an external
    // texture pack replaces the original 128x128 source texture.
    J2DPicture* background = as_picture(screen->search(MULTI_CHAR('w_bas05')));
    if (background == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> bounds = background->getBounds();
    background->changeTexture(replacement, 0);
    background->resize(bounds.getWidth(), bounds.getHeight());
    background->move(bounds.i.x, bounds.i.y);
    background->setTexCoord(background->getTexture(0), BIND15, MIRROR0, false);
    background->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    background->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    background->setAlpha(255);
    background->show();
}

void hide_collect_decoration_texture(J2DPane* pane, ResTIMG const* decorationTexture) {
    if (pane == nullptr || decorationTexture == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr &&
        picture->getTexture(0)->getTexInfo() == decorationTexture) {
        picture->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_collect_decoration_texture(child, decorationTexture);
    }
}

void remove_collect_ornaments(J2DScreen* screen) {
    if (screen == nullptr) {
        return;
    }

    // These are the standalone curl, corner, crown, and engraved surround
    // images. The plainer button borders, item cells, and heart components use
    // separate textures and remain visible.
    constexpr const char* ornamentTextures[] = {
        "tt_gold_uzu_long2.bti",
        "tt_horiwaku_lu.bti",
        "tt_horiwaku_side_l_rr.bti",
        "tt_horiwaku_top_rr.bti",
        "tt_kazari_2nd_okan_64.bti",
        "tt_kazari_kani_00.bti",
        "zelda_kazari_sample3_mix_4ch_4_1.bti",
    };
    for (const char* textureName : ornamentTextures) {
        hide_collect_decoration_texture(screen, collect_archive_texture(textureName));
    }
}

void find_largest_picture(J2DPane* pane, J2DPicture*& largest, f32& largestArea) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane)) {
        const JGeometry::TBox2<f32> bounds = picture->getBounds();
        const f32 area = bounds.getWidth() * bounds.getHeight();
        if (area > largestArea) {
            largest = picture;
            largestArea = area;
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        find_largest_picture(child, largest, largestArea);
    }
}

void hide_other_pictures(J2DPane* pane, J2DPicture* keep) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane); picture != nullptr && picture != keep) {
        picture->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_other_pictures(child, keep);
    }
}

void hide_other_pictures(J2DPane* pane, J2DPicture* keepA,
    J2DPicture* keepB) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture != keepA && picture != keepB) {
        picture->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_other_pictures(child, keepA, keepB);
    }
}

void hide_collect_stone_rails(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture != s_collectTopRule &&
        picture != s_collectBottomRule) {
        const auto bounds = picture->getBounds();
        if (bounds.getWidth() > 500.0f && bounds.getHeight() < 24.0f &&
            (bounds.i.y < 80.0f || bounds.i.y > 350.0f)) {
            picture->hide();
        }
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_collect_stone_rails(child);
    }
}

// The remaining GameCube rails are nested below the Collection screen's
// transformed panes, so their local bounds do not identify them reliably.
// Log only wide, shallow panes near the screen edges once, using global bounds
// and tags, so we can remove the exact legacy art without affecting the HD
// background.
void log_collect_rail_candidates(J2DPane* pane) {
    if (pane == nullptr || s_collectRailDiagnosticsLogged) {
        return;
    }

    const auto global = pane->getGlbBounds();
    const f32 width = global.getWidth();
    const f32 height = global.getHeight();
    if (width > 300.0f && height > 1.0f && height < 120.0f &&
        (global.i.y < 100.0f || global.i.y > 330.0f)) {
        char message[192];
        std::snprintf(message, sizeof(message),
            "collect rail candidate tag=%08llx local=(%.1f,%.1f %.1fx%.1f) global=(%.1f,%.1f %.1fx%.1f) type=%u",
            static_cast<unsigned long long>(pane->mInfoTag), pane->getBounds().i.x,
            pane->getBounds().i.y, pane->getBounds().getWidth(),
            pane->getBounds().getHeight(), global.i.x, global.i.y, width, height,
            pane->getTypeID());
        svc_log->info(mod_ctx, message);
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        log_collect_rail_candidates(child);
    }
}

void style_collect_edge_rules(J2DScreen* screen) {
    ResTIMG const* texture = resource_texture(s_collectMenuButtonResource);
    if (screen == nullptr || texture == nullptr) {
        return;
    }

    const f32 width = screen->getWidth();
    auto ensureRule = [&](J2DPicture*& rule, const u64 tag, const f32 y) {
        if (rule == nullptr) {
            rule = JKR_NEW J2DPicture(tag,
                JGeometry::TBox2<f32>(0.0f, y, width, y + 1.0f),
                texture, nullptr);
            rule->setTexCoord(rule->getTexture(0), BIND15, MIRROR0, false);
            // Rules belong behind the footer panels, as in TPHD. Appending
            // would draw the lower rule through Save Game and Options.
            screen->insertChild(screen->getFirstChildPane(), rule);
        }
        rule->resize(width, 1.0f);
        rule->move(0.0f, y);
        rule->setBlackWhite(JUtility::TColor(190, 186, 150, 255),
            JUtility::TColor(190, 186, 150, 255));
        rule->setAlpha(210);
        rule->show();
    };

    // TPHD uses a narrow paired rule at each screen edge, not a single
    // GameCube stone band. Keep the pair behind all interactive panels.
    ensureRule(s_collectTopRule, MULTI_CHAR('hd_ctr1'), 20.0f);
    ensureRule(s_collectTopRuleInner, MULTI_CHAR('hd_ctr3'), 23.0f);
    ensureRule(s_collectBottomRule, MULTI_CHAR('hd_ctr2'), 420.0f);
    ensureRule(s_collectBottomRuleInner, MULTI_CHAR('hd_ctr4'), 423.0f);

    // These are the three full-width brick/stone bands in the original
    // Collection screen. Hide them by their stable tags rather than hiding
    // their parent, which also carries part of the HD replacement background.
    for (const u64 tag : {MULTI_CHAR('w_bas02'), MULTI_CHAR('w_sen01'), MULTI_CHAR('w_sen02'),
             MULTI_CHAR('w_sen03'), MULTI_CHAR('w_btn_bg'),
             MULTI_CHAR('w_btn_b1'), MULTI_CHAR('w_btn_kg')}) {
        if (J2DPane* rail = screen->search(tag)) {
            // Visibility is animated back on after Collection queues its
            // screen. Collapse the authored bounds as well, which the
            // animation does not reconstruct.
            rail->hide();
            rail->resize(1.0f, 1.0f);
            rail->move(-1000.0f, -1000.0f);
        }
    }
    hide_collect_stone_rails(screen);
}

void style_collect_menu_panel(J2DScreen* screen, const u64 groupTag,
    const u64 frameTag, J2DPicture*& cachedFrame) {
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    J2DPane* group = screen != nullptr ? screen->search(groupTag) : nullptr;
    if (group == nullptr || frameTexture == nullptr) {
        return;
    }

    J2DPicture* frame = cachedFrame;
    if (frame == nullptr) {
        J2DPicture* originalFrame = nullptr;
        f32 largestArea = 0.0f;
        find_largest_picture(group, originalFrame, largestArea);
        const auto originalBounds = originalFrame != nullptr ?
            originalFrame->getBounds() : group->getBounds();
        const f32 centerX = (originalBounds.i.x + originalBounds.f.x) * 0.5f;
        const f32 centerY = (originalBounds.i.y + originalBounds.f.y) * 0.5f;
        frame = JKR_NEW J2DPicture(frameTag,
            JGeometry::TBox2<f32>(centerX, centerY, centerX + 1.0f,
                centerY + 1.0f), frameTexture, nullptr);
        frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
        group->insertChild(group->getFirstChildPane(), frame);
        cachedFrame = frame;
    }
    if (frame == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> bounds = frame->getBounds();
    constexpr f32 targetWidth = 156.0f;
    constexpr f32 targetHeight = 28.0f;
    const f32 centerX = (bounds.i.x + bounds.f.x) * 0.5f;
    const f32 centerY = (bounds.i.y + bounds.f.y) * 0.5f;
    hide_other_pictures(group, frame);
    frame->changeTexture(frameTexture, 0);
    frame->resize(targetWidth, targetHeight);
    frame->move(centerX - targetWidth * 0.5f, centerY - targetHeight * 0.5f);
    frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
    frame->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    frame->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    frame->setAlpha(255);
    frame->show();
}

void configure_hd_picture(J2DPicture* picture);

J2DPane* collect_title_container(J2DScreen* screen) {
    J2DPane* title = screen != nullptr ? screen->search(MULTI_CHAR('t_t00')) : nullptr;
    J2DPane* container = title;
    while (container != nullptr && container->getParentPane() != nullptr) {
        J2DPane* parent = container->getParentPane();
        container = parent;
        if (parent == screen || parent->getWidth() > 220.0f ||
            parent->getHeight() > 40.0f) {
            break;
        }
    }
    return container;
}

void anchor_collect_title_panel() {
    if (s_collectTitleFrame == nullptr) {
        return;
    }
    const auto local = s_collectTitleFrame->getBounds();
#if defined(_WIN32)
    // Windows rewrites the Collection parent transforms immediately before
    // drawing. Compute the current left edge through the complete pane chain;
    // getGlbBounds() can still contain the preceding frame's cached origin at
    // this point, which is what caused the title to flash on entry/return.
    CPaneMgr paneMgr;
    Mtx matrix;
    const Vec globalLeft = paneMgr.getGlobalVtx(
        s_collectTitleFrame, &matrix, 0, false, 0);
    const Vec globalRight = paneMgr.getGlobalVtx(
        s_collectTitleFrame, &matrix, 1, false, 0);
    const f32 localWidth = local.getWidth();
    const f32 globalWidth = std::fabs(globalRight.x - globalLeft.x);
    const f32 scaleX = localWidth > 0.0f && globalWidth > 0.0f ?
        globalWidth / localWidth : 1.0f;
    const f32 targetLeft = collection_left_edge() + 12.0f;
    const f32 delta = targetLeft - globalLeft.x;
#else
    const auto global = s_collectTitleFrame->getGlbBounds();
    const f32 localWidth = local.getWidth();
    const f32 globalWidth = global.getWidth();
    const f32 scaleX = localWidth > 0.0f && globalWidth > 0.0f ?
        globalWidth / localWidth : 1.0f;
    const f32 targetLeft = collection_left_edge() + 12.0f;
    const f32 delta = targetLeft - global.i.x;
#endif
    if (std::fabs(delta) > 0.25f) {
        s_collectTitleFrame->add(delta / scaleX, 0.0f);
    }
    if (s_collectTitleLabel != nullptr) {
        const auto frameBounds = s_collectTitleFrame->getBounds();
        s_collectTitleLabel->resize(frameBounds.getWidth(), frameBounds.getHeight());
        s_collectTitleLabel->move(frameBounds.i.x, frameBounds.i.y);
    }
}

void style_collect_title_panel(J2DScreen* screen, const bool position) {
    J2DPane* container = collect_title_container(screen);
    ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource);
    if (container == nullptr || frameTexture == nullptr) {
        return;
    }

    J2DPicture* frame = s_collectTitleFrame;
    if (frame == nullptr) {
        J2DPicture* originalFrame = nullptr;
        f32 largestArea = 0.0f;
        find_largest_picture(container, originalFrame, largestArea);
        if (originalFrame == nullptr) {
            return;
        }
        const auto originalBounds = originalFrame->getBounds();
        const f32 centerX = (originalBounds.i.x + originalBounds.f.x) * 0.5f;
        const f32 centerY = (originalBounds.i.y + originalBounds.f.y) * 0.5f;
        frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_ctfr'),
            JGeometry::TBox2<f32>(centerX, centerY, centerX + 1.0f,
                centerY + 1.0f), frameTexture, nullptr);
        frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
        container->insertChild(container->getFirstChildPane(), frame);
        s_collectTitleFrame = frame;
    }
    if (frame == nullptr) {
        return;
    }

    constexpr f32 targetWidth = 235.0f;
    constexpr f32 targetHeight = 36.0f;
    const auto bounds = frame->getBounds();
    const f32 centerX = (bounds.i.x + bounds.f.x) * 0.5f;
    const f32 centerY = (bounds.i.y + bounds.f.y) * 0.5f;
    hide_other_pictures(container, frame);
    frame->changeTexture(frameTexture, 0);
    frame->resize(targetWidth, targetHeight);
    frame->move(centerX - targetWidth * 0.5f, centerY - targetHeight * 0.5f);
    configure_hd_picture(frame);
    frame->setBlackWhite(JUtility::TColor(38, 40, 20, 255),
        JUtility::TColor(224, 224, 178, 255));

    if (!position) {
        if (s_collectTitleLabel != nullptr) {
            const auto frameBounds = frame->getBounds();
            s_collectTitleLabel->resize(frameBounds.getWidth(),
                frameBounds.getHeight());
            s_collectTitleLabel->move(frameBounds.i.x, frameBounds.i.y);
            s_collectTitleLabel->setFontSize(20.0f, 20.0f);
            s_collectTitleLabel->show();
        }
        for (const u64 tag : {MULTI_CHAR('t_t00'), MULTI_CHAR('f_t00')}) {
            if (J2DPane* nativeTitle = screen->search(tag)) {
                nativeTitle->hide();
            }
        }
        anchor_collect_title_panel();
        return;
    }

    container->add(-175.0f, -2.0f);
    frame->add(-138.0f, -5.0f);
    JUTFont* titleFont = nullptr;
    for (const u64 tag : {MULTI_CHAR('t_t00'), MULTI_CHAR('f_t00')}) {
        auto* text = static_cast<J2DTextBox*>(screen->search(tag));
        if (text == nullptr) {
            continue;
        }
        if (titleFont == nullptr && text->getFont() != nullptr) {
            titleFont = text->getFont();
        }
        text->hide();
    }

    const auto frameBounds = frame->getBounds();
    if (s_collectTitleLabel == nullptr) {
        s_collectTitleLabel = JKR_NEW J2DTextBox(MULTI_CHAR('hd_ctxt'),
            frameBounds, nullptr, "Collection", 32, HBIND_CENTER,
            VBIND_CENTER);
        s_collectTitleLabel->setFont(titleFont);
        container->appendChild(s_collectTitleLabel);
    }
    s_collectTitleLabel->resize(frameBounds.getWidth(), frameBounds.getHeight());
    s_collectTitleLabel->move(frameBounds.i.x, frameBounds.i.y);
    s_collectTitleLabel->setFontSize(20.0f, 20.0f);
    s_collectTitleLabel->setCharSpace(0.0f);
    s_collectTitleLabel->setFontColor(JUtility::TColor(245, 245, 238, 255),
        JUtility::TColor(255, 255, 255, 255));
    s_collectTitleLabel->show();
    anchor_collect_title_panel();
}

void refresh_collect_menu_frames(dMenu_Collect2D_c* menu) {
    if (menu == nullptr || menu->mpScreen == nullptr) {
        return;
    }
    style_collect_title_panel(menu->mpScreen, false);
    style_collect_menu_panel(menu->mpScreen, MULTI_CHAR('sa_tex_n'),
        MULTI_CHAR('hd_csav'), s_collectSaveFrame);
    style_collect_menu_panel(menu->mpScreen, MULTI_CHAR('op_tex_n'),
        MULTI_CHAR('hd_copt'), s_collectOptionsFrame);
    if (J2DPane* footer = menu->mpScreen->search(MULTI_CHAR('sa_op_n'))) {
        hide_other_pictures(footer, s_collectSaveFrame, s_collectOptionsFrame);
    }
    style_collect_edge_rules(menu->mpScreen);
}

void position_collect_footer_cursor(dMenu_Collect2D_c* menu) {
    if (menu == nullptr || menu->mpDrawCursor == nullptr ||
        menu->mpDrawCursor->mpScreen == nullptr ||
        s_collectSaveFrame == nullptr || s_collectOptionsFrame == nullptr) {
        return;
    }

    // Collection exposes an exact footer row. Using it avoids stale global
    // bounds from the transformed GameCube layout, which made the prior
    // proximity test reject both buttons even while visibly selected.
    if (menu->getCursorY() != 5) {
        return;
    }
    J2DPicture* target = menu->getCursorX() == 0 ? s_collectSaveFrame :
        menu->getCursorX() == 1 ? s_collectOptionsFrame : nullptr;
    if (target == nullptr) {
        return;
    }
    if (!s_collectCursorDiagnosticsLogged) {
        char message[96];
        std::snprintf(message, sizeof(message),
            "collect footer cursor selected x=%u target=%s",
            static_cast<unsigned>(menu->getCursorX()),
            target == s_collectOptionsFrame ? "options" : "save");
        svc_log->info(mod_ctx, message);
        s_collectCursorDiagnosticsLogged = true;
    }
#if defined(_WIN32)
    // The Windows desktop layout scales the footer's parent rather than the
    // frame itself. dSelect_cursor_c::setPos(..., target, true) only sees the
    // frame's local scale, so it places the four corners using the much wider
    // unscaled bounds. Drive the final corner centers from rendered global
    // bounds instead; this is the last pass before the cursor is drawn.
    const auto& bounds = target->getGlbBounds();
    // J2DPane::getGlbBounds() supplies the correct rendered dimensions here,
    // but its cached origin does not include the footer's final PC parent
    // transforms. Use the same recursive center calculation as Collection's
    // native cursorPosSet() so Save Game and Options each inherit their own
    // complete transform chain.
    CPaneMgr paneMgr;
    const Vec center = paneMgr.getGlobalVtxCenter(target, false, 0);
    const f32 halfWidth = bounds.getWidth() * 0.5f + 5.0f;
    const f32 halfHeight = bounds.getHeight() * 0.5f;
    menu->mpDrawCursor->setPos(center.x, center.y, nullptr, false);
    constexpr u64 cornerTags[] = {
        MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null'),
    };
    for (int corner = 0; corner < 4; ++corner) {
        if (J2DPane* pane = menu->mpDrawCursor->mpScreen->search(
                cornerTags[corner])) {
            menu->mpDrawCursor->moveCenter(pane,
                corner < 2 ? -halfWidth : halfWidth,
                corner % 2 == 0 ? -halfHeight : halfHeight);
        }
    }
#else
    position_cursor_outside_frame(menu->mpDrawCursor, target, 2.0f, 3.0f);
#endif
}

void apply_collect_menu_typography(dMenu_Collect2D_c* menu) {
    if (menu == nullptr || menu->mpScreen == nullptr || menu->getIconScreen() == nullptr) {
        return;
    }

    J2DScreen* mainScreen = menu->mpScreen;
    J2DScreen* iconScreen = menu->getIconScreen();
    auto* prompt = static_cast<J2DTextBox*>(iconScreen->search(MULTI_CHAR('atext1_5')));
    if (prompt == nullptr || prompt->getFont() == nullptr) {
        return;
    }

    // Collection was authored with the larger ruby/title face. Reuse the
    // action-prompt font so it matches Equip and Back while retaining the
    // title's authored size and centering.
    for (const u64 tag : {MULTI_CHAR('t_t00'), MULTI_CHAR('f_t00')}) {
        auto* title = static_cast<J2DTextBox*>(mainScreen->search(tag));
        if (title != nullptr) {
            title->setFont(prompt->getFont());
        }
    }

    J2DTextBox::TFontSize promptSize;
    prompt->getFontSize(promptSize);
    constexpr u64 saveTextTags[] = {
        MULTI_CHAR('sav_0'), MULTI_CHAR('sav_1'), MULTI_CHAR('sav_2'),
        MULTI_CHAR('f_sav_0'), MULTI_CHAR('f_sav_1'), MULTI_CHAR('f_sav_2'),
    };
    constexpr u64 optionTextTags[] = {
        MULTI_CHAR('opt_0'), MULTI_CHAR('opt_1'), MULTI_CHAR('opt_2'),
        MULTI_CHAR('f_opt_0'), MULTI_CHAR('f_opt_1'), MULTI_CHAR('f_opt_2'),
    };
    for (const u64 tag : saveTextTags) {
        auto* text = static_cast<J2DTextBox*>(mainScreen->search(tag));
        if (text != nullptr) {
            text->hide();
        }
    }
    for (const u64 tag : optionTextTags) {
        auto* text = static_cast<J2DTextBox*>(mainScreen->search(tag));
        if (text != nullptr) {
            text->hide();
        }
    }

    // The GameCube layout stacks three copies of each footer label to create
    // its broad glow. TPHD uses one clean, compact label inside each framed
    // button, so retain only the primary localized text pane.
    auto* saveLabel = static_cast<J2DTextBox*>(
        mainScreen->search(MULTI_CHAR('f_sav_0')));
    auto* optionsLabel = static_cast<J2DTextBox*>(
        mainScreen->search(MULTI_CHAR('f_opt_0')));
    if (saveLabel != nullptr) {
        saveLabel->setFont(prompt->getFont());
        saveLabel->setFontSize(15.0f, 15.0f);
        saveLabel->setFontColor(JUtility::TColor(242, 242, 237, 255),
            JUtility::TColor(255, 255, 255, 255));
        saveLabel->setString(0x20, "Save Game");
        // Keep the label centred in the visible HD frame.
        saveLabel->add(0.5f, 5.5f);
        saveLabel->show();
    }
    if (optionsLabel != nullptr) {
        optionsLabel->setFont(prompt->getFont());
        optionsLabel->setFontSize(15.0f, 15.0f);
        optionsLabel->setFontColor(JUtility::TColor(242, 242, 237, 255),
            JUtility::TColor(255, 255, 255, 255));
        // Keep the label centred in the visible HD frame.
        optionsLabel->add(2.5f, 5.5f);
        optionsLabel->show();
    }

    remove_collect_ornaments(mainScreen);
    style_collect_title_panel(mainScreen, true);

    if (J2DPane* save = mainScreen->search(MULTI_CHAR('sa_tex_n'))) {
        save->add(-133.0f, 12.0f);
    }
    if (J2DPane* options = mainScreen->search(MULTI_CHAR('op_tex_n'))) {
        options->add(102.0f, 11.0f);
    }
    style_collect_menu_panel(mainScreen, MULTI_CHAR('sa_tex_n'),
        MULTI_CHAR('hd_csav'), s_collectSaveFrame);
    style_collect_menu_panel(mainScreen, MULTI_CHAR('op_tex_n'),
        MULTI_CHAR('hd_copt'), s_collectOptionsFrame);
    if (s_collectSaveFrame != nullptr) {
        s_collectSaveFrame->add(112.0f, -5.0f);
    }
    if (s_collectOptionsFrame != nullptr) {
        s_collectOptionsFrame->add(-115.0f, -5.0f);
    }
    if (J2DPane* footer = mainScreen->search(MULTI_CHAR('sa_op_n'))) {
        hide_other_pictures(footer, s_collectSaveFrame, s_collectOptionsFrame);
    }
    style_collect_edge_rules(mainScreen);
}

void configure_hd_picture(J2DPicture* picture);

// Options menu --------------------------------------------------------------

void hide_option_native_art(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }

    if (as_picture(pane) != nullptr || pane->getTypeID() == 17 ||
        pane->getTypeID() == 19) {
        pane->hide();
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        hide_option_native_art(child);
    }
}

void configure_hd_picture(J2DPicture* picture) {
    if (picture == nullptr) {
        return;
    }
    picture->setTexCoord(picture->getTexture(0), BIND15, MIRROR0, false);
    picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    picture->setAlpha(255);
    picture->show();
}

// Dungeon map ---------------------------------------------------------------

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
                resource_texture(s_collectMenuButtonResource)) {
            auto* frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_ddfr'),
                JGeometry::TBox2<f32>(18.0f, 14.0f, 250.0f, 50.0f),
                frameTexture, nullptr);
            configure_hd_picture(frame);
            titleGroup->appendChild(frame);
        }
        auto* title = JKR_NEW J2DTextBox(MULTI_CHAR('hd_ddtx'),
            JGeometry::TBox2<f32>(30.0f, 15.0f, 238.0f, 49.0f), nullptr,
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
        constexpr f32 rowY[] = {10.0f, 28.0f};
        ResTIMG const* icons[] = {
            menu_face_button_texture(true),
            menu_face_button_texture(false),
        };
        for (std::size_t index = 0; index < 2; ++index) {
            auto* text = JKR_NEW J2DTextBox(textTags[index],
                JGeometry::TBox2<f32>(0.0f, rowY[index] - 11.0f,
                    132.0f, rowY[index] + 11.0f), nullptr, labels[index], 32,
                HBIND_RIGHT, VBIND_CENTER);
            text->setFont(mDoExt_getMesgFont());
            text->setFontSize(11.0f, 11.0f);
            text->setFontColor(JUtility::TColor(238, 238, 232, 255),
                JUtility::TColor(255, 255, 255, 255));
            promptGroup->appendChild(text);
            if (icons[index] != nullptr) {
                auto* icon = JKR_NEW J2DPicture(iconTags[index],
                    JGeometry::TBox2<f32>(138.0f, rowY[index] - 9.0f,
                        156.0f, rowY[index] + 9.0f), icons[index], nullptr);
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
        titleGroup->scale(mDoGph_gInf_c::hudAspectScaleDown, 1.0f);
    }
    if (promptGroup != nullptr) {
        update_menu_face_button(map->mButtonScreen, MULTI_CHAR('hd_daic'), true);
        update_menu_face_button(map->mButtonScreen, MULTI_CHAR('hd_dbic'), false);
        promptGroup->move(438.0f, 4.0f);
        promptGroup->scale(mDoGph_gInf_c::hudAspectScaleDown, 1.0f);
    }
}

// Field map -----------------------------------------------------------------

J2DTextBox* fmap_text_source(J2DScreen* screen, const u64* tags,
    std::size_t count) {
    if (screen == nullptr) {
        return nullptr;
    }
    for (std::size_t index = 0; index < count; ++index) {
        auto* text = static_cast<J2DTextBox*>(screen->search(tags[index]));
        const char* value = text_box_string(text);
        if (text != nullptr && value != nullptr && value[0] != '\0') {
            return text;
        }
    }
    return nullptr;
}

void apply_fmap_background(dMenu_Fmap2DBack_c* map) {
    if (map == nullptr || map->mpBackScreen == nullptr || map->mpBackTex == nullptr) {
        return;
    }
    ResTIMG const* background = resource_texture(s_fileSelectBackgroundResource);
    if (background == nullptr) {
        return;
    }

    // The field-map back screen contains only the original tiled stone wall.
    // Keep it out of the draw and reuse the already full-viewport back picture
    // for the circular TPHD backdrop. This naturally covers ultrawide output.
    map->mpBackScreen->hide();
    map->mpBackTex->changeTexture(background, 0);
    configure_hd_picture(map->mpBackTex);

    // The original field map was placed for the narrow GameCube frame. The
    // widened frame exposes more empty space on its right, so move the map's
    // own coordinate origin—not the frame—to restore its visual center.
    g_fmapHIO.mMapTopLeftPosX = 170.0f;
}

void add_fmap_top_overlay(dMenu_Fmap2DTop_c* map) {
    if (map == nullptr || map->mpTitleScreen == nullptr ||
        map->mpTitleScreen->search(MULTI_CHAR('hd_ftop')) != nullptr) {
        return;
    }

    constexpr u64 titleTags[] = {
        MULTI_CHAR('ffont00'), MULTI_CHAR('ffontl0'), MULTI_CHAR('ffontl1'),
        MULTI_CHAR('ffontl2'), MULTI_CHAR('ffontb0'), MULTI_CHAR('ffontb3'),
        MULTI_CHAR('ffontb4'),
    };
    constexpr u64 aTags[] = {
        MULTI_CHAR('font_at1'), MULTI_CHAR('font_at2'), MULTI_CHAR('font_at3'),
        MULTI_CHAR('font_at4'), MULTI_CHAR('font_at5'),
    };
    J2DTextBox* titleSource = fmap_text_source(map->mpTitleScreen,
        titleTags, std::size(titleTags));
    J2DTextBox* promptSource = fmap_text_source(map->mpTitleScreen,
        aTags, std::size(aTags));
    if (titleSource == nullptr || promptSource == nullptr) {
        return;
    }

    JKRHeap* previousHeap = map->mpHeap != nullptr ?
        mDoExt_setCurrentHeap(map->mpHeap) : nullptr;
    auto* group = JKR_NEW J2DPane(MULTI_CHAR('hd_ftop'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    map->mpTitleScreen->appendChild(group);

    if (ResTIMG const* frameTexture = resource_texture(s_collectMenuButtonResource)) {
        auto* frame = JKR_NEW J2DPicture(MULTI_CHAR('hd_ftfr'),
            JGeometry::TBox2<f32>(18.0f, 16.0f, 246.0f, 51.0f),
            frameTexture, nullptr);
        configure_hd_picture(frame);
        group->appendChild(frame);
    }
    auto* title = JKR_NEW J2DTextBox(MULTI_CHAR('hd_fttx'),
        JGeometry::TBox2<f32>(30.0f, 17.0f, 234.0f, 50.0f), nullptr,
        "", 64, HBIND_CENTER, VBIND_CENTER);
    title->setFont(titleSource->getFont());
    title->setFontSize(20.0f, 20.0f);
    title->setCharSpace(-0.5f);
    title->setFontColor(JUtility::TColor(242, 242, 235, 255),
        JUtility::TColor(255, 255, 255, 255));
    group->appendChild(title);

    // Use the native control pane only to locate and suppress the old prompt
    // art. The replacement lives on the title screen so it can be anchored to
    // the current widescreen-safe edge without inheriting native animation.
    constexpr f32 fontSizes[] = {8.5f, 10.0f, 11.0f};
    constexpr f32 iconSizes[] = {10.0f, 14.0f, 16.0f};
    constexpr f32 centers[] = {12.0f, 34.0f, 60.0f};
    constexpr u64 textTags[] = {
        MULTI_CHAR('hd_fpt0'), MULTI_CHAR('hd_fpt1'), MULTI_CHAR('hd_fpt2'),
    };
    constexpr u64 iconTags[] = {
        MULTI_CHAR('hd_fri0'), MULTI_CHAR('hd_fai1'), MULTI_CHAR('hd_fbi2'),
    };
    ResTIMG const* iconTextures[] = {
        styled_r_button_texture(), menu_face_button_texture(true),
        menu_face_button_texture(false),
    };
    J2DPane* controlAnchor = map->mpContPane->getPanePtr();
    if (controlAnchor == nullptr) {
        if (previousHeap != nullptr) {
            mDoExt_setCurrentHeap(previousHeap);
        }
        return;
    }
    for (J2DPane* child = controlAnchor->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        child->hide();
    }
    constexpr f32 promptWidth = 150.0f;
    constexpr f32 promptHeight = 76.0f;
    auto* promptGroup = JKR_NEW J2DPane(MULTI_CHAR('hd_fctl'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, promptWidth, promptHeight));
    group->appendChild(promptGroup);
    constexpr f32 railX = promptWidth - 8.0f;
    for (std::size_t index = 0; index < std::size(textTags); ++index) {
        auto* text = JKR_NEW J2DTextBox(textTags[index],
            JGeometry::TBox2<f32>(0.0f, centers[index] - 9.0f,
                railX - 12.0f, centers[index] + 9.0f), nullptr, "", 48,
            HBIND_RIGHT, VBIND_CENTER);
        text->setFont(promptSource->getFont());
        text->setFontSize(fontSizes[index], fontSizes[index]);
        text->setCharSpace(-0.25f);
        text->setFontColor(JUtility::TColor(238, 238, 232, 255),
            JUtility::TColor(255, 255, 255, 255));
        promptGroup->appendChild(text);

        if (iconTextures[index] != nullptr) {
            const f32 halfIcon = iconSizes[index] * 0.5f;
            auto* icon = JKR_NEW J2DPicture(iconTags[index],
                JGeometry::TBox2<f32>(railX - halfIcon,
                    centers[index] - halfIcon, railX + halfIcon,
                    centers[index] + halfIcon),
                iconTextures[index], nullptr);
            configure_hd_picture(icon);
            promptGroup->appendChild(icon);
        }
    }
    if (previousHeap != nullptr) {
        mDoExt_setCurrentHeap(previousHeap);
    }
}

void position_fmap_prompt_overlay(dMenu_Fmap2DTop_c* map) {
    if (map == nullptr || map->mpTitleScreen == nullptr ||
        map->mpContPane == nullptr) {
        return;
    }
    J2DPane* promptGroup = map->mpTitleScreen->search(MULTI_CHAR('hd_fctl'));
    J2DPane* controlAnchor = map->mpContPane->getPanePtr();
    if (promptGroup == nullptr || controlAnchor == nullptr) {
        return;
    }

    constexpr f32 promptWidth = 150.0f;
    // This hook runs before fMapTopWide(), so the native pane still contains
    // the previous frame's resize transform. Calculate the same right-safe
    // anchor directly for the current frame to prevent the controls jumping
    // while the window is resized.
    constexpr f32 frameRightAnchor = 515.0f;
    constexpr f32 frameTopAnchor = 83.0f;
    promptGroup->move(
        mDoGph_gInf_c::ScaleHUDXRight(frameRightAnchor) - promptWidth,
        frameTopAnchor);
    promptGroup->setAlpha(255);
    promptGroup->show();
    controlAnchor->hide();
}

void apply_fmap_top(dMenu_Fmap2DTop_c* map) {
    if (map == nullptr || map->mpTitleScreen == nullptr) {
        return;
    }
    add_fmap_top_overlay(map);
    update_menu_face_button(map->mpTitleScreen, MULTI_CHAR('hd_fai1'), true);
    update_menu_face_button(map->mpTitleScreen, MULTI_CHAR('hd_fbi2'), false);
    position_fmap_prompt_overlay(map);

    constexpr u64 titleTags[] = {
        MULTI_CHAR('ffont00'), MULTI_CHAR('ffontl0'), MULTI_CHAR('ffontl1'),
        MULTI_CHAR('ffontl2'), MULTI_CHAR('ffontb0'), MULTI_CHAR('ffontb3'),
        MULTI_CHAR('ffontb4'),
    };
    constexpr u64 zTags[] = {
        MULTI_CHAR('font_zt1'), MULTI_CHAR('font_zt2'), MULTI_CHAR('font_zt3'),
        MULTI_CHAR('font_zt4'), MULTI_CHAR('font_zt5'),
    };
    constexpr u64 aTags[] = {
        MULTI_CHAR('font_at1'), MULTI_CHAR('font_at2'), MULTI_CHAR('font_at3'),
        MULTI_CHAR('font_at4'), MULTI_CHAR('font_at5'),
    };
    constexpr u64 bTags[] = {
        MULTI_CHAR('font_bt1'), MULTI_CHAR('font_bt2'), MULTI_CHAR('font_bt3'),
        MULTI_CHAR('font_bt4'), MULTI_CHAR('font_bt5'),
    };
    const u64* sourceTags[] = {zTags, aTags, bTags};
    const std::size_t sourceCounts[] = {
        std::size(zTags), std::size(aTags), std::size(bTags),
    };
    constexpr u64 outputTags[] = {
        MULTI_CHAR('hd_fpt0'), MULTI_CHAR('hd_fpt1'), MULTI_CHAR('hd_fpt2'),
    };
    constexpr u64 iconTags[] = {
        MULTI_CHAR('hd_fri0'), MULTI_CHAR('hd_fai1'), MULTI_CHAR('hd_fbi2'),
    };

    if (auto* output = static_cast<J2DTextBox*>(
            map->mpTitleScreen->search(MULTI_CHAR('hd_fttx')))) {
        if (J2DTextBox* source = fmap_text_source(map->mpTitleScreen,
                titleTags, std::size(titleTags))) {
            output->setString(64, text_box_string(source));
        }
    }
    for (std::size_t index = 0; index < std::size(outputTags); ++index) {
        J2DTextBox* source = fmap_text_source(map->mpTitleScreen,
            sourceTags[index], sourceCounts[index]);
        auto* output = static_cast<J2DTextBox*>(map->mpTitleScreen->search(outputTags[index]));
        J2DPane* icon = map->mpTitleScreen->search(iconTags[index]);
        const char* value = text_box_string(source);
        const bool visible = value != nullptr && value[0] != '\0';
        if (output != nullptr) {
            output->setString(48, visible ? value : "");
            visible ? output->show() : output->hide();
        }
        if (icon != nullptr) {
            visible ? icon->show() : icon->hide();
        }
    }

    map->mpNamePane->hide();
    map->mpSubPane->hide();
    map->mpContPane->hide();
}

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
        "Options", 16, HBIND_CENTER, VBIND_CENTER);
    title->setFont(menu->mpFont);
    title->setFontSize(21.0f, 21.0f);
    title->setCharSpace(-0.5f);
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

    auto* confirm = JKR_NEW J2DTextBox(MULTI_CHAR('hd_ocfm'),
        JGeometry::TBox2<f32>(490.0f, 17.0f, 555.0f, 38.0f), nullptr,
        "Confirm", 16, HBIND_RIGHT, VBIND_CENTER);
    confirm->setFont(menu->mpFont);
    confirm->setFontSize(13.5f, 13.5f);
    confirm->setFontColor(JUtility::TColor(235, 235, 230, 255),
        JUtility::TColor(255, 255, 255, 255));
    group->appendChild(confirm);

    addPicture(MULTI_CHAR('hd_oapi'),
        JGeometry::TBox2<f32>(558.0f, 14.0f, 585.0f, 41.0f),
        menu_face_button_texture(true));
    addPicture(MULTI_CHAR('hd_obck'),
        JGeometry::TBox2<f32>(508.0f, 35.0f, 547.0f, 51.0f),
        resource_texture(s_fileSelectBackLabelResource));
    addPicture(MULTI_CHAR('hd_obpi'),
        JGeometry::TBox2<f32>(548.5f, 35.5f, 570.5f, 57.5f),
        menu_face_button_texture(false));
    addPicture(MULTI_CHAR('hd_oflr'),
        JGeometry::TBox2<f32>(562.0f, 29.0f, 600.0f, 67.0f),
        resource_texture(s_fileSelectPromptFlourishResource), 205);

    auto* displayPrompt = JKR_NEW J2DTextBox(MULTI_CHAR('hd_odsp'),
        JGeometry::TBox2<f32>(403.0f, 412.0f, 570.0f, 435.0f), nullptr,
        "Brightness / Screen Ratio", 32, HBIND_RIGHT, VBIND_CENTER);
    displayPrompt->setFont(menu->mpFont);
    displayPrompt->setFontSize(11.5f, 11.5f);
    displayPrompt->setFontColor(JUtility::TColor(205, 201, 176, 255),
        JUtility::TColor(244, 238, 191, 255));
    group->appendChild(displayPrompt);
    addPicture(MULTI_CHAR('hd_orbt'),
        JGeometry::TBox2<f32>(575.0f, 414.0f, 601.0f, 433.0f),
        styled_r_button_texture());
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
    if (selected < 3 && overlay != nullptr) {
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
        "Device Settings", 24, HBIND_CENTER, VBIND_CENTER);
    title->setFont(font);
    title->setFontSize(19.0f, 19.0f);
    title->setCharSpace(-0.5f);
    title->setFontColor(JUtility::TColor(242, 242, 235, 255),
        JUtility::TColor(255, 255, 255, 255));
    overlay->appendChild(title);

    auto* complete = JKR_NEW J2DTextBox(MULTI_CHAR('hd_tvcp'),
        JGeometry::TBox2<f32>(475.0f, 17.0f, 555.0f, 41.0f), nullptr,
        "Complete", 16, HBIND_RIGHT, VBIND_CENTER);
    complete->setFont(font);
    complete->setFontSize(14.0f, 14.0f);
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
            text->setString(48, "Make the adjustments on the device.");
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

    const JGeometry::TBox2<f32> bounds = button->getBounds();
    button->changeTexture(replacement, 0);
    button->resize(bounds.getWidth(), bounds.getHeight());
    button->move(bounds.i.x, bounds.i.y);
    button->setTexCoord(button->getTexture(0), BIND15, MIRROR0, false);
    button->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    button->setCornerColor(JUtility::TColor(255, 255, 255, 255));
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
    ResTIMG const* label = resource_texture(s_fileSelectBackLabelResource);
    J2DPane* buttonPane = menu != nullptr && menu->mBbtnPane != nullptr ?
        menu->mBbtnPane->getPanePtr() : nullptr;
    if (buttonPane == nullptr ||
        label == nullptr ||
        buttonPane->search(MULTI_CHAR('hd_back')) != nullptr) {
        return;
    }

    // The label is a true child of B. It therefore inherits every move,
    // resize, aspect-ratio correction, and visibility animation of the
    // button instead of drifting in screen coordinates.
    auto* overlay = JKR_NEW J2DPicture(MULTI_CHAR('hd_back'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), label, nullptr);
    overlay->setTexCoord(overlay->getTexture(0), BIND15, MIRROR0, false);
    overlay->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    overlay->setCornerColor(JUtility::TColor(255, 255, 255, 255));
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
        label->resize(39.0f, 16.0f);
        // The label texture has transparent side bearings. Let its picture
        // overlap the button pane slightly so the visible word sits close to
        // the B disc, as it does in TPHD.
        label->move(-25.0f, (buttonHeight - 16.0f) * 0.5f);
    }

    if (flourish != nullptr) {
        // Preserve the taller three-curl silhouette used by the TPHD prompt.
        flourish->resize(30.0f, 32.0f);
        flourish->move(buttonWidth - 12.0f,
            (buttonHeight - 32.0f) * 0.5f - 2.0f);
    }
}

f32 copy_title_text_width(JUTFont* font, const char* text, f32 size,
    f32 spacing);
J2DTextBox* copy_metadata_text(J2DPane* group, u64 tag,
    const JGeometry::TBox2<f32>& bounds, JUTFont* font,
    J2DTextBoxHBinding binding, f32 size,
    const JUtility::TColor& color);

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

    for (std::size_t index = 0; index < 3; ++index) {
        J2DPane* group = menu->m3mSelPane[index] != nullptr ?
            menu->m3mSelPane[index]->getPanePtr() : nullptr;
        if (group == nullptr) {
            continue;
        }

        // A Yes/No modal temporarily hides these panes. Restore them once
        // File Selection returns to its ordinary action-selection state.
        if (menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT ||
            menu->mDataSelProc ==
                dFile_select_c::DATASELPROC_MENU_SELECT_MOVE_ANM) {
            group->show();
        }

        auto* frame = as_picture(group->search(frameTags[index]));
        if (frame == nullptr) {
            frame = JKR_NEW J2DPicture(frameTags[index],
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f),
                frameTexture, nullptr);
            frame->setTexCoord(frame->getTexture(0), BIND15, MIRROR0, false);
            frame->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            // Insert first so the existing text pane is drawn afterward.
            // Appending placed the opaque panel over Copy/Start/Erase.
            group->insertChild(group->getFirstChildPane(), frame);
        }

        // The original three-menu artwork stacks bright stone borders,
        // patterned fills, and selection effects. TPHD reduces each action
        // to one compact dark panel with a fine gold edge.
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
        frame->setAlpha(255);
        frame->show();

        auto* text = menu->m3mSelTextPane[index] != nullptr ?
            static_cast<J2DTextBox*>(menu->m3mSelTextPane[index]->getPanePtr()) : nullptr;
        if (text != nullptr) {
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
    const JGeometry::TBox2<f32> bounds = panel->getBounds();
    panel->changeTexture(clear, 0);
    panel->resize(bounds.getWidth(), bounds.getHeight());
    panel->move(bounds.i.x, bounds.i.y);
    panel->setTexCoord(panel->getTexture(0), BIND15, MIRROR0, false);
    panel->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    panel->setCornerColor(JUtility::TColor(255, 255, 255, 255));
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
    pane->add(targetX - centerX, targetY - centerY);
}

void position_file_select_prompts(dFile_select_c* menu) {
    if (menu == nullptr || menu->fileSel.Scr == nullptr) {
        return;
    }
    const JGeometry::TBox2<f32> rootBounds = menu->fileSel.Scr->getBounds();
    const f32 right = rootBounds.f.x;
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

    J2DPicture* background = as_picture(
        menu->fileSel.Scr->search(MULTI_CHAR('w_dmbase')));
    if (background == nullptr) {
        return;
    }
    replace_large_file_select_backgrounds(menu->fileSel.Scr, brick, replacement,
        background);
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
            const JGeometry::TBox2<f32> bounds = base->getBounds();
            base->changeTexture(replacement, 0);
            base->resize(bounds.getWidth(), bounds.getHeight());
            base->move(bounds.i.x, bounds.i.y);
            base->setTexCoord(base->getTexture(0), BIND15, MIRROR0, false);
            base->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            base->setCornerColor(JUtility::TColor(255, 255, 255, 255));
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
                const JGeometry::TBox2<f32> bounds = base->getBounds();
                base->changeTexture(clear, 0);
                base->resize(bounds.getWidth(), bounds.getHeight());
                base->move(bounds.i.x, bounds.i.y);
                base->setTexCoord(base->getTexture(0), BIND15, MIRROR0, false);
                base->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    JUtility::TColor(255, 255, 255, 255));
                base->setCornerColor(JUtility::TColor(255, 255, 255, 255));
                base->show();
            }
        }
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
    auto* name = copy_metadata_text(overlay, MULTI_CHAR('hd_mnam'),
        JGeometry::TBox2<f32>(left + 125.0f, rowY - 21.0f,
            left + 235.0f, rowY - 3.0f), font, HBIND_RIGHT, 13.5f,
        JUtility::TColor(235, 232, 211, 255));
    auto* saveLabel = copy_metadata_text(overlay, MULTI_CHAR('hd_msav'),
        JGeometry::TBox2<f32>(left + 125.0f, rowY - 5.0f,
            left + 235.0f, rowY + 9.0f), font, HBIND_RIGHT, 11.5f,
        JUtility::TColor(190, 202, 255, 255));
    auto* playLabel = copy_metadata_text(overlay, MULTI_CHAR('hd_mply'),
        JGeometry::TBox2<f32>(left + 105.0f, rowY + 10.0f,
            left + 235.0f, rowY + 25.0f), font, HBIND_RIGHT, 11.5f,
        JUtility::TColor(188, 225, 112, 255));
    auto* saveValue = copy_metadata_text(overlay, MULTI_CHAR('hd_msva'),
        JGeometry::TBox2<f32>(left + 245.0f, rowY - 5.0f,
            left + 420.0f, rowY + 9.0f), font, HBIND_LEFT, 11.5f,
        JUtility::TColor(190, 202, 255, 255));
    auto* playValue = copy_metadata_text(overlay, MULTI_CHAR('hd_mpva'),
        JGeometry::TBox2<f32>(left + 245.0f, rowY + 10.0f,
            left + 420.0f, rowY + 25.0f), font, HBIND_LEFT, 11.5f,
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
            heartPicture->move(left + 245.0f + column * 15.0f,
                rowY - 17.0f + line * 12.0f);
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

bool copy_destination_confirm_state(const dFile_select_c* menu) {
    if (menu == nullptr) {
        return false;
    }
    return menu->mDataSelProc >= dFile_select_c::DATASELPROC_YES_NO_SELECT &&
        menu->mDataSelProc <= dFile_select_c::DATASELPROC_CMD_EXEC_PANE_MOVE0;
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
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_cno'), MULTI_CHAR('hd_cyes'),
    };
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
        auto* frame = as_picture(group->search(frameTags[index]));
        if (frame == nullptr) {
            frame = JKR_NEW J2DPicture(frameTags[index],
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
        // File Selection normally scales/slides the old and new Yes/No panes
        // for several frames. Those transforms fight this fixed TPHD layout
        // and produce a visible flash when changing choices, so detach them.
        group->setAnimation(static_cast<J2DAnmTransform*>(nullptr));
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
        if (selected && menu->mSelIcon != nullptr) {
            // Native File Selection fades this cursor out while its original
            // Yes/No panes animate between choices. Our panes are fixed, so
            // keep the corners visible and simply move them to the new choice.
            menu->mSelIcon->setAlphaRate(1.0f);
            position_cursor_outside_frame(menu->mSelIcon, frame);
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
    const bool confirmActive = copy_destination_confirm_state(menu);
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
            rows[index]->resize(435.0f, 62.0f);
            rows[index]->move((608.0f - 435.0f) * 0.5f,
                rowY - 31.0f);
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
    const int selectedIndex = menu->getSelectNum();
    for (std::size_t index = 0; index < 3; ++index) {
        ResTIMG const* texture = static_cast<int>(index) == selectedIndex ? selected : normal;
        J2DPane* rowGroup = menu->fileSel.Scr->search(rowBaseTags[index]);
        J2DPicture* rowBase = as_picture(rowGroup);
        if (rowBase == nullptr) {
            f32 largestArea = 0.0f;
            find_largest_picture(rowGroup, rowBase, largestArea);
        }
        apply_file_select_row_texture(rowBase, texture);
    }
}

void position_cursor_outside_frame(dSelect_cursor_c* cursor, J2DPane* target,
    f32 horizontalPadding, f32 verticalPadding) {
    if (cursor == nullptr || target == nullptr || cursor->mpScreen == nullptr) {
        return;
    }

    const auto& bounds = target->getGlbBounds();
    cursor->setPos((bounds.i.x + bounds.f.x) * 0.5f,
        (bounds.i.y + bounds.f.y) * 0.5f, target, true);

    constexpr u64 cornerTags[] = {
        MULTI_CHAR('l_u_null'), MULTI_CHAR('l_d_null'),
        MULTI_CHAR('r_u_null'), MULTI_CHAR('r_d_null'),
    };
    const f32 phase = cursor->field_0x40;
    const f32 oscillation = phase < 10.0f ? phase / 10.0f :
        (20.0f - phase) / 10.0f;
    const f32 pulse = (1.0f - cursor->mParam3) +
        oscillation * cursor->mParam3;

    for (int corner = 0; corner < 4; ++corner) {
        cursor->field_0x94[corner] +=
            corner < 2 ? -horizontalPadding : horizontalPadding;
        cursor->field_0xa4[corner] +=
            corner % 2 == 0 ? -verticalPadding : verticalPadding;
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

    constexpr u64 actionFrameTags[] = {
        MULTI_CHAR('hd_3m0'), MULTI_CHAR('hd_3m1'), MULTI_CHAR('hd_3m2'),
    };
    const bool launchingFromStart =
        menu->mDataSelProc == dFile_select_c::DATASELPROC_NEXT_MODE_WAIT &&
        menu->mIsSelectEnd && menu->mSelectMenuNum == 1;
    if ((menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT ||
            menu->mDataSelProc == dFile_select_c::DATASELPROC_MENU_SELECT_MOVE_ANM ||
            launchingFromStart) &&
        menu->mSelectMenuNum < 3) {
        J2DPane* group = menu->m3mSelPane[menu->mSelectMenuNum] != nullptr ?
            menu->m3mSelPane[menu->mSelectMenuNum]->getPanePtr() : nullptr;
        J2DPane* frame = group != nullptr ?
            group->search(actionFrameTags[menu->mSelectMenuNum]) : nullptr;
        if (frame != nullptr) {
            position_cursor_outside_frame(menu->mSelIcon, frame);
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
    const JGeometry::TBox2<f32>& bounds = rowBase->getGlbBounds();
    menu->mSelIcon->setPos((bounds.i.x + bounds.f.x) * 0.5f,
        (bounds.i.y + bounds.f.y) * 0.5f, rowBase, true);

    // Apply one small, row-independent inset in the pane's virtual layout
    // units.  The selected box may scale, but these offsets remain tied to its
    // edges instead of to the host window's pixels.
    for (int corner = 0; corner < 4; ++corner) {
        menu->mSelIcon->field_0x94[corner] += corner < 2 ? 2.0f : -2.0f;
    }
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
        const JGeometry::TBox2<f32>& groupGlobal = group->getGlbBounds();
        const f32 scaleX = group->getWidth() > 0.0f ?
            groupGlobal.getWidth() / group->getWidth() : 1.0f;
        const f32 scaleY = group->getHeight() > 0.0f ?
            groupGlobal.getHeight() / group->getHeight() : 1.0f;

        // Resolve every position from live pane geometry. The archive applies
        // one last transform after reset/create; additive one-time offsets
        // consequently left individual labels stranded around the screen.
        // These relationship-based corrections are idempotent and safe to
        // repeat immediately before every draw.
        if (saveLabel != nullptr && std::isfinite(scaleY) &&
            std::fabs(scaleY) > 0.001f) {
            bool hasSecondHeartLine = false;
            for (std::size_t heart = 10; heart < 20; ++heart) {
                J2DPane* pane = group->search(heartTags[heart]);
                if (pane != nullptr && pane->isVisible()) {
                    hasSecondHeartLine = true;
                    break;
                }
            }
            const f32 saveTop = saveLabel->getGlbBounds().i.y;
            for (std::size_t heart = 0; heart < 20; ++heart) {
                J2DPane* pane = group->search(heartTags[heart]);
                if (pane == nullptr || !pane->isVisible()) {
                    continue;
                }
                const JGeometry::TBox2<f32>& bounds = pane->getGlbBounds();
                const f32 targetBottom = saveTop - 3.0f * scaleY -
                    (hasSecondHeartLine && heart < 10 ?
                        bounds.getHeight() : 0.0f);
                if (std::isfinite(bounds.f.y) &&
                    std::isfinite(targetBottom)) {
                    pane->add(0.0f, (targetBottom - bounds.f.y) / scaleY);
                }
            }
        }

        const f32 heartLeft = firstHeart != nullptr ?
            firstHeart->getGlbBounds().i.x : 0.0f;
        const bool canAlign = firstHeart != nullptr &&
            std::isfinite(heartLeft) && std::isfinite(scaleX) &&
            std::isfinite(scaleY) && std::fabs(scaleX) > 0.001f &&
            std::fabs(scaleY) > 0.001f;

        if (name != nullptr) {
            name->setFontSize(13.5f, 13.5f);
            if (saveLabel != nullptr && canAlign) {
                name->mFlags = static_cast<u8>((name->mFlags & ~0x0c) |
                    (HBIND_RIGHT << 2));
                const JGeometry::TBox2<f32>& nameBounds = name->getGlbBounds();
                const JGeometry::TBox2<f32>& labelBounds =
                    saveLabel->getGlbBounds();
                const JGeometry::TBox2<f32>& heartBounds =
                    firstHeart->getGlbBounds();
                const f32 deltaX = labelBounds.f.x - nameBounds.f.x;
                const f32 deltaY =
                    (heartBounds.i.y + heartBounds.f.y -
                        nameBounds.i.y - nameBounds.f.y) * 0.5f;
                if (std::isfinite(deltaX) && std::isfinite(deltaY)) {
                    name->add(deltaX / scaleX, deltaY / scaleY);
                }
            }
        }
        for (u64 tag : labelTags) {
            auto* text = static_cast<J2DTextBox*>(group->search(tag));
            if (text != nullptr) {
                text->setFontSize(11.5f, 11.5f);
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
                        text->getGlbBounds().i.x;
                    if (std::isfinite(deltaX)) {
                        text->add(deltaX / scaleX, 0.0f);
                    }
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
    const bool active = nativeString != nullptr &&
        std::strstr(nativeString, "Save to which log?") != nullptr;
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

    constexpr f32 size = 24.0f;
    constexpr f32 spacing = -0.5f;
    JUTFont* font = menu->mSaveSel.font[0];
    const char* pieces[] = {"Save to which lo", "g", "?"};
    constexpr u64 tags[] = {
        MULTI_CHAR('hd_st0'), MULTI_CHAR('hd_st1'), MULTI_CHAR('hd_st2'),
    };
    const f32 totalWidth = copy_title_text_width(font,
        "Save to which log?", size, spacing);
    f32 left = (608.0f - totalWidth) * 0.5f;
    for (std::size_t index = 0; index < 3; ++index) {
        const f32 pieceWidth = copy_title_text_width(font, pieces[index],
            size, spacing);
        const f32 opticalDrop = index == 1 ? 2.0f : 0.0f;
        auto* text = copy_metadata_text(group, tags[index],
            JGeometry::TBox2<f32>(left, 30.0f + opticalDrop,
                left + pieceWidth + 4.0f, 66.0f + opticalDrop),
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
                panel->move(0.0f, 0.0f);
                panel->resize(row->getWidth(), row->getHeight());
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
    for (std::size_t index = 0; index < 3; ++index) {
        apply_file_select_row_texture(save_menu_row_picture(menu, index),
            index == menu->mSelectedFile ? selected : normal);
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
        J2DPane* firstHeart = group->search(MULTI_CHAR('hear_20'));
        const f32 heartLeft = firstHeart != nullptr ?
            firstHeart->getGlbBounds().i.x : 0.0f;
        const bool canAlignValues = firstHeart != nullptr &&
            std::isfinite(heartLeft) && heartLeft > -1000.0f &&
            heartLeft < 1000.0f;
        if (name != nullptr) {
            name->setFontSize(13.5f, 13.5f);
        }
        const JGeometry::TBox2<f32>& groupGlobal = group->getGlbBounds();
        const f32 scaleX = group->getWidth() > 0.0f ?
            groupGlobal.getWidth() / group->getWidth() : 1.0f;
        const f32 scaleY = group->getHeight() > 0.0f ?
            groupGlobal.getHeight() / group->getHeight() : 1.0f;
        if (saveLabel != nullptr && std::isfinite(scaleY) &&
            std::fabs(scaleY) > 0.001f) {
            bool hasSecondHeartLine = false;
            for (std::size_t heart = 10; heart < 20; ++heart) {
                J2DPane* pane = group->search(heartTags[heart]);
                if (pane != nullptr && pane->isVisible()) {
                    hasSecondHeartLine = true;
                    break;
                }
            }
            const f32 saveTop = saveLabel->getGlbBounds().i.y;
            for (std::size_t heart = 0; heart < 20; ++heart) {
                J2DPane* pane = group->search(heartTags[heart]);
                if (pane == nullptr || !pane->isVisible()) {
                    continue;
                }
                const JGeometry::TBox2<f32>& bounds = pane->getGlbBounds();
                const f32 targetBottom = saveTop - 3.0f * scaleY -
                    (hasSecondHeartLine && heart < 10 ?
                        bounds.getHeight() : 0.0f);
                if (std::isfinite(bounds.f.y) &&
                    std::isfinite(targetBottom)) {
                    pane->add(0.0f, (targetBottom - bounds.f.y) / scaleY);
                }
            }
        }
        if (name != nullptr && saveLabel != nullptr && firstHeart != nullptr &&
            std::isfinite(scaleX) && std::isfinite(scaleY) &&
            std::fabs(scaleX) > 0.001f && std::fabs(scaleY) > 0.001f) {
            name->mFlags = static_cast<u8>((name->mFlags & ~0x0c) |
                (HBIND_RIGHT << 2));
            const JGeometry::TBox2<f32>& nameBounds = name->getGlbBounds();
            const JGeometry::TBox2<f32>& labelBounds =
                saveLabel->getGlbBounds();
            const JGeometry::TBox2<f32>& heartBounds =
                firstHeart->getGlbBounds();
            const f32 deltaX = labelBounds.f.x - nameBounds.f.x;
            const f32 deltaY =
                (heartBounds.i.y + heartBounds.f.y -
                    nameBounds.i.y - nameBounds.f.y) * 0.5f;
            if (std::isfinite(deltaX) && std::isfinite(deltaY)) {
                name->add(deltaX / scaleX, deltaY / scaleY);
            }
        }
        for (u64 tag : labelTags) {
            auto* text = static_cast<J2DTextBox*>(group->search(tag));
            if (text != nullptr) {
                text->setFontSize(11.5f, 11.5f);
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
                const f32 valueLeft = text->getGlbBounds().i.x;
                if (canAlignValues && std::isfinite(valueLeft) &&
                    valueLeft > -1000.0f && valueLeft < 1000.0f) {
                    // Match File Selection's shared value column: both the
                    // date and total time begin beneath the first heart.
                    text->add(heartLeft + 5.0f - valueLeft, 0.0f);
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

void add_save_menu_fixed_prompts(dMenu_save_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->mSaveSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }
    if (screen->search(MULTI_CHAR('hd_sprm')) != nullptr) {
        update_menu_face_button(screen, MULTI_CHAR('hd_sapi'), true);
        update_menu_face_button(screen, MULTI_CHAR('hd_sbpi'), false);
        return;
    }
    auto* group = JKR_NEW J2DPane(MULTI_CHAR('hd_sprm'),
        JGeometry::TBox2<f32>(0.0f, 0.0f, 608.0f, 448.0f));
    screen->appendChild(group);

    auto addPicture = [group](u64 tag, const JGeometry::TBox2<f32>& bounds,
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

    auto* confirm = JKR_NEW J2DTextBox(MULTI_CHAR('hd_sconf'),
        JGeometry::TBox2<f32>(496.0f, 16.0f, 560.0f, 37.0f), nullptr,
        "Confirm", 16, HBIND_RIGHT, VBIND_CENTER);
    confirm->setFont(menu->mSaveSel.font[0]);
    confirm->setFontSize(13.5f, 13.5f);
    confirm->setFontColor(JUtility::TColor(235, 235, 230, 255),
        JUtility::TColor(255, 255, 255, 255));
    group->appendChild(confirm);

    addPicture(MULTI_CHAR('hd_sapi'),
        JGeometry::TBox2<f32>(558.0f, 14.0f, 585.0f, 41.0f),
        menu_face_button_texture(true));
    addPicture(MULTI_CHAR('hd_sbck'),
        JGeometry::TBox2<f32>(508.0f, 34.0f, 547.0f, 50.0f),
        resource_texture(s_fileSelectBackLabelResource));
    addPicture(MULTI_CHAR('hd_sbpi'),
        JGeometry::TBox2<f32>(546.0f, 32.0f, 573.0f, 59.0f),
        menu_face_button_texture(false));
    addPicture(MULTI_CHAR('hd_sflr'),
        JGeometry::TBox2<f32>(562.0f, 29.0f, 600.0f, 67.0f),
        resource_texture(s_fileSelectPromptFlourishResource), 205);
}

void add_save_menu_prompt_overlays(dMenu_save_c* menu) {
    J2DPane* button = menu != nullptr && menu->mpBBtnIcon != nullptr ?
        menu->mpBBtnIcon->getPanePtr() : nullptr;
    if (button == nullptr) {
        return;
    }
    if (button->search(MULTI_CHAR('hd_sback')) == nullptr) {
        if (ResTIMG const* texture = resource_texture(s_fileSelectBackLabelResource)) {
            auto* label = JKR_NEW J2DPicture(MULTI_CHAR('hd_sback'),
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), texture, nullptr);
            label->setTexCoord(label->getTexture(0), BIND15, MIRROR0, false);
            label->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            label->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            button->appendChild(label);
        }
    }
    if (button->search(MULTI_CHAR('hd_sswrl')) == nullptr) {
        if (ResTIMG const* texture =
                resource_texture(s_fileSelectPromptFlourishResource)) {
            auto* flourish = JKR_NEW J2DPicture(MULTI_CHAR('hd_sswrl'),
                JGeometry::TBox2<f32>(0.0f, 0.0f, 1.0f, 1.0f), texture, nullptr);
            flourish->setTexCoord(flourish->getTexture(0), BIND15, MIRROR0, false);
            flourish->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            flourish->setCornerColor(JUtility::TColor(255, 255, 255, 205));
            button->appendChild(flourish);
        }
    }
}

void position_save_menu_prompts(dMenu_save_c* menu) {
    J2DScreen* screen = menu != nullptr ? menu->mSaveSel.Scr : nullptr;
    if (screen == nullptr) {
        return;
    }
    const auto rootBounds = screen->getBounds();
    const f32 right = rootBounds.f.x;
    const f32 top = rootBounds.i.y;
    move_file_select_pane_center(menu->mpConfirmTxt != nullptr ?
        menu->mpConfirmTxt->getPanePtr() : nullptr, right - 66.0f, top + 28.0f);
    move_file_select_pane_center(menu->mpABtnIcon != nullptr ?
        menu->mpABtnIcon->getPanePtr() : nullptr, right - 30.0f, top + 28.0f);
    move_file_select_pane_center(menu->mpBackTxt != nullptr ?
        menu->mpBackTxt->getPanePtr() : nullptr, right - 67.0f, top + 46.0f);
    move_file_select_pane_center(menu->mpBBtnIcon != nullptr ?
        menu->mpBBtnIcon->getPanePtr() : nullptr, right - 47.0f, top + 46.0f);

    J2DPane* button = menu->mpBBtnIcon != nullptr ?
        menu->mpBBtnIcon->getPanePtr() : nullptr;
    if (button == nullptr) {
        return;
    }
    if (J2DPane* label = button->search(MULTI_CHAR('hd_sback'))) {
        label->resize(39.0f, 16.0f);
        label->move(-25.0f, (button->getHeight() - 16.0f) * 0.5f);
    }
    if (J2DPane* flourish = button->search(MULTI_CHAR('hd_sswrl'))) {
        flourish->resize(30.0f, 32.0f);
        flourish->move(button->getWidth() - 12.0f,
            (button->getHeight() - 32.0f) * 0.5f - 2.0f);
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
    add_save_menu_prompt_overlays(menu);
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
    style_save_select_title(menu);
    add_save_menu_title_rules(menu);
    style_save_menu_prompts(menu);
    add_save_menu_fixed_prompts(menu);
    style_save_menu_yes_no_buttons(menu);
}

void replace_collect_button_group(J2DPicture* glyph, ResTIMG const* baseTexture,
    ResTIMG const* decorationTexture, ResTIMG const* replacement) {
    if (glyph == nullptr || baseTexture == nullptr || replacement == nullptr) {
        return;
    }

    J2DPane* parent = glyph->getParentPane();
    if (parent == nullptr) {
        return;
    }

    J2DPicture* button = nullptr;
    for (J2DPane* child = parent->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        J2DPicture* picture = as_picture(child);
        if (picture == nullptr || picture->getTexture(0) == nullptr ||
            picture->getTexture(0)->getTexInfo() != baseTexture) {
            continue;
        }

        button = picture;
        break;
    }
    if (button == nullptr) {
        return;
    }

    const JGeometry::TBox2<f32> buttonBounds = button->getBounds();
    button->changeTexture(replacement, 0);
    button->resize(buttonBounds.getWidth(), buttonBounds.getHeight());
    button->move(buttonBounds.i.x, buttonBounds.i.y);
    button->setTexCoord(button->getTexture(0), BIND15, MIRROR0, false);
    button->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
        JUtility::TColor(255, 255, 255, 255));
    button->setCornerColor(JUtility::TColor(255, 255, 255, 255));
    glyph->hide();

    // TPHD keeps one restrained gold flourish just to the right of each
    // neutral button disc. Remove the GameCube halo's other image layers and
    // refit the long gold curl to that compact footprint. Prompt text panes
    // are not pictures and remain untouched.
    for (J2DPane* child = parent->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        J2DPicture* picture = as_picture(child);
        if (picture == nullptr || picture == button || picture == glyph ||
            picture->getTexture(0) == nullptr) {
            continue;
        }

        ResTIMG const* texture = picture->getTexture(0)->getTexInfo();
        if (texture == decorationTexture) {
            const f32 targetHeight = buttonBounds.getHeight() * 0.82f;
            const f32 sourceHeight = static_cast<f32>(decorationTexture->height);
            const f32 sourceWidth = static_cast<f32>(decorationTexture->width);
            const f32 targetWidth = sourceHeight > 0.0f ?
                std::min(targetHeight * sourceWidth / sourceHeight,
                    buttonBounds.getWidth() * 1.35f) : targetHeight;
            picture->resize(targetWidth, targetHeight);
            picture->move(buttonBounds.f.x - buttonBounds.getWidth() * 0.08f,
                buttonBounds.i.y + (buttonBounds.getHeight() - targetHeight) * 0.5f);
            picture->setCornerColor(JUtility::TColor(255, 255, 255, 255));
            picture->show();
        } else {
            picture->hide();
        }
    }
}

void replace_collect_buttons_in_tree(J2DPane* pane, ResTIMG const* baseTexture,
    ResTIMG const* aGlyphTexture, ResTIMG const* bGlyphTexture,
    ResTIMG const* decorationTexture,
    ResTIMG const* buttonA, ResTIMG const* buttonB) {
    if (pane == nullptr) {
        return;
    }

    if (J2DPicture* picture = as_picture(pane);
        picture != nullptr && picture->getTexture(0) != nullptr) {
        ResTIMG const* texture = picture->getTexture(0)->getTexInfo();
        if (texture == aGlyphTexture) {
            replace_collect_button_group(picture, baseTexture, decorationTexture, buttonA);
        } else if (texture == bGlyphTexture) {
            replace_collect_button_group(picture, baseTexture, decorationTexture, buttonB);
        }
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane()) {
        replace_collect_buttons_in_tree(child, baseTexture, aGlyphTexture,
            bGlyphTexture, decorationTexture, buttonA, buttonB);
    }
}

void apply_out_font_face_button_layout(COutFont_c* outFont) {
    if (outFont == nullptr) {
        return;
    }

    // Collection descriptions use out-font types 0 and 1 for their inline A
    // and B controls. They share one out-font across every selected item, so
    // updating it once keeps all descriptions consistent with the menu prompts.
    constexpr struct {
        int type;
        bool nativeAAction;
    } buttons[] = {
        {0, true},
        {1, false},
    };
    for (const auto& button : buttons) {
        J2DPicture* picture = outFont->mpPane[button.type];
        ResTIMG const* replacement = menu_face_button_texture(button.nativeAAction);
        if (picture != nullptr && replacement != nullptr) {
            picture->changeTexture(replacement, 0);
            set_neutral_picture_colors(picture);
        }
    }
}

void apply_collect_menu_button_layout(dMenu_Collect2D_c* menu) {
    if (menu == nullptr || menu->getIconScreen() == nullptr) {
        return;
    }

    J2DScreen* screen = menu->getIconScreen();
    // The Collection layout does not use the gameplay HUD's pane tags. Locate
    // its A/B groups by the texture resources they contain: each group has a
    // shared round base plus a distinct A or B glyph. This also survives
    // regional layouts whose pane tags differ.
    ResTIMG const* baseTexture = collect_archive_texture("tt_zelda_button_ab_maru.bti");
    ResTIMG const* aGlyphTexture = collect_archive_texture("tt_zelda_button_a_text.bti");
    ResTIMG const* bGlyphTexture = collect_archive_texture("tt_zelda_button_b_text.bti");
    ResTIMG const* decorationTexture = collect_archive_texture("tt_gold_uzu_long2.bti");
    ResTIMG const* buttonA = styled_face_button_texture('A');
    ResTIMG const* buttonB = styled_face_button_texture('B');

    switch (button_layout()) {
    case ButtonLayout::Nintendo:
        break;
    case ButtonLayout::Xbox:
        std::swap(buttonA, buttonB);
        break;
    case ButtonLayout::Universal: {
        ResTIMG const* blank = styled_blank_face_button_texture();
        buttonA = blank;
        buttonB = blank;
        break;
    }
    }

    replace_collect_buttons_in_tree(screen, baseTexture, aGlyphTexture,
        bGlyphTexture, decorationTexture, buttonA, buttonB);
    if (menu->mpString != nullptr) {
        apply_out_font_face_button_layout(menu->mpString->mpOutFont);
    }
    simplify_collect_button_decoration(screen);
    replace_collect_background(menu->mpScreen);
    apply_collect_menu_typography(menu);
}

ResTIMG const* collection_submenu_texture(JKRArchive* archive,
    const char* textureName) {
    return archive != nullptr && textureName != nullptr ?
        static_cast<ResTIMG const*>(archive->getResource('TIMG', textureName)) : nullptr;
}

void apply_collection_submenu_button_layout(J2DScreen* screen,
    JKRArchive* archive, dMsgString_c* strings) {
    if (screen == nullptr || archive == nullptr) {
        return;
    }

    // Letters, Fish Journal, Hidden Skills, and Golden Bugs all instantiate
    // this same prompt layout from their own mounted archive. Match the native
    // A/B glyphs by those archive-local pointers, then install the configured
    // Nintendo, Xbox-swapped, universal, or alternate-style face buttons.
    ResTIMG const* baseTexture = collection_submenu_texture(
        archive, "tt_zelda_button_ab_maru.bti");
    ResTIMG const* aGlyphTexture = collection_submenu_texture(
        archive, "tt_zelda_button_a_text.bti");
    ResTIMG const* bGlyphTexture = collection_submenu_texture(
        archive, "tt_zelda_button_b_text.bti");
    ResTIMG const* decorationTexture = collection_submenu_texture(
        archive, "tt_gold_uzu_long2.bti");

    replace_collect_buttons_in_tree(screen, baseTexture, aGlyphTexture,
        bGlyphTexture, decorationTexture, menu_face_button_texture(true),
        menu_face_button_texture(false));
    simplify_collect_button_decoration(screen);
    if (strings != nullptr) {
        apply_out_font_face_button_layout(strings->mpOutFont);
    }
}

// In-game HUD ---------------------------------------------------------------

void apply_button_layout_preference(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    const ButtonLayout layout = button_layout();
    ResTIMG const* buttonA = styled_face_button_texture('A');
    ResTIMG const* buttonB = styled_face_button_texture('B');

    if (layout == ButtonLayout::Nintendo) {
        set_face_button_texture(meter, MULTI_CHAR('a_btn'), buttonA);
        set_face_button_texture(meter, MULTI_CHAR('b_btn'), buttonB);
        set_face_button_texture(meter, MULTI_CHAR('x_btn'), styled_face_button_texture('X'));
        set_face_button_texture(meter, MULTI_CHAR('y_btn'), styled_face_button_texture('Y'));
        return;
    }

    if (layout == ButtonLayout::Universal) {
        ResTIMG const* blank = styled_blank_face_button_texture();
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
        set_face_button_texture(meter, MULTI_CHAR('x_btn'), styled_face_button_texture('Y'));
        set_face_button_texture(meter, MULTI_CHAR('y_btn'), styled_face_button_texture('X'));
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
        // This stock item-light picture retains its original top-right HUD
        // anchor at 100% scale and exposes a clipped sword-shaped remnant at
        // the screen edge. The replacement B disc already supplies its own
        // shading, so the extra item light is both redundant and unsafe to
        // relocate independently of its parent group.
        meter->mpLightB->hide();
        meter->mpLightB->setAlphaRate(0.0f);
    }

    // The B action uses only the primary item picture. The stock meter also
    // owns a dynamically-created secondary picture for layered item artwork;
    // its animation can re-show it at the original GameCube top-right anchor,
    // leaving the lower end of the sword visible through the screen edge.
    if (meter->mpItemBPane != nullptr) {
        meter->mpItemBPane->hide();
        meter->mpItemBPane->setAlpha(0);
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

    ResTIMG const* texture = styled_r_button_texture();
    if (s_wiiURButtonPicture == nullptr) {
        s_wiiURButtonPicture = as_picture(meter->mpScreen->search(MULTI_CHAR('zbtn')));
        if (texture != nullptr && s_wiiURButtonPicture != nullptr) {
            s_wiiURButtonPicture->changeTexture(texture, 0);
            if (s_wiiURButtonPicture->getTexture(0) != nullptr) {
                s_wiiURButtonPicture->setTexCoord(
                    s_wiiURButtonPicture->getTexture(0), BIND15, MIRROR0, false);
            }
            resize_pane_around_center(s_wiiURButtonPicture, 64.0f, 64.0f);
        }
    } else if (texture != nullptr) {
        s_wiiURButtonPicture->changeTexture(texture, 0);
    }

    const u8 itemNo = dComIfGp_getSelectItem(kZItemSlot);
    const bool itemRingOpen = s_ringZPrompt.ring != nullptr;
    const bool rHudVisible = meter->mpButtonParent != nullptr &&
        meter->mpButtonParent->getPanePtr()->isVisible() &&
        meter->mpButtonParent->getAlphaRate() > 0.01f &&
        itemNo != dItemNo_NONE_e && itemNo != 0 &&
        !daPy_py_c::checkNowWolf() &&
        (!z_item_menu_or_pause_context() || itemRingOpen);

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

    // The Wii U badge includes its own R glyph.  The stock Z prompt is split
    // into a glyph, glow, and action-text pane; its animation can re-show the
    // panes after hide(), so also force their local alpha to zero.  This does
    // not touch zbtn (our R badge), item_r_n, or the assigned item/count.
    constexpr u64 legacyZPanes[] = {
        MULTI_CHAR('z_btn_t'),
        MULTI_CHAR('z_btnl'),
        MULTI_CHAR('z_text_n'),
    };
    for (const u64 tag : legacyZPanes) {
        if (J2DPane* pane = meter->mpScreen->search(tag)) {
            pane->hide();
            pane->setAlpha(0);
            pane->scale(0.0f, 0.0f);
        }
    }
    if (meter->mpTextXY[2] != nullptr) {
        meter->mpTextXY[2]->hide();
        meter->mpTextXY[2]->getPanePtr()->setAlpha(0);
        meter->mpTextXY[2]->scale(0.0f, 0.0f);
    }

    // z_text_n is only the common parent.  The stock renderer retains five
    // independently drawn text/shadow children and can leave those children
    // visible even after the parent has been reused for our R slot.  Clear and
    // collapse every layer directly so the detached dot + "Z" cannot survive
    // a status update or an item-ring transition.
    if (meter->mpBTextXY[2] != nullptr) {
        meter->mpBTextXY[2]->hide();
        meter->mpBTextXY[2]->getPanePtr()->setAlpha(0);
        meter->mpBTextXY[2]->scale(0.0f, 0.0f);
    }
    for (int i = 0; i < 5; ++i) {
        CPaneMgr* textLayer = meter->mpXYText[i][2];
        if (textLayer == nullptr) {
            continue;
        }
        textLayer->hide();
        textLayer->getPanePtr()->setAlpha(0);
        textLayer->scale(0.0f, 0.0f);
        if (auto* textBox = dynamic_cast<J2DTextBox*>(textLayer->getPanePtr())) {
            // These panes remain owned and reused by the stock HUD. Do not
            // shrink their backing buffers to a one-byte empty string: later
            // scene/event updates write native labels back into them, which
            // trips Dusklight's bounded-string check ("Need 6, have 1").
            textBox->setString(64, "");
        }
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

    // Recolor the stock D-Pad without changing its artwork or geometry. Its
    // texture uses black as the transparent endpoint, so that endpoint must
    // retain zero alpha or the full rectangular texture becomes visible.
    constexpr u64 dpadTags[] = {
        MULTI_CHAR('juji_001'), MULTI_CHAR('juji_002'),
        MULTI_CHAR('juji_003'), MULTI_CHAR('juji_004'),
    };
    const bool blackPro = button_style() == ButtonStyle::BlackPro;
    for (const u64 tag : dpadTags) {
        if (J2DPane* piece = meter->mpScreen->search(tag)) {
            piece->show();
            if (J2DPicture* picture = as_picture(piece)) {
                picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                    blackPro ? JUtility::TColor(112, 117, 124, 255)
                             : JUtility::TColor(255, 255, 255, 255));
            }
        }
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
        // In Follow mode, mirror the user's Call Midna assignment. The stock
        // Items row is above the cross; move it below only while Midna is not
        // assigned to D-Pad Down.
        constexpr f32 upperRowOffsetY = 15.0f;
        constexpr f32 lowerRowOffsetY = 46.0f;
        meter->mpTextI->paneTrans(0.0f,
            midna_uses_dpad_down() ? upperRowOffsetY : lowerRowOffsetY);
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
            const char* currentLabel = text_box_string(text);
            if (currentLabel == nullptr || std::strcmp(currentLabel, label) != 0) {
                text->setString(0x40, label);
            }
        }
    }
}

void apply_hud_backing_visibility(dMeter2Draw_c* meter) {
    if (meter == nullptr || meter->mpUzu == nullptr) {
        return;
    }

    // `uzu_n` is not a standalone flourish in this archive: it contains a
    // complete, static copy of the old face-button cluster. At reduced HUD
    // scales that copy becomes visible at the stock top-right anchor. Keep the
    // authored replacement diamond and suppress this legacy composite.
    meter->mpUzu->hide();
    meter->mpUzu->setAlphaRate(0.0f);
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

// Item wheel and R-slot HUD -------------------------------------------------

J2DPane* item_wheel_z_anchor(J2DScreen* screen) {
    return screen != nullptr ? screen->search(MULTI_CHAR('r_btn_n')) : nullptr;
}

void apply_item_wheel_z_offset(Vec& pos) {
    pos.x += 5.0f;
    pos.y -= 5.0f;
}

ResTIMG const* ring_assignment_face_texture(const bool xButton) {
    const ButtonLayout layout = button_layout();
    if (layout == ButtonLayout::Universal) {
        return styled_blank_face_button_texture();
    }
    if (layout == ButtonLayout::Xbox) {
        return styled_face_button_texture(xButton ? 'Y' : 'X');
    }
    return styled_face_button_texture(xButton ? 'X' : 'Y');
}

void style_ring_assignment_face_buttons(dMenu_Ring_c* ring) {
    if (ring == nullptr || ring->mpScreen == nullptr || ring->mPlayerIsWolf) {
        return;
    }

    // These archive groups contain several decorative picture children and
    // their visible button is not consistently the first child.  Hide the
    // complete native shapes; draw_ring_z_prompt renders deterministic,
    // standalone circular replacements at these groups' authored centers.
    if (J2DPane* xButton = ring->mpScreen->search(MULTI_CHAR('x_btn_n'))) {
        xButton->hide();
    }
    if (J2DPane* yButton = ring->mpScreen->search(MULTI_CHAR('y_btn_n'))) {
        yButton->hide();
    }
}

void hide_picture_descendants(J2DPane* pane) {
    if (pane == nullptr) {
        return;
    }
    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        if (child->getTypeID() == 18) {
            child->hide();
            child->setAlpha(0);
        }
        hide_picture_descendants(child);
    }
}

void style_ring_combo_prompt(dMenu_Ring_c* ring) {
    if (ring == nullptr || ring->mpScreen == nullptr) {
        return;
    }

    // gr_n owns the bottom-right Bow & Arrow Combo/Combo OFF prompt.  Keep
    // its authored text layers and remove only the GameCube R-wedge art; the
    // custom overlay below supplies the TPHD ZL label in the same location.
    hide_picture_descendants(ring->mpScreen->search('gr_n'));
}

void destroy_ring_z_prompt(dMenu_Ring_c* ring) {
    if (s_ringZPrompt.ring != ring) {
        return;
    }

    JKR_DELETE(s_ringZPrompt.button);
    s_ringZPrompt.button = nullptr;
    JKR_DELETE(s_ringZPrompt.buttonX);
    s_ringZPrompt.buttonX = nullptr;
    JKR_DELETE(s_ringZPrompt.buttonY);
    s_ringZPrompt.buttonY = nullptr;
    JKR_DELETE(s_ringZPrompt.buttonR);
    s_ringZPrompt.buttonR = nullptr;
    s_ringZPrompt.comboZL = nullptr;
    JKR_DELETE(s_ringZPrompt.screen);
    s_ringZPrompt.screen = nullptr;
    s_ringZPrompt.ring = nullptr;
}

void create_ring_z_prompt(dMenu_Ring_c* ring) {
    destroy_ring_z_prompt(s_ringZPrompt.ring);
    if (ring == nullptr || ring->mPlayerIsWolf || ring->mpScreen == nullptr) {
        return;
    }

    style_ring_assignment_face_buttons(ring);

    J2DPane* anchor = item_wheel_z_anchor(ring->mpScreen);
    if (anchor != nullptr) {
        anchor->translate(anchor->getTranslateX() + 40.0f, anchor->getTranslateY() + 11.0f);
        anchor->hide();
    }

    J2DScreen* screen = JKR_NEW J2DScreen();
    if (screen == nullptr) {
        return;
    }
    if (!screen->setPriority("zelda_game_image.blo", 0x20000,
            dComIfGp_getMain2DArchive())) {
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

    J2DPicture* buttonX = nullptr;
    J2DPicture* buttonY = nullptr;
    J2DPicture* buttonR = nullptr;
    if (ResTIMG const* texture = ring_assignment_face_texture(true)) {
        buttonX = JKR_NEW J2DPicture(texture);
    }
    if (ResTIMG const* texture = ring_assignment_face_texture(false)) {
        buttonY = JKR_NEW J2DPicture(texture);
    }
    if (ResTIMG const* texture = styled_r_button_texture()) {
        buttonR = JKR_NEW J2DPicture(texture);
    }

    auto* comboZL = JKR_NEW J2DTextBox(MULTI_CHAR('hd_czl'),
        JGeometry::TBox2<f32>(516.0f, 384.0f, 552.0f, 411.0f), nullptr,
        "ZL", 4, HBIND_CENTER, VBIND_CENTER);
    if (comboZL != nullptr) {
        comboZL->setFont(mDoExt_getMesgFont());
        comboZL->setFontSize(16.0f, 16.0f);
        comboZL->setFontColor(JUtility::TColor(245, 245, 240, 255),
            JUtility::TColor(255, 255, 255, 255));
        screen->appendChild(comboZL);
        comboZL->hide();
    }

    style_ring_combo_prompt(ring);
    s_ringZPrompt = {
        .ring = ring,
        .screen = screen,
        .button = button,
        .buttonX = buttonX,
        .buttonY = buttonY,
        .buttonR = buttonR,
        .comboZL = comboZL,
    };
}

void draw_ring_z_prompt(dMenu_Ring_c* ring) {
    if (s_ringZPrompt.ring != ring || s_ringZPrompt.screen == nullptr ||
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

    // The native combo group still owns the localized message and controls
    // its visibility.  Draw the ZL badge independently so the original
    // GameCube wedge can remain hidden without loading another layout screen.
    const bool comboVisible = ring->mpTextParent[4] != nullptr &&
        ring->mpTextParent[4]->isVisible();
    if (s_ringZPrompt.comboZL != nullptr) {
        s_ringZPrompt.comboZL->setAlpha(static_cast<u8>(
            std::clamp(ring->mAlphaRate, 0.0f, 1.0f) * 255.0f));
        if (comboVisible) {
            s_ringZPrompt.comboZL->show();
        } else {
            s_ringZPrompt.comboZL->hide();
        }
    }

    if (s_ringZPrompt.button != nullptr) {
        s_ringZPrompt.button->hide();
        s_ringZPrompt.button->setAlphaRate(0.0f);
        s_ringZPrompt.button->scale(0.0f, 0.0f);
        if (J2DPane* pane = s_ringZPrompt.button->getPanePtr()) {
            hide_pane_tree(pane);
            pane->setAlpha(0);
            pane->scale(0.0f, 0.0f);
        }
    }
    s_ringZPrompt.screen->draw(0.0f, 0.0f, dComIfGp_getCurrentGrafPort());

    // J2D ring coordinates are rendered at roughly twice the final window
    // size.  A 52-unit square made these prompts about 100 pixels wide;
    // 24 units matches the authored face-button prompt scale.
    constexpr f32 kFaceButtonSize = 24.0f;
    const auto drawFaceButton = [&](J2DPicture* picture, const u64 anchorTag,
                                    const f32 offsetX) {
        J2DPane* faceAnchor = ring->mpScreen->search(anchorTag);
        if (picture == nullptr || faceAnchor == nullptr) {
            return;
        }

        CPaneMgr facePaneMgr;
        Vec facePos = facePaneMgr.getGlobalVtxCenter(faceAnchor, true, 0);
        facePos.x += ring->mCenterPosX;
        facePos.y += ring->mCenterPosY;
        facePos.x += offsetX;
        picture->setAlpha(static_cast<u8>(std::clamp(ring->mAlphaRate, 0.0f, 1.0f) * 255.0f));
        picture->draw(facePos.x - kFaceButtonSize * 0.5f,
            facePos.y - kFaceButtonSize * 0.5f,
            kFaceButtonSize, kFaceButtonSize, false, false, false);
    };
    drawFaceButton(s_ringZPrompt.buttonX, MULTI_CHAR('x_btn_n'), -4.0f);
    drawFaceButton(s_ringZPrompt.buttonY, MULTI_CHAR('y_btn_n'), 0.0f);

    // The third assignment target is R.  Do not reuse the helper screen's
    // GameCube Z pane here: keeping that pane collapsed is what prevents the
    // detached white orb and standalone Z from returning.
    if (s_ringZPrompt.buttonR != nullptr) {
        J2DPane* rAnchor = ring->mpScreen->search(MULTI_CHAR('r_btn_n'));
        if (rAnchor != nullptr) {
            CPaneMgr rPaneMgr;
            Vec rPos = rPaneMgr.getGlobalVtxCenter(rAnchor, true, 0);
            rPos.x += ring->mCenterPosX;
            rPos.y += ring->mCenterPosY;
            // Leave a small visual gap after X while correcting the inherited
            // vertical shift from the native third-slot pane.
            rPos.x += 12.0f;
            rPos.y -= 10.0f;
            constexpr f32 kRButtonHeight = 20.0f;
            f32 rButtonWidth = 38.0f;
            if (JUTTexture* texture = s_ringZPrompt.buttonR->getTexture(0)) {
                if (ResTIMG const* image = texture->getTexInfo();
                    image != nullptr && image->height != 0) {
                    rButtonWidth = kRButtonHeight *
                        static_cast<f32>(image->width) /
                        static_cast<f32>(image->height);
                }
            }
            s_ringZPrompt.buttonR->setAlpha(static_cast<u8>(
                std::clamp(ring->mAlphaRate, 0.0f, 1.0f) * 255.0f));
            s_ringZPrompt.buttonR->draw(rPos.x - rButtonWidth * 0.5f,
                rPos.y - kRButtonHeight * 0.5f,
                rButtonWidth, kRButtonHeight, false, false, false);
        }
    }
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

bool is_hud_bow_combo(const u8 itemNo) {
    return itemNo == dItemNo_BOMB_ARROW_e || itemNo == dItemNo_HAWK_ARROW_e;
}

void arrange_hud_bow_combo_layers(const u8 itemNo, J2DPicture* primaryPicture,
    J2DPicture* secondaryPicture) {
    if (primaryPicture == nullptr || secondaryPicture == nullptr) {
        return;
    }

    J2DPane* primaryParent = primaryPicture->getParentPane();
    if (primaryParent == nullptr) {
        return;
    }

    // The stock loader makes the second picture a child of the first, which
    // forces the bow layer to draw over the attachment. Make both pictures
    // siblings and insert the bow immediately before the attachment instead.
    // This changes only pane order; the live texture buffers remain untouched.
    if (secondaryPicture->getParentPane() == primaryPicture) {
        J2DPane* commonParent = primaryPicture->getParentPane();
        auto* primaryTree =
            const_cast<JSUTree<J2DPane>*>(primaryPicture->getPaneTree());
        auto* secondaryTree =
            const_cast<JSUTree<J2DPane>*>(secondaryPicture->getPaneTree());
        if (primaryTree->removeChild(secondaryTree)) {
            commonParent->insertChild(primaryPicture, secondaryPicture);
        }
    }

    if (secondaryPicture->getParentPane() != primaryPicture->getParentPane()) {
        return;
    }

    // As siblings, the two pictures no longer inherit one another's geometry.
    // Keep their centers and authored HUD scale identical, then reduce only
    // the attachment by 40 percent for a combo.
    const f32 centerX = primaryPicture->getTranslateX();
    const f32 centerY = primaryPicture->getTranslateY();
    secondaryPicture->resize(primaryPicture->getWidth(), primaryPicture->getHeight());
    // Both pictures now share the same parent and base position. Adjusting by
    // their translation delta avoids J2DPane::move re-applying the parent's
    // non-zero layout origin, which displaced the bow to the left at 75% HUD
    // scale.
    secondaryPicture->add(centerX - secondaryPicture->getTranslateX(),
        centerY - secondaryPicture->getTranslateY());
    secondaryPicture->rotate(primaryPicture->getWidth() * 0.5f,
        primaryPicture->getHeight() * 0.5f, ROTATE_Z,
        primaryPicture->getRotateZ());

    const f32 authoredScaleX = primaryPicture->getScaleX();
    const f32 authoredScaleY = primaryPicture->getScaleY();
    secondaryPicture->scale(authoredScaleX, authoredScaleY);
    if (is_hud_bow_combo(itemNo)) {
        primaryPicture->scale(authoredScaleX * 0.6f, authoredScaleY * 0.6f);
    }
}

void fix_xy_hud_bow_combo_layering(dMeter2Draw_c* meter) {
    if (meter == nullptr) {
        return;
    }

    for (int i = 0; i < 2; ++i) {
        if (meter->mpItemXY[i] == nullptr || meter->mpItemXYPane[i] == nullptr) {
            continue;
        }
        const u8 itemNo = dComIfGp_getSelectItem(i);
        arrange_hud_bow_combo_layers(itemNo,
            static_cast<J2DPicture*>(meter->mpItemXY[i]->getPanePtr()),
            meter->mpItemXYPane[i]);
    }

    // The R-slot layout is rebuilt every frame, so its two combo pictures
    // need the same final alignment after layout_z_hud_item() has run. Doing
    // this here also keeps the result stable when the HUD scale changes.
    if (meter->mpItemR != nullptr && meter->mpItemXYPane[2] != nullptr) {
        const u8 itemNo = dComIfGp_getSelectItem(kZItemSlot);
        arrange_hud_bow_combo_layers(itemNo,
            static_cast<J2DPicture*>(meter->mpItemR->getPanePtr()),
            meter->mpItemXYPane[2]);
    }
}

void change_z_hud_item_texture(dMeter2Draw_c* meter, const u8 itemNo) {
    if (s_zHudItemMeter != meter) {
        s_zHudItemMeter = meter;
        s_zHudLastItem = dItemNo_NONE_e;
    }

    const u8 textureItem = hud_texture_item(itemNo);
    // Scene changes destroy and recreate the meter's J2D pictures. The meter
    // object itself can be allocated at the same address, so pointer identity
    // plus an unchanged item number is not sufficient to prove that the new
    // R picture is still attached to our persistent texture buffer.
    auto* primaryPicture =
        static_cast<J2DPicture*>(meter->mpItemR->getPanePtr());
    JUTTexture* primaryTexture =
        primaryPicture != nullptr ? primaryPicture->getTexture(0) : nullptr;
    const bool textureStillAttached =
        primaryTexture != nullptr &&
        primaryTexture->getTexInfo() == z_hud_item_tex(s_zHudItemTexPage, 0);
    if (s_zHudLastItem == textureItem && textureStillAttached) {
        return;
    }

    s_zHudItemTexPage ^= 1;
    ResTIMG* primary = z_hud_item_tex(s_zHudItemTexPage, 0);
    ResTIMG* secondary = z_hud_item_tex(s_zHudItemTexPage, 1);
    const s32 textureCount =
        dMeter2Info_readItemTexture(textureItem, primary,
            primaryPicture, secondary,
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
    arrange_hud_bow_combo_layers(itemNo,
        static_cast<J2DPicture*>(meter->mpItemR->getPanePtr()), meter->mpItemXYPane[2]);
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

    const JGeometry::TBox2<f32>& itemBounds = meter->mpItemR->getPanePtr()->getGlbBounds();
    const int digitCount = itemNum >= 100 ? 3 : 2;
    const f32 countX = itemBounds.f.x - digitSize * digitCount;
    // Android's high-density HUD transform leaves the custom digits visibly
    // farther below the R-item artwork than the desktop renderer does. Keep
    // the desktop placement intact and compensate in the same logical J2D
    // coordinate space so the correction scales with Android resolutions.
#if defined(__ANDROID__)
    constexpr f32 platformAmmoOffsetY = -8.0f;
#else
    constexpr f32 platformAmmoOffsetY = 0.0f;
#endif
    const f32 countY = itemBounds.f.y - digitSize * 0.65f + platformAmmoOffsetY;
    const u8 alpha = clamp_hud_alpha(itemAlphaRate * 255.0f);

    for (int i = 0; i < 3; ++i) {
        if (i == 2 && itemNum < 100) {
            continue;
        }
        s_zItemNumTex[i]->setAlpha(alpha);
        s_zItemNumTex[i]->draw(countX + digitSize * i, countY,
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
    // visibility path. Attach Midna to the D-Pad group and mirror the user's
    // Call Midna direction when Follow Dusklight Bindings is active.
    J2DPane* anchorPane = meter->mpScreen->search(MULTI_CHAR('juji_n'));
    constexpr f32 positionX = 8.0f;
    const f32 positionY = midna_uses_dpad_down() ? 25.0f : -21.0f;

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

bool menu_overlay_hides_rupees() {
    // Unlike the Collection object's resource lifetime, these flags remain
    // asserted for the full pause menu and every nested Save dialog.
    return dMeter2Info_getWindowStatus() != 0 ||
        dMeter2Info_getPauseStatus() != 0 || dComIfGp_isPauseFlag();
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

void after_pad_read(ModContext*, void*, void*, void*) {
    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);

    // Fixed TPHD bindings refer to the controller's physical ZL trigger, not
    // the emulated GameCube L input selected in Dusklight's profile. Reading
    // the SDL trigger directly keeps L and ZL distinct even when both feed
    // GameCube-style trigger state elsewhere in the game.
    const bool fixedTphdBindings =
        controller_compatibility() == ControllerCompatibility::FixedTphd;
    bool physicalZlHeld = false;
    if (fixedTphdBindings) {
        const PADSignedNativeAxis nativeAxis = PADGetNativeAxisPulled(PAD_1);
        physicalZlHeld = nativeAxis.nativeAxis == kSdlLeftTriggerAxis &&
            nativeAxis.sign == AXIS_SIGN_POSITIVE;
    }
    s_fixedZlTrig = physicalZlHeld && !s_fixedZlHeld;
    s_fixedZlHeld = physicalZlHeld;

    // Dusklight exposes both physical shoulder controls through GameCube L.
    // In the fixed layout, reserve physical ZL for TPHD lock/combo actions and
    // remove only that press from the base game's L/shield state. Physical L
    // remains untouched and continues to control the shield normally.
    if (fixedTphdBindings && physicalZlHeld) {
        pad.mButtonFlags &= ~PAD_TRIGGER_L;
        pad.mPressedButtonFlags &= ~PAD_TRIGGER_L;
        pad.mTriggerLeft = 0.0f;
        pad.mHoldLockL = false;
        pad.mTrigLockL = false;
    }

    if (z_item_menu_or_pause_context() ||
        !fixedTphdBindings)
    {
        s_dpadMidnaHeld = false;
        s_dpadMidnaTrig = false;
        return;
    }

    s_dpadMidnaHeld = (pad.mButtonFlags & PAD_BUTTON_UP) != 0;
    s_dpadMidnaTrig = (pad.mPressedButtonFlags & PAD_BUTTON_UP) != 0;
}

void after_set_stick_data(ModContext*, void* args, void*, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    if (link == nullptr || z_item_menu_or_pause_context()) {
        return;
    }

    // A dedicated Dusklight action binding takes precedence over the third
    // item if both resolve to the same physical control.
    if (midna_action_triggered()) {
        link->mItemTrigger &= ~daAlink_c::BTN_Z;
        return;
    }

    // TPHD's third slot uses logical R in either compatibility mode. Dusklight
    // remains responsible for deciding which physical control produces R.
    // Apply the translation only after the game's input snapshot is built so
    // the shared controller profile and pad state remain untouched.
    if (mDoCPd_c::getTrigR(PAD_1)) {
        link->mItemTrigger |= daAlink_c::BTN_Z;
    }
    if (mDoCPd_c::getHoldR(PAD_1)) {
        link->mItemButton |= daAlink_c::BTN_Z;
    }
}

void after_collect_create(ModContext*, void* args, void*, void*) {
    s_activeCollectMenu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    s_collectTitleFrame = nullptr;
    s_collectSaveFrame = nullptr;
    s_collectOptionsFrame = nullptr;
    s_collectTopRule = nullptr;
    s_collectBottomRule = nullptr;
    s_collectTopRuleInner = nullptr;
    s_collectBottomRuleInner = nullptr;
    s_collectTitleLabel = nullptr;
    s_collectRailDiagnosticsLogged = false;
    s_collectCursorDiagnosticsLogged = false;
    s_collectLayoutReady = false;
    apply_collect_menu_button_layout(s_activeCollectMenu);
    // The native desktop widescreen pass can still rewrite the title
    // container once immediately after creation. Keep the replacement hidden
    // until before_collect_draw has anchored its final rendered bounds, so the
    // title never flashes at the authored GameCube position for one frame.
    if (s_collectTitleFrame != nullptr) {
        s_collectTitleFrame->hide();
    }
    if (s_collectTitleLabel != nullptr) {
        s_collectTitleLabel->hide();
    }
}

void after_letter_create(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Letter_c*>(args, 0);
    if (menu != nullptr) {
        apply_collection_submenu_button_layout(menu->mpIconScreen,
            menu->mpArchive, menu->mpString);
    }
}

void after_fishing_create(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Fishing_c*>(args, 0);
    if (menu != nullptr) {
        apply_collection_submenu_button_layout(menu->mpIconScreen,
            menu->mpArchive, menu->mpString);
    }
}

void after_skill_create(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Skill_c*>(args, 0);
    if (menu != nullptr) {
        apply_collection_submenu_button_layout(menu->mpIconScreen,
            menu->mpArchive, menu->mpString);
    }
}

void after_insect_create(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Insect_c*>(args, 0);
    if (menu != nullptr) {
        apply_collection_submenu_button_layout(menu->mpIconScreen,
            menu->mpArchive, menu->mpString);
    }
}

void after_collect_move(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    refresh_collect_menu_frames(menu);
    if (menu != nullptr && menu->mpScreen != nullptr &&
        !s_collectRailDiagnosticsLogged) {
        log_collect_rail_candidates(menu->mpScreen);
        s_collectRailDiagnosticsLogged = true;
    }
}

#if defined(_WIN32)
void after_collect_wide(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    if (menu == nullptr || menu != s_activeCollectMenu ||
        menu->mpScreen == nullptr) {
        return;
    }

    // menuCollectWide() is Windows' final layout reset before mpScreen draws.
    // Anchor and reveal the replacement here so neither initial entry nor a
    // return from Letters/Fish Journal can render its pre-widescreen position.
    anchor_collect_title_panel();
    if (s_collectTitleFrame != nullptr) {
        s_collectTitleFrame->show();
    }
    if (s_collectTitleLabel != nullptr) {
        s_collectTitleLabel->show();
    }
    s_collectLayoutReady = true;
}
#endif

HookAction before_collect_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    if (menu != nullptr && menu->mpScreen != nullptr) {
        // Native Collection animation restores the stone rails after _move.
        // Reassert the exact replacement immediately before it is rendered.
        style_collect_edge_rules(menu->mpScreen);
        refresh_collect_prompt_width(menu->getIconScreen());
#if defined(_WIN32)
        // The post-menuCollectWide hook below will reveal these only after the
        // final Windows transform has been applied for this exact frame.
        if (s_collectTitleFrame != nullptr) {
            s_collectTitleFrame->hide();
        }
        if (s_collectTitleLabel != nullptr) {
            s_collectTitleLabel->hide();
        }
#else
        anchor_collect_title_panel();
        if (!s_collectLayoutReady) {
            if (s_collectTitleFrame != nullptr) {
                s_collectTitleFrame->show();
            }
            if (s_collectTitleLabel != nullptr) {
                s_collectTitleLabel->show();
            }
            s_collectLayoutReady = true;
        }
#endif
    }
    return HOOK_CONTINUE;
}

HookAction before_letter_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Letter_c*>(args, 0);
    refresh_collect_prompt_width(menu != nullptr ? menu->mpIconScreen : nullptr);
    return HOOK_CONTINUE;
}

HookAction before_fishing_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Fishing_c*>(args, 0);
    refresh_collect_prompt_width(menu != nullptr ? menu->mpIconScreen : nullptr);
    return HOOK_CONTINUE;
}

HookAction before_skill_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Skill_c*>(args, 0);
    refresh_collect_prompt_width(menu != nullptr ? menu->mpIconScreen : nullptr);
    return HOOK_CONTINUE;
}

HookAction before_insect_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Insect_c*>(args, 0);
    refresh_collect_prompt_width(menu != nullptr ? menu->mpIconScreen : nullptr);
    return HOOK_CONTINUE;
}

HookAction before_select_cursor_draw(ModContext*, void* args, void*, void*) {
    auto* cursor = mods::arg<dSelect_cursor_c*>(args, 0);
    if (s_activeCollectMenu != nullptr &&
        cursor == s_activeCollectMenu->mpDrawCursor) {
        // This is the final point before the cursor's four panes render;
        // Collection's animation has already finished moving them here.
        position_collect_footer_cursor(s_activeCollectMenu);
    }
    return HOOK_CONTINUE;
}

void after_select_cursor_update(ModContext*, void* args, void*, void*) {
    auto* cursor = mods::arg<dSelect_cursor_c*>(args, 0);
    if (s_activeCollectMenu != nullptr &&
        cursor == s_activeCollectMenu->mpDrawCursor) {
        // update() is the last operation that applies the cursor animation
        // and rewrites its corner transforms. Correct the footer geometry
        // only after that work is complete.
        position_collect_footer_cursor(s_activeCollectMenu);
    }
}

HookAction before_fmap_move(ModContext*, void*, void*, void*) {
    // The map's Portals action is still wired to GameCube Z internally.
    // Translate the displayed logical R only while this map processes input.
    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);
    if ((pad.mButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mButtonFlags |= PAD_TRIGGER_Z;
    }
    if ((pad.mPressedButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mPressedButtonFlags |= PAD_TRIGGER_Z;
    }
    return HOOK_CONTINUE;
}

HookAction before_fmap_draw(ModContext*, void* args, void*, void*) {
    // Prompt strings are rewritten as map zoom state changes, so mirror the
    // current values immediately before every field-map draw.
    auto* map = mods::arg<dMenu_Fmap_c*>(args, 0);
    if (map != nullptr) {
        apply_fmap_background(map->mpDraw2DBack);
        apply_fmap_top(map->mpDraw2DTop);
    }
    return HOOK_CONTINUE;
}

HookAction before_dmap_draw(ModContext*, void* args, void*, void*) {
    auto* map = mods::arg<dMenu_Dmap_c*>(args, 0);
    if (map != nullptr) {
        apply_dmap_hd_layout(map->mpDrawBg);
    }
    return HOOK_CONTINUE;
}

HookAction before_collect_delete(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Collect2D_c*>(args, 0);
    if (s_activeCollectMenu == menu) {
        s_activeCollectMenu = nullptr;
        s_collectTitleFrame = nullptr;
        s_collectSaveFrame = nullptr;
        s_collectOptionsFrame = nullptr;
        s_collectTopRule = nullptr;
        s_collectBottomRule = nullptr;
        s_collectTopRuleInner = nullptr;
        s_collectBottomRuleInner = nullptr;
        s_collectTitleLabel = nullptr;
    }
    return HOOK_CONTINUE;
}

void after_option_create(ModContext*, void* args, void*, void*) {
    apply_option_hd_style(mods::arg<dMenu_Option_c*>(args, 0));
}

void after_brightness_check_screen_set(ModContext*, void* args, void*, void*) {
    style_brightness_check_screen(mods::arg<dBrightCheck_c*>(args, 0));
}

HookAction before_option_move(ModContext*, void*, void*, void*) {
    // The original display submenu is bound to GameCube Z. The HD layout uses
    // logical R, so translate R only while the Options menu is
    // processing input; gameplay's separate R-item mapping remains untouched.
    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);
    if ((pad.mButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mButtonFlags |= PAD_TRIGGER_Z;
    }
    if ((pad.mPressedButtonFlags & PAD_TRIGGER_R) != 0) {
        pad.mPressedButtonFlags |= PAD_TRIGGER_Z;
    }
    return HOOK_CONTINUE;
}

HookAction before_option_draw(ModContext*, void* args, void*, void*) {
    // The native option animation recalculates row, arrow, and cursor
    // positions during movement. Reassert the HD presentation immediately
    // before drawing without touching its values or state machine.
    apply_option_hd_style(mods::arg<dMenu_Option_c*>(args, 0));
    ++s_descenderCorrectionDrawDepth;
    return HOOK_CONTINUE;
}

void after_option_draw(ModContext*, void*, void*, void*) {
    if (s_descenderCorrectionDrawDepth > 0) {
        --s_descenderCorrectionDrawDepth;
    }
}

HookAction before_brightness_check_draw(ModContext*, void*, void*, void*) {
    ++s_descenderCorrectionDrawDepth;
    return HOOK_CONTINUE;
}

void after_brightness_check_draw(ModContext*, void*, void*, void*) {
    if (s_descenderCorrectionDrawDepth > 0) {
        --s_descenderCorrectionDrawDepth;
    }
}

HookAction before_res_font_draw_char(ModContext*, void* args, void*, void*) {
    if (s_descenderCorrectionDrawDepth > 0) {
        const int glyph = mods::arg<int>(args, 5);
        if (glyph == 'g') {
            mods::arg_ref<f32>(args, 2) += 2.0f;
        } else if (glyph == 'p') {
            mods::arg_ref<f32>(args, 2) += 1.0f;
        }
    }
    return HOOK_CONTINUE;
}

HookAction before_option_draw_arrows(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_Option_c*>(args, 0);
    if (menu == nullptr || menu->mpMeterHaihai == nullptr ||
        menu->mpShadowScreen == nullptr) {
        return HOOK_SKIP_ORIGINAL;
    }

    const u8 selected = menu->getSelectType();
    constexpr u64 frameTags[] = {
        MULTI_CHAR('hd_orf0'), MULTI_CHAR('hd_orf1'), MULTI_CHAR('hd_orf2'),
    };
    J2DPane* frame = selected < 3 ?
        menu->mpShadowScreen->search(frameTags[selected]) : nullptr;
    if (frame != nullptr && menu->field_0x3f3 == 5 &&
        menu->field_0x3ef != dMenu_Option_c::PROC_CONFIRM_OPEN_MOVE_e &&
        menu->field_0x3ef != dMenu_Option_c::PROC_CONFIRM_MOVE_MOVE_e &&
        menu->field_0x3ef != dMenu_Option_c::PROC_CONFIRM_SELECT_MOVE_e &&
        menu->field_0x3ef != dMenu_Option_c::PROC_CONFIRM_CLOSE_MOVE_e) {
        const auto& bounds = frame->getGlbBounds();
        constexpr f32 arrowGap = 16.0f;
        const f32 left = bounds.i.x - arrowGap;
        const f32 right = bounds.f.x + arrowGap;
        const f32 centerY = (bounds.i.y + bounds.f.y) * 0.5f;
        menu->mpMeterHaihai->_execute(0);
        menu->mpMeterHaihai->drawHaihai(
            dMeterHaihai_c::DIR_LEFT_e | dMeterHaihai_c::DIR_RIGHT_e,
            (left + right) * 0.5f, centerY, right - left, 0.0f);
    }
    return HOOK_SKIP_ORIGINAL;
}

void after_file_select_create(ModContext*, void* args, void*, void*) {
    s_activeFileSelect = mods::arg<dFile_select_c*>(args, 0);
    s_fileSelectYesNoLayoutReady = false;
    apply_file_select_hd_style(s_activeFileSelect);
}

void after_file_select_move(ModContext*, void* args, void*, void*) {
    position_file_select_prompts(mods::arg<dFile_select_c*>(args, 0));
}

HookAction before_file_select_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dFile_select_c*>(args, 0);
    // Dusklight's Reset command reconstructs several archive panes without
    // constructing a new dFile_select_c. Reapply only idempotent styling here
    // so the reset path cannot restore native lines, panels, or row artwork.
    replace_file_select_background(menu);
    replace_file_select_prompt(menu->mAbtnPane,
        menu_face_button_texture(true), MULTI_CHAR('hd_fsab'));
    replace_file_select_prompt(menu->mBbtnPane,
        menu_face_button_texture(false), MULTI_CHAR('hd_fsbb'));
    remove_file_select_stonework(menu);
    remove_file_select_decorative_bands(menu);
    simplify_file_select_rows(menu);
    simplify_file_select_numbers(menu);
    clear_file_select_prompt_panel(menu->mModoruTxtPane);
    clear_file_select_prompt_panel(menu->mKetteiTxtPane);
    add_file_select_title_rules(menu);
    add_file_select_back_label(menu);
    add_file_select_prompt_flourish(menu);
    style_file_select_dynamic_text(menu);
    style_file_select_metadata(menu);
    position_file_select_prompts(menu);
    style_file_select_action_buttons(menu);
    update_file_select_row_selection(menu);
    style_copy_destination_screen(menu);
    position_file_select_cursor(menu);
    // Copy and Erase confirmations share File Selection's native Yes/No
    // screen. Restore the cached local translations after the native choice
    // animation has run so neither button can slide during selection changes.
    style_copy_destination_yes_no(menu,
        copy_destination_confirm_state(menu));
    return HOOK_CONTINUE;
}

HookAction before_file_select_main_draw(ModContext*, void*, void*, void*) {
    // File Selection queues this screen for a later draw. Keep the descender
    // correction active while that queued screen renders so titles such as
    // "Quest Log" use the same baseline as the other updated menus.
    ++s_descenderCorrectionDrawDepth;
    return HOOK_CONTINUE;
}

void after_file_select_main_draw(ModContext*, void*, void*, void*) {
    if (s_descenderCorrectionDrawDepth > 0) {
        --s_descenderCorrectionDrawDepth;
    }
}

void after_save_menu_screen_set(ModContext*, void* args, void*, void*) {
    s_activeSaveMenu = mods::arg<dMenu_save_c*>(args, 0);
    apply_save_menu_hd_style(s_activeSaveMenu);
}

HookAction before_save_menu_delete(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_save_c*>(args, 0);
    if (s_activeSaveMenu == menu) {
        s_activeSaveMenu = nullptr;
    }
    return HOOK_CONTINUE;
}

HookAction before_save_menu_draw(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_save_c*>(args, 0);
    if (menu != nullptr && menu->mSaveSel.Scr != nullptr) {
        // Save-menu animations continue to drive the original selection and
        // prompt layers. Reassert only the visual overrides before queuing the
        // screen for draw; save state and input remain entirely native.
        simplify_save_menu_rows(menu);
        hide_save_menu_stonework(menu);
        update_save_menu_row_selection(menu);
        style_save_select_title(menu);
        style_save_menu_prompts(menu);
        add_save_menu_fixed_prompts(menu);
        style_save_menu_yes_no_buttons(menu);
    }
    return HOOK_CONTINUE;
}

void after_save_menu_wide(ModContext*, void* args, void*, void*) {
    auto* menu = mods::arg<dMenu_save_c*>(args, 0);
    if (menu == nullptr) {
        return;
    }
    // menuSaveWide applies the save layout's aspect transform immediately
    // before it is queued for drawing. Position the shared HD prompt cluster
    // and cursor afterward so they use the same final coordinate space as
    // File Selection.
    position_save_menu_prompts(menu);
    style_save_menu_metadata(menu);
    style_save_select_title(menu);
    update_save_menu_row_selection(menu);
    if (menu->mSelIcon == nullptr) {
        return;
    }

    J2DPane* cursorTarget = nullptr;
    if (menu->mHeaderTxtType == 1 && menu->mYesNoCursor < 2) {
        constexpr u64 yesNoFrameTags[] = {
            MULTI_CHAR('hd_sno'), MULTI_CHAR('hd_syes'),
        };
        J2DPane* group = menu->mpNoYes[menu->mYesNoCursor] != nullptr ?
            menu->mpNoYes[menu->mYesNoCursor]->getPanePtr() : nullptr;
        cursorTarget = group != nullptr ?
            group->search(yesNoFrameTags[menu->mYesNoCursor]) : nullptr;
    } else {
        const std::size_t selected = menu->mSelectedFile;
        cursorTarget = selected < 3 ? save_menu_row_picture(menu, selected) : nullptr;
    }

    if (cursorTarget != nullptr) {
        if (menu->mHeaderTxtType == 1) {
            // The stock cursor hugs the exact target bounds. TPHD's compact
            // action buttons instead leave the corner ornaments clearly
            // outside the frame, as on File Selection's Start button.
            position_cursor_outside_frame(menu->mSelIcon, cursorTarget);
        } else {
            const auto& bounds = cursorTarget->getGlbBounds();
            menu->mSelIcon->setPos((bounds.i.x + bounds.f.x) * 0.5f,
                (bounds.i.y + bounds.f.y) * 0.5f, cursorTarget, true);
        }
    }
}

HookAction before_save_dlst_draw(ModContext*, void* args, void*, void*) {
    auto* drawList = mods::arg<dDlst_MenuSave_c*>(args, 0);
    if (s_activeSaveMenu != nullptr &&
        drawList == &s_activeSaveMenu->mSaveSel) {
        // This is the final call before J2DScreen::draw, after all save-menu
        // animation and widescreen transforms have updated global bounds.
        replace_save_menu_background(s_activeSaveMenu);
        simplify_save_menu_rows(s_activeSaveMenu);
        hide_save_menu_stonework(s_activeSaveMenu);
        update_save_menu_row_selection(s_activeSaveMenu);
        style_save_menu_prompts(s_activeSaveMenu);
        position_save_menu_prompts(s_activeSaveMenu);
        style_save_menu_yes_no_buttons(s_activeSaveMenu);
    }
    return HOOK_CONTINUE;
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

HookAction before_menu_ring_delete(ModContext*, void*, void*, void*) {
    destroy_ring_z_prompt(s_ringZPrompt.ring);
    return HOOK_CONTINUE;
}

void after_ring_draw(ModContext*, void* args, void*, void*) {
    auto* ring = mods::arg<dMenu_Ring_c*>(args, 0);
    draw_ring_z_prompt(ring);
}

HookAction before_item_explain_draw(ModContext*, void* args, void*, void*) {
    apply_item_explain_button_layout(mods::arg<dMenu_ItemExplain_c*>(args, 0));
    return HOOK_CONTINUE;
}

void after_ring_set_mix_message(ModContext*, void* args, void*, void*) {
    style_ring_combo_prompt(mods::arg<dMenu_Ring_c*>(args, 0));
}

void hide_legacy_overlay_z(dMeterButton_c* buttons) {
    if (buttons == nullptr) {
        return;
    }

    // Dusklight maps the third item assignment to R.  The independent
    // emphasis-button overlay can otherwise recreate its stock Z glow and
    // glyph after the item HUD and ring have already been styled, so collapse
    // the complete Z group at both update and draw boundaries.
    if (buttons->mpButtonZ != nullptr) {
        buttons->mpButtonZ->hide();
        buttons->mpButtonZ->setAlphaRate(0.0f);
        buttons->mpButtonZ->scale(0.0f, 0.0f);
        if (J2DPane* pane = buttons->mpButtonZ->getPanePtr()) {
            pane->hide();
            pane->setAlpha(0);
            pane->scale(0.0f, 0.0f);
        }
    }
    if (buttons->mpButtonScreen == nullptr) {
        return;
    }

    constexpr u64 overlayZPanes[] = {
        MULTI_CHAR('zbtn_n'),
        MULTI_CHAR('zbtn'),
        MULTI_CHAR('z_btnl'),
        MULTI_CHAR('z_btn_t'),
    };
    for (const u64 tag : overlayZPanes) {
        if (J2DPane* pane = buttons->mpButtonScreen->search(tag)) {
            pane->hide();
            pane->setAlpha(0);
            pane->scale(0.0f, 0.0f);
        }
    }
}

void capture_context_r_pictures(J2DPane* pane, J2DPicture* buttonBase) {
    if (pane == nullptr) {
        return;
    }

    if (pane != buttonBase && as_picture(pane) != nullptr &&
        s_contextRPictureStateCount < s_contextRPictureStates.size())
    {
        s_contextRPictureStates[s_contextRPictureStateCount++] = {
            pane, pane->isVisible(),
        };
    }

    for (J2DPane* child = pane->getFirstChildPane(); child != nullptr;
         child = child->getNextChildPane())
    {
        capture_context_r_pictures(child, buttonBase);
    }
}

void apply_context_button_layout(dMeterButton_c* buttons) {
    if (buttons == nullptr || buttons->mpButtonScreen == nullptr) {
        return;
    }

    // Context actions (Open, Let go, Pick up, Speak, and so on) are drawn by
    // a separate emphasis-button layout instead of the regular meter HUD.
    // Keep its A picture on the same layout/style path as every other confirm
    // prompt: Nintendo shows A, Xbox shows B, and Universal is blank.
    ResTIMG const* actionTexture = menu_face_button_texture(true);
    set_menu_face_button_texture(
        buttons->mpButtonScreen, MULTI_CHAR('a_btn1'), actionTexture);

    constexpr u64 actionPictures[] = {
        MULTI_CHAR('a_btn1'),
    };
    for (const u64 tag : actionPictures) {
        if (J2DPicture* picture =
                as_picture(buttons->mpButtonScreen->search(tag)))
        {
            picture->setBlackWhite(JUtility::TColor(0, 0, 0, 0),
                JUtility::TColor(255, 255, 255, 255));
            picture->setCornerColor(
                JUtility::TColor(255, 255, 255, 255));
        }
    }

    // The emphasis layout builds its stock green A from three stacked panes.
    // Our replacement is already a complete button, so the old glow and
    // separate letter would otherwise cover it and make the swap appear to
    // have failed.  Keep the animation on the parent group and remove only
    // those redundant visual layers at the final draw boundary.
    constexpr u64 stockActionLayers[] = {
        MULTI_CHAR('a_btn_l1'),
        MULTI_CHAR('a_btn_t'),
    };
    for (const u64 tag : stockActionLayers) {
        if (J2DPane* pane = buttons->mpButtonScreen->search(tag)) {
            pane->hide();
            pane->setAlpha(0);
        }
    }

    // The boomerang's lock action now uses ZL, while the same context layout
    // still serves legitimate R prompts elsewhere. Preserve the archive's R
    // artwork and replace it only while Link is holding the boomerang ready.
    J2DPicture* rButton = as_picture(
        buttons->mpButtonScreen->search(MULTI_CHAR('r_btn_b')));
    if (s_contextRButtonScreen != buttons->mpButtonScreen) {
        s_contextRButtonScreen = buttons->mpButtonScreen;
        s_contextRButtonTexture =
            rButton != nullptr && rButton->getTexture(0) != nullptr ?
                rButton->getTexture(0)->getTexInfo() : nullptr;
        s_contextRButtonUsesZl = false;
        s_contextRPictureStateCount = 0;
    }

    daAlink_c* link = daAlink_getAlinkActorClass();
    const bool boomerangLock =
        link != nullptr && link->checkBoomerangReadyAnime();
    if (rButton != nullptr && boomerangLock) {
        if (!s_contextRButtonUsesZl && buttons->mpButtonR != nullptr) {
            s_contextRPictureStateCount = 0;
            capture_context_r_pictures(
                buttons->mpButtonR->getPanePtr(), rButton);
        }
        if (ResTIMG const* zlTexture = styled_zl_button_texture()) {
            set_menu_face_button_texture(
                buttons->mpButtonScreen, MULTI_CHAR('r_btn_b'), zlTexture);
            set_neutral_picture_colors(rButton);
            s_contextRButtonUsesZl = true;
        }
        for (std::size_t index = 0;
             index < s_contextRPictureStateCount; ++index)
        {
            if (s_contextRPictureStates[index].pane != nullptr) {
                s_contextRPictureStates[index].pane->hide();
            }
        }
    } else if (s_contextRButtonUsesZl) {
        set_menu_face_button_texture(buttons->mpButtonScreen,
            MULTI_CHAR('r_btn_b'), s_contextRButtonTexture);
        for (std::size_t index = 0;
             index < s_contextRPictureStateCount; ++index)
        {
            ContextRPictureState& state = s_contextRPictureStates[index];
            if (state.pane != nullptr) {
                state.visible ? state.pane->show() : state.pane->hide();
            }
        }
        s_contextRPictureStateCount = 0;
        s_contextRButtonUsesZl = false;
    }
}

void hide_ring_stock_z_prompt(dMeter2Draw_c* meter) {
    if (meter == nullptr || s_ringZPrompt.ring == nullptr) {
        return;
    }

    // The obsolete Z label has five independently drawn text layers.  Its
    // separate button/glow draw is already suppressed by the meter-button
    // hooks above.  Do not hide mpButtonXY[2] here: that pane also owns the
    // assigned third-slot item and its light, which now represent R.
    for (int i = 0; i < 5; ++i) {
        CPaneMgr* textLayer = meter->mpXYText[i][2];
        if (textLayer == nullptr || textLayer->getPanePtr() == nullptr) {
            continue;
        }
        auto* text = static_cast<J2DTextBox*>(textLayer->getPanePtr());
        text->setString(64, "");
        hide_pane_tree(textLayer->getPanePtr());
        textLayer->setAlphaRate(0.0f);
    }
}

HookAction before_meter_button_draw(ModContext*, void* args, void*, void*) {
    auto* buttons = mods::arg<dMeterButton_c*>(args, 0);
    apply_context_button_layout(buttons);
    hide_legacy_overlay_z(buttons);
    if (s_ringZPrompt.ring != nullptr) {
        // The item ring already draws its complete X/Y/R assignment row.  The
        // separate emphasis-button screen is a second, self-contained draw
        // pass; on GameCube it contributes the detached glow + "Z" seen at
        // the upper right.  Its panes are made visible inside draw(), after
        // our pane styling runs, so suppress that redundant pass while the
        // ring is open instead of trying to chase its animation every frame.
        return HOOK_SKIP_ORIGINAL;
    }
    return HOOK_CONTINUE;
}

HookAction before_meter_button_execute(ModContext*, void* args, void*, void*) {
    // _execute arguments are: this, flags, A, B, R, Z, ... . The mod uses R
    // for this assignment globally, so never request the obsolete Z prompt.
    mods::arg_ref<bool>(args, 5) = false;
    return HOOK_CONTINUE;
}

void after_meter_button_execute(ModContext*, void* args, void*, void*) {
    auto* buttons = mods::arg<dMeterButton_c*>(args, 0);
    apply_context_button_layout(buttons);
    hide_legacy_overlay_z(buttons);
}

HookAction before_meter_draw(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
    update_z_hud_item(meter);
    restore_archive_face_button_diamond(meter);
    // Archive restoration resets the primary item-picture scale. Apply combo
    // geometry immediately afterward so the attachment remains 40% smaller
    // and the nested bow retains its full authored size for this draw.
    fix_xy_hud_bow_combo_layering(meter);
    align_action_text_shadow_layers(meter);
    apply_wii_u_r_button_art(meter);
    apply_wii_u_item_num_layout(meter);
    // The bundled main2D.arc supplies the Wii U prompt art and its native pane
    // geometry. Applying the older synthetic round-button and relocation pass
    // here would overwrite those resources and move the panes twice.
    apply_wii_u_archive_layout_corrections(meter);
    stabilize_wii_u_rupee_counter(meter);
    const bool hideRupees = menu_overlay_hides_rupees() ||
        s_activeCollectMenu != nullptr ||
        (s_activeSaveMenu != nullptr && s_activeSaveMenu->mDisplayMenu);
    if (hideRupees) {
        if (meter->mpRupeeKeyParent != nullptr) {
            meter->mpRupeeKeyParent->hide();
            meter->mpRupeeKeyParent->setAlphaRate(0.0f);
        }
        for (CPaneMgrAlpha* parent : meter->mpRupeeParent) {
            if (parent != nullptr) {
                parent->hide();
                parent->setAlphaRate(0.0f);
            }
        }
    }
    apply_wii_u_dpad_style(meter);
    apply_hud_backing_visibility(meter);
    hide_ring_stock_z_prompt(meter);
    return HOOK_CONTINUE;
}

void after_meter_draw(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
    restore_wii_u_item_num_layout(meter);
    draw_z_hud_item_meters(meter);
    if (!menu_overlay_hides_rupees() && s_activeCollectMenu == nullptr &&
        (s_activeSaveMenu == nullptr || !s_activeSaveMenu->mDisplayMenu)) {
        draw_uniform_rupee_digits(meter);
    }
    draw_wolf_action_icons(meter);
}

void after_meter_draw_button_z(ModContext*, void* args, void*, void*) {
    auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
    if (meter == nullptr || meter->mpTextXY[2] == nullptr) {
        return;
    }

    // The stock Z update can run after the general HUD preparation and
    // re-show its detached "Z" action label.  Dusklight presents this third
    // item as R, so suppress the native label at the point where it is
    // authored.  When the item ring is open, keep the R badge and its assigned
    // item visible at the same third-slot target used by X and Y.
    meter->mpTextXY[2]->hide();
    meter->mpTextXY[2]->getPanePtr()->setAlpha(0);
    meter->mpTextXY[2]->scale(0.0f, 0.0f);
    apply_wii_u_r_button_art(meter);
    if (s_ringZPrompt.ring != nullptr) {
        update_z_hud_item(meter);
        hide_ring_stock_z_prompt(meter);
    }
}

HookAction before_meter_draw_button_z(ModContext*, void* args, void*, void*) {
    if (s_ringZPrompt.ring != nullptr) {
        // The item ring supplies its own Y/X/R assignment row.  The stock Z
        // action renderer is the owner of the detached glow + "Z" at upper
        // right, and hiding its panes after the call is too late on some
        // frames.  Suppress that draw pass completely while the ring is open.
        auto* meter = mods::arg<dMeter2Draw_c*>(args, 0);
        hide_ring_stock_z_prompt(meter);
        return HOOK_SKIP_ORIGINAL;
    }
    return HOOK_CONTINUE;
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

    // Fixed mode uses the physical ZL edge captured after PADRead. Follow
    // mode honors whichever physical control the user's Dusklight profile
    // maps to the logical GameCube L input.
    const bool comboTriggered =
        controller_compatibility() == ControllerCompatibility::FixedTphd ?
            s_fixedZlTrig : mDoCPd_c::getTrigL(PAD_1);
    if (comboTriggered) {
        s_pendingAssign = {};
        if (item_assign_allowed(ring)) {
            if (!set_z_mix_item(ring)) {
                for (int i = 0; i < MAX_SELECT_ITEM; ++i) {
                    ring->setSelectItemForce(i);
                }
                ring->setMixItem();
            }
        } else {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, nullptr, 0, 0, 1.0f, 1.0f,
                -1.0f, -1.0f, 0);
        }
        return HOOK_SKIP_ORIGINAL;
    }

    if (mDoCPd_c::getTrigR(PAD_1)) {
        s_pendingAssign = {};
        if (item_assign_allowed(ring)) {
            assign_current_item(ring, kZItemSlot);
            if (ring->mpItemExplain->getStatus() == 0) {
                ring->setStatus(dMenu_Ring_c::STATUS_WAIT);
                ring->stick_wait_init();
            }
        } else {
            Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR, nullptr, 0, 0, 1.0f, 1.0f,
                -1.0f, -1.0f, 0);
        }
        return HOOK_SKIP_ORIGINAL;
    }

    if (!mDoCPd_c::getTrigZ(PAD_1)) {
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

    *static_cast<BOOL*>(retval) =
        s_dpadMidnaTrig || midna_action_triggered() || consume_touch_midna_trigger();
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_item_action_trigger(ModContext*, void* args, void* retval, void*) {
    auto* link = mods::arg<daAlink_c*>(args, 0);
    if (link == nullptr || !link->checkBoomerangReadyAnime()) {
        return HOOK_CONTINUE;
    }

    // The original boomerang lock action shares GameCube R with the generic
    // item-action path.  R belongs to the third item slot in this layout, so
    // redirect only the boomerang's ready/aiming state to TPHD's ZL action.
    // Follow mode uses the logical L action supplied by Dusklight's profile;
    // Fixed mode reads the physical ZL edge captured after the pad update.
    *static_cast<BOOL*>(retval) =
        controller_compatibility() == ControllerCompatibility::FixedTphd ?
            s_fixedZlTrig : mDoCPd_c::getTrigL(PAD_1);
    return HOOK_SKIP_ORIGINAL;
}

HookAction before_menu_window_execute(ModContext*, void*, void*, void*) {
    s_menuWindowSuppressedHeld = 0;
    s_menuWindowSuppressedTrig = 0;

    // Only hide Midna's control from the no-menu dispatcher, where the same
    // input could otherwise open the item wheel. Once a menu is active, Up is
    // navigation input and must remain available to that screen.
    if (dMeter2Info_getWindowStatus() != 0) {
        return HOOK_CONTINUE;
    }

    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);
    u32 suppressMask = 0;
    if (controller_compatibility() == ControllerCompatibility::FixedTphd) {
        // Fixed Bindings reserves Up for Midna.
        suppressMask = PAD_BUTTON_UP;
    } else if (midna_action_triggered()) {
        // Follow mode leaves the controller profile untouched. When its Call
        // Midna action fires, suppress the normal game button fed by that same
        // physical control only while the item-menu dispatcher processes it.
        suppressMask = midna_game_button_mask();
    }

    s_menuWindowSuppressedHeld = pad.mButtonFlags & suppressMask;
    s_menuWindowSuppressedTrig = pad.mPressedButtonFlags & suppressMask;
    pad.mButtonFlags &= ~suppressMask;
    pad.mPressedButtonFlags &= ~suppressMask;
    return HOOK_CONTINUE;
}

void after_menu_window_execute(ModContext*, void*, void*, void*) {
    interface_of_controller_pad& pad = mDoCPd_c::getCpadInfo(PAD_1);
    pad.mButtonFlags |= s_menuWindowSuppressedHeld;
    pad.mPressedButtonFlags |= s_menuWindowSuppressedTrig;
    s_menuWindowSuppressedHeld = 0;
    s_menuWindowSuppressedTrig = 0;
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
    const ModResult result = mods::hook::add_pre<Hook>(svc_hook, callback);
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
    const ModResult result = mods::hook::add_post<Hook>(svc_hook, callback);
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
    constexpr const char* blackProPaths[] = {
        "hud/face-button-a-black-pro.bti",
        "hud/face-button-b-black-pro.bti",
        "hud/face-button-x-black-pro.bti",
        "hud/face-button-y-black-pro.bti",
    };
    for (std::size_t index = 0; index < std::size(blackProPaths); ++index) {
        if (svc_resource->load(mod_ctx, blackProPaths[index],
                &s_blackProFaceButtonResources[index]) != MOD_OK) {
            svc_log->warn(mod_ctx, "Unable to load a Black Pro button texture");
        }
    }
    if (svc_resource->load(mod_ctx, "hud/face-button-blank-black-pro.bti",
            &s_blackProBlankFaceButtonResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the blank Black Pro button texture");
    }
    if (svc_resource->load(mod_ctx, "hud/shoulder-button-r-black-pro.bti",
            &s_blackProShoulderButtonResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the Black Pro R button texture");
    }
    if (svc_resource->load(mod_ctx, "hud/shoulder-button-zl.bti",
            &s_zlShoulderButtonResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the ZL button texture");
    }
    if (svc_resource->load(mod_ctx, "hud/shoulder-button-zl-black-pro.bti",
            &s_blackProZlShoulderButtonResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the Black Pro ZL button texture");
    }
    if (svc_resource->load(mod_ctx, "menu/collection-background.bti",
            &s_collectBackgroundResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the Collection menu background");
    }
    if (svc_resource->load(mod_ctx, "menu/menu-button-frame.bti",
            &s_collectMenuButtonResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the Collection menu button frame");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-background.bti",
            &s_fileSelectBackgroundResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the File Selection background");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-row.bti",
            &s_fileSelectRowResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the File Selection row panel");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-row-selected.bti",
            &s_fileSelectSelectedRowResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the selected File Selection row panel");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-row-clear.bti",
            &s_fileSelectClearRowResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the clear File Selection row panel");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-title-rules.bti",
            &s_fileSelectTitleRulesResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the File Selection title rules");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-back-label.bti",
            &s_fileSelectBackLabelResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the File Selection Back label");
    }
    if (svc_resource->load(mod_ctx, "menu/file-select-prompt-flourish.bti",
            &s_fileSelectPromptFlourishResource) != MOD_OK) {
        svc_log->warn(mod_ctx, "Unable to load the File Selection prompt flourish");
    }
    constexpr const char* numberPaths[] = {
        "menu/file-select-number-1.bti",
        "menu/file-select-number-2.bti",
        "menu/file-select-number-3.bti",
    };
    for (std::size_t index = 0; index < 3; ++index) {
        if (svc_resource->load(mod_ctx, numberPaths[index],
                &s_fileSelectNumberResources[index]) != MOD_OK) {
            svc_log->warn(mod_ctx, "Unable to load a File Selection numeral");
        }
    }
}

void shutdown_face_button_textures() {
    free_resource(s_faceButtonAResource);
    free_resource(s_faceButtonBResource);
    for (ResourceBuffer& resource : s_blackProFaceButtonResources) {
        free_resource(resource);
    }
    free_resource(s_blackProBlankFaceButtonResource);
    free_resource(s_blackProShoulderButtonResource);
    free_resource(s_zlShoulderButtonResource);
    free_resource(s_blackProZlShoulderButtonResource);
    free_resource(s_collectBackgroundResource);
    free_resource(s_collectMenuButtonResource);
    free_resource(s_fileSelectBackgroundResource);
    free_resource(s_fileSelectRowResource);
    free_resource(s_fileSelectSelectedRowResource);
    free_resource(s_fileSelectClearRowResource);
    free_resource(s_fileSelectTitleRulesResource);
    free_resource(s_fileSelectBackLabelResource);
    free_resource(s_fileSelectPromptFlourishResource);
    for (ResourceBuffer& resource : s_fileSelectNumberResources) {
        free_resource(resource);
    }
}

void shutdown_item_slot_resources() {
    destroy_ring_z_prompt(s_ringZPrompt.ring);
    s_pendingAssign = {};
    s_activeFileSelect = nullptr;
    s_activeSaveMenu = nullptr;
    s_activeCollectMenu = nullptr;
    s_collectTitleFrame = nullptr;
    s_collectSaveFrame = nullptr;
    s_collectOptionsFrame = nullptr;
    s_collectTopRule = nullptr;
    s_collectBottomRule = nullptr;
    s_collectTopRuleInner = nullptr;
    s_collectBottomRuleInner = nullptr;
    s_collectTitleLabel = nullptr;
    s_descenderCorrectionDrawDepth = 0;
    s_fileSelectYesNoLayoutReady = false;
    s_dpadMidnaHeld = false;
    s_dpadMidnaTrig = false;
    s_menuWindowSuppressedHeld = 0;
    s_menuWindowSuppressedTrig = 0;
    s_getActionBindTrig = nullptr;
    s_getActionBindButton = nullptr;
    clear_z_heavy_boots_input_lock();
}

void shutdown_wolf_action_icons() {
    free_picture(s_senseIconResource, s_senseIconPicture);
    free_picture(s_digIconResource, s_digIconPicture);
    free_picture(s_attackIconResource, s_attackIconPicture);
}

ModResult install_item_slot_hooks(ModError* error) {
    resolve_action_binding_functions();

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
    ADD_PRE(ItemActionTriggerHook, before_item_action_trigger,
        "boomerang ZL lock input");
    ADD_POST(SetStickDataHook, after_set_stick_data, "scoped third-item input");
    ADD_POST(RingCreateHook, after_ring_create, "item ring create");
    ADD_PRE(MenuRingDeleteHook, before_menu_ring_delete, "item ring prompt cleanup");
    ADD_POST(RingDrawHook, after_ring_draw, "item ring draw");
    ADD_PRE(ItemExplainDrawHook, before_item_explain_draw,
        "item description button styling");
    ADD_PRE(MeterButtonExecuteHook, before_meter_button_execute,
        "disable legacy item-ring Z overlay");
    ADD_POST(MeterButtonExecuteHook, after_meter_button_execute,
        "collapse legacy item-ring Z overlay");
    ADD_PRE(MeterButtonDrawHook, before_meter_button_draw,
        "hide legacy item-ring Z overlay");
    ADD_PRE(MeterDrawHook, before_meter_draw, "HUD draw (before)");
    ADD_POST(MeterDrawHook, after_meter_draw, "HUD draw (after)");
    ADD_PRE(MeterDrawButtonZHook, before_meter_draw_button_z,
        "disable item-ring Z action label");
    ADD_POST(MeterDrawButtonZHook, after_meter_draw_button_z,
        "replace item-ring Z prompt with R");
    ADD_POST(MeterMoveButtonCrossHook, after_meter_move_button_cross, "D-pad update");
    ADD_PRE(MeterDrawKanteraHook, before_meter_draw_kantera, "lantern meter draw");
    ADD_PRE(MeterDrawOxygenHook, before_meter_draw_oxygen, "oxygen meter draw");
    ADD_POST(MeterMidnaAlphaHook, after_meter_midna_alpha, "Midna icon opacity");
    ADD_PRE(MeterMapDrawHook, before_meter_map_draw, "minimap draw (before)");
    ADD_POST(MeterMapDrawHook, after_meter_map_draw, "minimap draw (after)");
    ADD_POST(CollectCreateHook, after_collect_create, "collection menu buttons");
    ADD_POST(LetterCreateHook, after_letter_create, "letter menu buttons");
    ADD_PRE(LetterDrawHook, before_letter_draw, "letter menu responsive buttons");
    ADD_POST(FishingCreateHook, after_fishing_create, "fish journal buttons");
    ADD_PRE(FishingDrawHook, before_fishing_draw, "fish journal responsive buttons");
    ADD_POST(SkillCreateHook, after_skill_create, "hidden skills menu buttons");
    ADD_PRE(SkillDrawHook, before_skill_draw, "hidden skills responsive buttons");
    ADD_POST(InsectCreateHook, after_insect_create, "golden bugs menu buttons");
    ADD_PRE(InsectDrawHook, before_insect_draw, "golden bugs responsive buttons");
    ADD_POST(CollectMoveHook, after_collect_move, "collection menu frame styling");
    ADD_PRE(CollectDrawHook, before_collect_draw, "collection menu HD draw");
    ADD_PRE(CollectDeleteHook, before_collect_delete, "collection menu cleanup");
#if defined(_WIN32)
    ADD_POST(CollectWideHook, after_collect_wide,
        "collection Windows final title anchor");
#endif
    ADD_POST(SelectCursorUpdateHook, after_select_cursor_update,
        "collection footer cursor final alignment");
    ADD_PRE(FmapMoveHook, before_fmap_move, "field map portals R button mapping");
    ADD_PRE(FmapDrawHook, before_fmap_draw,
        "field map HD background, title, and prompts");
    ADD_PRE(DmapDrawHook, before_dmap_draw,
        "dungeon map HD background, title, and prompts");
    ADD_POST(OptionCreateHook, after_option_create, "options menu HD style");
    ADD_POST(BrightCheckScreenSetHook, after_brightness_check_screen_set,
        "brightness check HD style");
    ADD_PRE(OptionMoveHook, before_option_move, "options display button mapping");
    ADD_PRE(OptionDrawHook, before_option_draw, "options menu HD draw");
    ADD_POST(OptionDrawHook, after_option_draw, "options descender scope");
    ADD_PRE(BrightCheckDrawHook, before_brightness_check_draw,
        "brightness check descender scope");
    ADD_POST(BrightCheckDrawHook, after_brightness_check_draw,
        "brightness check descender cleanup");
    ADD_PRE(ResFontDrawCharHook, before_res_font_draw_char,
        "menu font descender correction");
    ADD_PRE(OptionDrawArrowsHook, before_option_draw_arrows,
        "options menu selection arrows");
    ADD_POST(FileSelectCreateHook, after_file_select_create, "file selection HD style");
    ADD_POST(FileSelectMoveHook, after_file_select_move, "file selection HD prompt position");
    ADD_PRE(FileSelectDrawHook, before_file_select_draw, "file selection HD text");
    ADD_PRE(FileSelectMainDrawHook, before_file_select_main_draw,
        "file selection descender scope");
    ADD_POST(FileSelectMainDrawHook, after_file_select_main_draw,
        "file selection descender cleanup");
    ADD_POST(SaveMenuScreenSetHook, after_save_menu_screen_set, "save menu HD style");
    ADD_PRE(SaveMenuDeleteHook, before_save_menu_delete, "save menu cleanup");
    ADD_PRE(SaveMenuDrawHook, before_save_menu_draw, "save menu HD draw");
    ADD_POST(SaveMenuWideHook, after_save_menu_wide, "save menu HD wide layout");
    ADD_PRE(SaveDlstDrawHook, before_save_dlst_draw, "save menu HD final draw");
    ADD_PRE(MenuWindowExecuteHook, before_menu_window_execute, "Midna pause-menu input");
    ADD_POST(MenuWindowExecuteHook, after_menu_window_execute, "Midna input restore");
    ADD_PRE(RingSetActiveCursorHook, before_ring_set_active_cursor, "item ring cursor (before)");
    ADD_POST(RingSetActiveCursorHook, after_ring_set_active_cursor, "item ring cursor (after)");
    ADD_POST(RingSetMixMessageHook, after_ring_set_mix_message,
             "item ring combo prompt");
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
