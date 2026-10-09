"""The HUD draw hook must preserve the companion partition and native geometry."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
callback = source.split('HookAction before_meter_screen_draw(', 1)[1].split('\nvoid after_meter_draw_kantera_meter(', 1)[0]
callback = 'HookAction before_meter_screen_draw(' + callback
prefix = 'HookAction before_gauge_screen_draw(' + source.split('HookAction before_gauge_screen_draw(',1)[1].split('    auto* meter = s_gaugeDraw.meter;',1)[0]
calls = '''refresh_native_face_button_items_for_touch_transition update_z_hud_item
restore_archive_face_button_diamond fix_xy_hud_bow_combo_layering
align_action_text_shadow_layers apply_wii_u_r_button_art apply_wii_u_item_num_layout
apply_wii_u_archive_layout_corrections stabilize_wii_u_rupee_counter
scale_rupee_icon_for_draw apply_wii_u_dpad_style style_native_midna_backing
align_native_midna_hud position_midna_hud update_midna_shoulder_badge
hide_ring_stock_z_prompt'''.split()
fixture = '''
#include "dual_screen.hpp"
#include <cassert>
using namespace twilight_hd_hud;
struct ModContext {};
enum HookAction { HOOK_CONTINUE };
struct J2DScreen {};
struct Pane {
 bool visible=true;void show(){visible=true;}void hide(){visible=false;}void setAlpha(int){}
 Pane* getPanePtr(){return this;}float getSizeX(){return 1;}float getSizeY(){return 1;}
 void rotate(float,float,int,float){}void setMirror(int){}
};
struct dMeter2Draw_c { J2DScreen* mpScreen; Pane* mpLightXY[3]{}; Pane* mpButtonCrossParent = nullptr; Pane* mpButtonParent=nullptr; Pane* mpItemB=nullptr; void drawButtonCross(float,float) {} };
namespace mods { template<class T> T arg(void* args, int) { return static_cast<T>(args); } }
bool companion = false;
bool query() { return companion; }
DualScreenHost s_dualScreenHost{query};
dMeter2Draw_c* s_pendingMeterDraw = nullptr;
int geometryCalls = 0, artworkCalls = 0;
enum class Feature {DpadShortcuts};
bool feature_enabled(Feature) {return true;}
enum class HudPaneSlot {DPad};
int paneState=0;
int& hud_pane_state(HudPaneSlot){return paneState;}
struct {float mButtonCrossOFFPosX=0;} g_drawHIO;
int dpadTransforms=0;
void apply_wii_u_dpad_transform(dMeter2Draw_c*){++dpadTransforms;}
bool full=false,showDpad=true;
bool dual_screen_full_diamond(){return full;}
bool touch_controls_active(){return false;}
bool z_item_menu_or_pause_context(){return false;}
void align_dual_screen_left_hud(dMeter2Draw_c*){}
int compactPrepares=0;
void prepare_dual_screen_compact_hud(dMeter2Draw_c*){++compactPrepares;}
int fullPrepares=0;
void prepare_dual_screen_full_hud(dMeter2Draw_c* m){++fullPrepares;m->mpButtonParent->show();}
bool dual_screen_show_dpad(){return showDpad;}
void* s_companionDrawItem=reinterpret_cast<void*>(1);
struct CompanionCompositeHook{static inline void* g_orig=reinterpret_cast<void*>(1);};
constexpr int dItemNo_NONE_e=255,MIRROR0=0,ROTATE_Z=0;
int dComIfGs_getBButtonItemKey(){return 0;}
Pane* first_picture_pane(Pane* p){return p;}
bool use_mod_item_slot() { return true; }
bool menu_overlay_hides_rupees() { return false; }
struct Collect {J2DScreen* mpScreen;J2DScreen* icons;J2DScreen* getIconScreen(){return icons;}};
Collect* s_activeCollectMenu=nullptr;
struct Save { bool mDisplayMenu = false; };
Save* s_activeSaveMenu = nullptr;
void apply_button_layout_preference(dMeter2Draw_c*) { ++artworkCalls; }
void apply_hud_backing_visibility(dMeter2Draw_c*) { ++artworkCalls; }
'''
fixture += '\n'.join(f'template<class... T> void {name}(T...) {{ ++geometryCalls; }}' for name in calls)
top_source=(root/'src/dual_screen_hud.inc').read_text()
fixture += top_source.split('// Draw the same archive',1)[0]
fixture += '\n' + callback
fixture += '''
int collectionLayouts=0,collectionPrompts=0;
void apply_collection_screen(Collect*){++collectionLayouts;}
void apply_collection_prompts(Collect*){++collectionPrompts;}
'''
fixture += prefix + '    return HOOK_CONTINUE;\n}\n'
fixture += '''
int main() {
    J2DScreen screen, other;
    dMeter2Draw_c meter{&screen};
    companion = true;
    s_pendingMeterDraw = &meter;
    before_meter_screen_draw(nullptr, &other, nullptr, nullptr);
    assert(s_pendingMeterDraw == &meter && artworkCalls == 0);
    before_meter_screen_draw(nullptr, &screen, nullptr, nullptr);
    assert(s_pendingMeterDraw == nullptr);
    assert(geometryCalls == 0 && artworkCalls == 2);
    Pane cross; meter.mpButtonCrossParent=&cross;
    s_pendingMeterDraw=&meter;
    before_meter_screen_draw(nullptr,&screen,nullptr,nullptr);
    assert(dpadTransforms==1 && geometryCalls==1);
    Pane buttons;meter.mpButtonParent=&buttons;
    showDpad=false;full=true;
    s_pendingMeterDraw=&meter;
    before_meter_screen_draw(nullptr,&screen,nullptr,nullptr);
    assert(!cross.visible&&buttons.visible&&fullPrepares==1);
    full=false;showDpad=true;
    s_pendingMeterDraw=&meter;
    before_meter_screen_draw(nullptr,&screen,nullptr,nullptr);
    assert(cross.visible&&compactPrepares==1);
    geometryCalls=0; artworkCalls=2;
    companion = false;
    s_pendingMeterDraw = &meter;
    before_meter_screen_draw(nullptr, &screen, nullptr, nullptr);
    assert(geometryCalls == 16 && artworkCalls == 3);
    geometryCalls = artworkCalls = 0;
    companion = true;
    s_pendingMeterDraw = &meter;
    before_meter_screen_draw(nullptr, &screen, nullptr, nullptr);
    assert(geometryCalls == 1 && artworkCalls == 2);
    Collect collection{&screen,&other};s_activeCollectMenu=&collection;
    before_gauge_screen_draw(nullptr,&screen,nullptr,nullptr);
    before_gauge_screen_draw(nullptr,&other,nullptr,nullptr);
    assert(collectionLayouts==1&&collectionPrompts==1);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp = Path(directory) / 'test.cpp'
    binary = Path(directory) / 'test'
    cpp.write_text(fixture)
    subprocess.run(['c++', '-std=c++20', '-I', str(root / 'src'), str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
print('PASS: companion HUD ownership, Collection final layout, and single-screen transitions')

shortcuts = 'void update_dpad_shortcuts(' + source.split('void update_dpad_shortcuts(', 1)[1].split('void after_pad_read(', 1)[0]
input_fixture = r"""
#include <cassert>
using u32=unsigned;
constexpr u32 PAD_BUTTON_UP=1, PAD_BUTTON_LEFT=2, PAD_BUTTON_RIGHT=4, DOWN=8;
struct interface_of_controller_pad {u32 mButtonFlags, mPressedButtonFlags;};
struct Touch {bool items=false; bool items_triggered(){return items;} bool items_held(){return items;}} s_touchInput;
bool enabled=true, combined=false;
bool s_combinedMapMinimapTrig=false,s_fixedOpenMapTrig=false,s_fixedToggleMinimapTrig=false;
bool use_tphd_dpad_map_bindings(){return enabled;}
struct Masks {u32 map=1,minimap=6;bool combined;};
Masks active_map_masks(){return {1,6,combined};}
""" + shortcuts + r"""
int main(){
 interface_of_controller_pad pad{15,15};
 update_dpad_shortcuts(pad,true);
 assert(s_fixedOpenMapTrig && s_fixedToggleMinimapTrig);
 assert(pad.mButtonFlags==DOWN && pad.mPressedButtonFlags==DOWN);
 pad={15,15}; enabled=false;update_dpad_shortcuts(pad,true);
 assert(pad.mButtonFlags==15 && !s_fixedOpenMapTrig && !s_fixedToggleMinimapTrig);
 enabled=true;update_dpad_shortcuts(pad,false);
 assert(pad.mButtonFlags==15 && !s_fixedOpenMapTrig);
 combined=true;pad={1,1};update_dpad_shortcuts(pad,true);
 assert(s_combinedMapMinimapTrig && !s_fixedOpenMapTrig && !s_fixedToggleMinimapTrig);
 s_touchInput.items=true;pad={1,1};update_dpad_shortcuts(pad,true);
 assert(!s_combinedMapMinimapTrig && pad.mPressedButtonFlags==1);
}
"""
with tempfile.TemporaryDirectory() as directory:
    cpp=Path(directory)/'input.cpp';binary=Path(directory)/'input'
    cpp.write_text(input_fixture)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: shared map shortcuts, native Down, disabled/menu input, and touch Items')

pose_source = top_source.split('struct DualHudPanePose {', 1)[1].split('struct DualRItemScope', 1)[0]
pose_fixture = r'''
#include <cassert>
#include <cstring>
#include <vector>
using f32=float; using u8=unsigned char; using Mtx=float[3][4];
namespace JGeometry {template<class T> struct TBox2 {T x=0,y=0,w=0,h=0;};}
struct J2DPane {
 JGeometry::TBox2<f32> mBounds{},mGlobalBounds{},mClipRect{};
 Mtx mPositionMtx{},mGlobalMtx{};
 f32 mScaleX=1,mScaleY=1,mTranslateX=0,mTranslateY=0;
 f32 mRotateZ=0,mRotateOffsetX=0,mRotateOffsetY=0;
 u8 mAlpha=255,mColorAlpha=255;bool mVisible=true,mIsInfluencedAlpha=true;char mRotAxis='Z';
 J2DPane* child=nullptr;J2DPane* next=nullptr;J2DPane* parent=nullptr;
 J2DPane* getFirstChildPane(){return child;}
 J2DPane* getNextChildPane(){return next;}
 J2DPane* getParentPane(){return parent;}
 void insertChild(J2DPane* before,J2DPane* p){
  if(p->parent){
   auto** link=&p->parent->child;
   while(*link&&*link!=p)link=&(*link)->next;
   if(*link)*link=p->next;
  }
  auto** link=&child;
  while(*link&&*link!=before)link=&(*link)->next;
  p->parent=this;p->next=*link;*link=p;
 }
 void appendChild(J2DPane* p){insertChild(nullptr,p);}
};
''' + 'struct DualHudPanePose {' + pose_source + r'''
int main(){
 J2DPane root,child,sibling,bow;
 root.appendChild(&child);root.appendChild(&sibling);child.appendChild(&bow);
 child.mTranslateX=37; child.mVisible=false;
 child.mGlobalMtx[0][3]=71;child.mGlobalBounds.x=12;
 sibling.mAlpha=42;sibling.mClipRect.w=128;
 {
  DualHudPoseScope scope(&root);
  root.insertChild(&child,&bow);
  assert(bow.getParentPane()==&root);
  root.mScaleX=3;root.mPositionMtx[1][1]=8;
  child.mTranslateX=99;child.mVisible=true;
  child.mGlobalMtx[0][3]=999;child.mGlobalBounds.x=88;
  child.mRotAxis='X';child.mRotateZ=180;child.mIsInfluencedAlpha=false;
  sibling.mAlpha=255;sibling.mClipRect.w=1;
 }
 assert(root.child==&child&&child.next==&sibling);
 assert(child.child==&bow&&bow.parent==&child&&bow.next==nullptr);
 assert(root.mScaleX==1&&root.mPositionMtx[1][1]==0);
 assert(child.mTranslateX==37&&!child.mVisible);
 assert(child.mGlobalMtx[0][3]==71&&child.mGlobalBounds.x==12);
 assert(child.mRotAxis=='Z'&&child.mRotateZ==0&&child.mIsInfluencedAlpha);
 assert(sibling.mAlpha==42&&sibling.mClipRect.w==128);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp=Path(directory)/'pose.cpp';binary=Path(directory)/'pose'
    cpp.write_text(pose_fixture)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: full HUD draw restores companion transforms, visibility, alpha, and cached bounds')

r_scope = 'struct DualRItemScope {' + top_source.split('struct DualRItemScope {', 1)[1].split('std::unique_ptr<DualHudPoseScope>', 1)[0]
r_fixture = r'''
#include <array>
#include <cassert>
using f32=float;
struct ResTIMG {int id;};
struct Texture {const ResTIMG* info;const ResTIMG* getTexInfo(){return info;}};
struct J2DPicture {
 Texture texture;
 Texture* getTexture(int){return &texture;}
 void changeTexture(const ResTIMG* info,int){texture.info=info;}
};
J2DPicture* as_picture(J2DPicture* p){return p;}
struct CPaneMgr {J2DPicture* pane;J2DPicture* getPanePtr(){return pane;}};
struct dMeter2Draw_c {
 struct item_params {float scale;};
 item_params mItemParams[4]{};
 float field_0x6ac[3]{},field_0x6b8[3]{},field_0x6c4[3]{},field_0x6d0[3]{};
 CPaneMgr* mpItemR;J2DPicture* mpItemXYPane[3]{};
};
''' + r_scope + r'''
int main(){
 ResTIMG oldPrimary{1},oldSecondary{2},topItem{3};
 J2DPicture primary{{&oldPrimary}},secondary{{&oldSecondary}};
 CPaneMgr manager{&primary};dMeter2Draw_c meter{};
 meter.mpItemR=&manager;meter.mpItemXYPane[2]=&secondary;
 meter.mItemParams[2].scale=0.7f;
 meter.field_0x6ac[2]=1;meter.field_0x6b8[2]=2;
 meter.field_0x6c4[2]=3;meter.field_0x6d0[2]=4;
 {
  DualRItemScope scope(&meter);
  primary.changeTexture(&topItem,0);secondary.changeTexture(&topItem,0);
  meter.mItemParams[2].scale=2;
  meter.field_0x6ac[2]=10;meter.field_0x6b8[2]=20;
  meter.field_0x6c4[2]=30;meter.field_0x6d0[2]=40;
 }
 assert(primary.texture.info==&oldPrimary&&secondary.texture.info==&oldSecondary);
 assert(meter.mItemParams[2].scale==0.7f);
 assert(meter.field_0x6ac[2]==1&&meter.field_0x6b8[2]==2);
 assert(meter.field_0x6c4[2]==3&&meter.field_0x6d0[2]==4);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp=Path(directory)/'r_item.cpp';binary=Path(directory)/'r_item'
    cpp.write_text(r_fixture)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: R rendering restores companion item textures, dimensions, and parameters')

alpha_source = 'void update_z_hud_item_alpha(' + source.split('void update_z_hud_item_alpha(',1)[1].split('void draw_z_ammo(',1)[0]
alpha_fixture = r'''
#include <algorithm>
#include <cassert>
using f32=float;using u8=unsigned char;
struct Pane {u8 alpha=255;f32 rate=1;u8 getInitAlpha(){return 255;}f32 getAlphaRate(){return rate;}void setAlpha(u8 a){alpha=a;}};
struct dMeter2Draw_c {Pane* mpItemR;Pane* mpLightXY[3]{};Pane* mpButtonXY[3]{};Pane* mpButtonParent;};
struct Hio {float mButtonZAlpha=1,mParentAlpha=1,mMainHUDButtonsAlpha=1,mButtonZItemBaseAlpha=1;u8 mButtonXYItemDimAlpha=96,mButtonXYBaseDimAlpha=128;} g_drawHIO;
u8 clamp_hud_alpha(float a){return static_cast<u8>(std::clamp(a,0.0f,255.0f));}
''' + alpha_source + r'''
int main(){
 Pane item,light,button,parent;dMeter2Draw_c meter{};
 meter.mpItemR=&item;meter.mpLightXY[2]=&light;
 meter.mpButtonXY[2]=&button;meter.mpButtonParent=&parent;
 update_z_hud_item_alpha(&meter,true);
 assert(item.alpha==255&&button.alpha==255);
 update_z_hud_item_alpha(&meter,false);
 assert(item.alpha==96&&button.alpha==128);
 parent.rate=0.5f;update_z_hud_item_alpha(&meter,true);
 assert(item.alpha==127&&button.alpha==127);
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp=Path(directory)/'alpha.cpp';binary=Path(directory)/'alpha'
    cpp.write_text(alpha_fixture)
    subprocess.run(['c++','-std=c++20',str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: usable R items stay opaque; unavailable items dim and respect HUD fades')
