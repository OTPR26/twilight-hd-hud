"""Item cards and dialogue scale their text and symbols together, then restore."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root/'src/dialogue_text_screen.inc').read_text()
fixture = r'''
#include "dialogue_text_layout.hpp"
#include "hud_scaling.hpp"
#include <array>
#include <algorithm>
#include <cassert>
#include <cmath>
using namespace twilight_hd_hud;
using s16=short;
struct Vec {float x,y,z;};
using Mtx=float[3][4];
struct J2DPane {
 float x=100,y=200,sx=1,sy=1;
 float getTranslateX(){return x;} float getTranslateY(){return y;}
 float getScaleX(){return sx;} float getScaleY(){return sy;}
 void scale(float a,float b){sx=a;sy=b;} void translate(float a,float b){x=a;y=b;}
};
struct CPaneMgr {
 J2DPane* pane=nullptr;J2DPane* getPanePtr(){return pane;}
 Vec getGlobalVtxCenter(J2DPane* p,bool,int){return {p->x,p->y,0};}
 Vec getGlobalVtx(J2DPane* p,Mtx*,int,bool,int){return {p->x-50,p->y-20,0};}
};
void shift_action_pane(J2DPane* p,float x,float y){p->x+=x;p->y+=y;}
struct CharInfo {float field_0x00,field_0x04,field_0x08,field_0x0c;};
struct COutFontSet_c {
 float mPosX=80,mPosY=180,mSizeX=24,mSizeY=24;
 int getType(){return 0;} J2DPane* getTextBoxPtr(){return nullptr;}
};
struct OutFont {std::array<COutFontSet_c*,35> mpOfs{};};
struct dMsgScrnBase_c {
 virtual ~dMsgScrnBase_c()=default;
 CPaneMgr* mpTm_c[7]{};CPaneMgr* mpTmr_c[7]{};CharInfo* mCharInfoPtr=nullptr;OutFont* mpOutFont=nullptr;
};
struct dMsgScrnTalk_c:dMsgScrnBase_c{};
struct dMsgScrnItem_c:dMsgScrnBase_c{};
struct Scales {float dialogueText=1;} scales;
Scales hud_scales(){return scales;}
struct Hio {float mBoxItemTextPosX=12,mBoxItemTextPosY=20;} g_MsgObject_HIO_c;
struct ModContext{};
namespace mods {template<class T>T arg(void* p,int){return *static_cast<T*>(p);}}
void end_item_prompt_font(){} void restore_item_get_assignment_icons(){}
bool near(float a,float b){return std::abs(a-b)<0.001f;}
'''
checks = r'''
int main(){
 for(bool item:{false,true}) for(float setting:{0.5f,1.f,1.25f}) {
  dMsgScrnTalk_c talk;dMsgScrnItem_c card;dMsgScrnBase_c* screen=item?static_cast<dMsgScrnBase_c*>(&card):&talk;
  J2DPane main,ruby;CPaneMgr mainMgr,rubyMgr;mainMgr.pane=&main;rubyMgr.pane=&ruby;
  screen->mpTm_c[0]=&mainMgr;screen->mpTmr_c[0]=&rubyMgr;
  struct Characters {CharInfo data[150]{};short count=1;} chars;
  chars.data[0]={80,180,1,1};screen->mCharInfoPtr=chars.data;
  COutFontSet_c symbol;OutFont out;out.mpOfs[0]=&symbol;screen->mpOutFont=&out;
  scales.dialogueText=setting;
  float factor=item?setting:dialogue_text_multiplier(setting);
  scale_dialogue_for_draw(screen);
  assert(near(main.sx,factor)&&near(ruby.sy,factor));
  assert(near(chars.data[0].field_0x08,factor)&&near(symbol.mSizeX,24*factor));
  float ox=item?10:0,oy=item?20:0;
  assert(near(symbol.mPosX+ox,(item?50:100)+(80+ox-(item?50:100))*factor));
  assert(near(symbol.mPosY+oy,200+(180+oy-200)*factor));
  after_message_screen_draw(nullptr,&screen,nullptr,nullptr);
  assert(main.sx==1&&ruby.sy==1&&main.x==100&&main.y==200);
  assert(symbol.mPosX==80&&symbol.mSizeX==24&&chars.data[0].field_0x08==1);
  assert(s_dialogueDrawState.screen==nullptr);
 }
 dMsgScrnBase_c menu;scale_dialogue_for_draw(&menu);assert(s_dialogueDrawState.screen==nullptr);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-message-scale-') as directory:
 p=Path(directory);(p/'test.cpp').write_text(fixture+source+checks)
 subprocess.run(['c++','-std=c++20','-I',str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: item/skill cards and dialogue text/ruby/symbol scaling, offsets and restoration')
