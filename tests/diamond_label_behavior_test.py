"""Run the production diamond-label placement at varied aspect/scale/binding."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1];s=(root/'src/item_slot_hooks.cpp').read_text()
fn=s[s.index('void separate_diamond_action_label('):s.index('void restore_archive_face_button_diamond(')]
fixture=r'''
#include "action_prompt_layout.hpp"
#include <cmath>
#include <cassert>
#include <algorithm>
using namespace twilight_hd_hud;using f32=float;using Mtx=float[3][4];
struct Vec{float x,y,z;};enum{HBIND_CENTER,HBIND_RIGHT,HBIND_LEFT};
struct JUTFont {int getHeight(){return 20;}int getWidth(unsigned){return 10;}bool isLeadByte(unsigned c){return c==0x82;}};
struct J2DPane {float left=0,right=100,scale=1,shift=0;};
struct J2DTextBox:J2DPane {struct TFontSize{float mSizeX,mSizeY;};JUTFont font;float size=20;int binding=HBIND_LEFT;
 JUTFont* getFont(){return &font;}void getFontSize(TFontSize& f){f={size,size};}
 float getCharSpace(){return 0;}int getHBinding(){return binding;}};
float screenRight=5000;
struct mDoGph_gInf_c {static float getMaxXF(){return screenRight;}};
J2DPane* first_picture_pane(J2DPane* p){return p;}
const char* label="Next";const char* text_box_string(J2DTextBox*){return label;}
struct CPaneMgr {J2DPane* pane=nullptr;J2DPane* getPanePtr(){return pane;}
 Vec getGlobalVtx(J2DPane* p,Mtx* m,int corner,bool,int){(*m)[0][0]=p->scale;return {(corner==0?p->left:p->right)+p->shift,0,0};}};
void shift_action_pane(J2DPane* p,float x,float){p->shift+=x;}
struct dMeter2Draw_c {CPaneMgr* mpButtonA=nullptr;CPaneMgr* mpTextA=nullptr;CPaneMgr* mpAText[5]={};};
'''
checks=r'''
int main(){
 for(float anchor:{400.f,700.f,1200.f,2000.f})
 for(float parent:{0.5f,1.f,1.25f,2.f})
 for(float size:{10.f,20.f,25.f,40.f})
 for(int binding:{HBIND_CENTER,HBIND_RIGHT,HBIND_LEFT}) {
  J2DPane button;button.left=anchor;button.right=anchor+88*parent;
  J2DTextBox text;text.left=anchor+20*parent;text.right=text.left+80*parent;
  text.scale=parent;text.size=size;text.binding=binding;
  CPaneMgr bm,tm;bm.pane=&button;tm.pane=&text;
  dMeter2Draw_c meter;meter.mpButtonA=&bm;meter.mpTextA=&tm;meter.mpAText[0]=&tm;
  separate_diamond_action_label(&meter);
  const float width=40*(size/20)*parent;
  const float factor=binding==HBIND_LEFT?0:binding==HBIND_RIGHT?1:0.5f;
  const float inkLeft=text.left+text.shift+(text.right-text.left-width)*factor;
  const float face=88*parent*action_prompt_layout::kFaceCoverage;
  const float faceRight=anchor+44*parent+face*0.5f;
  assert(inkLeft>=faceRight+face*0.30f-0.001f);
  const float shift=text.shift;separate_diamond_action_label(&meter);
  assert(std::abs(text.shift-shift)<0.001f);
  // Reproduce the clipped Next label at the right edge; the button stays put.
  screenRight=button.right;
  text.shift=0;
  separate_diamond_action_label(&meter);
  const float fittedLeft=text.left+text.shift+(text.right-text.left-width)*factor;
  assert(fittedLeft+width<=faceRight-face-face*0.30f+0.001f);
  assert(button.left==anchor&&button.right==anchor+88*parent);
  const float fittedShift=text.shift;separate_diamond_action_label(&meter);
  assert(std::abs(text.shift-fittedShift)<0.001f);
  screenRight=5000;
 }
}
'''
with tempfile.TemporaryDirectory(prefix='hud-label-geometry-') as d:
 p=Path(d);(p/'test.cpp').write_text(fixture+fn+checks)
 subprocess.run(['c++','-std=c++20','-I',str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: production A-label clearance at wide anchors, text scales and all text bindings')
