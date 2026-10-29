"""Exercise production item-card geometry and live arrow counts with mocked host data."""
from pathlib import Path
import re,subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'src/item_slot_hooks.cpp').read_text()
def section(a,b):return s[s.index(a):s.index(b,s.index(a))]
ammo=section('bool z_item_has_ammo(', 'bool ensure_z_item_num_textures(')
icons=section('struct ItemHelpIconPosition {','HookAction before_item_help_out_font_draw(')
items=sorted(set(re.findall(r'\bdItemNo_\w+_e\b',ammo+icons)))
fixture=r'''
#include <array>
#include <algorithm>
#include <cassert>
using u8=unsigned char;using s16=short;using f32=float;
'''+ 'enum {'+','.join(items)+'};\n'+r'''
constexpr u8 kZItemSlot=2;
bool wolf=false;struct daPy_py_c {static bool checkNowWolf(){return wolf;}};
u8 arrows=12,arrowMax=60,selected[3]={dItemNo_BOW_e,dItemNo_MAP_e,dItemNo_BOMB_ARROW_e};
s16 counts[3]={12,40,9};int maxima[3]={60,60,30};
u8 dComIfGp_getSelectItem(u8 slot){return selected[slot];}
u8 dComIfGs_getArrowNum(){return arrows;}u8 dComIfGs_getArrowMax(){return arrowMax;}
u8 dComIfGs_getPachinkoNum(){return 5;}u8 dComIfGs_getPachinkoMax(){return 50;}
s16 dComIfGp_getSelectItemNum(u8 slot){return counts[slot];}
int dComIfGp_getSelectItemMaxNum(u8 slot){return maxima[slot];}
struct dMeter2Draw_c {int calls[2]={};u8 value[2]={},maximum[2]={};
 void setItemNum(u8 slot,u8 count,u8 max){++calls[slot];value[slot]=count;maximum[slot]=max;}};
struct ResTIMG {int width=128,height=64;} texture;
const ResTIMG* styled_r_button_texture(){return &texture;}
struct COutFontSet_c {int type=0;float mPosX=40,mPosY=80,mSizeX=20,mSizeY=20;
 int getType(){return type;}float getPosX(){return mPosX;}float getPosY(){return mPosY;}
 float getSizeX(){return mSizeX;}float getSizeY(){return mSizeY;}};
struct OutFont {std::array<COutFontSet_c*,35> mpOfs{};};
struct dMsgScrnItem_c {OutFont* mpOutFont=nullptr;u8 mItemIndex=dItemNo_MAP_e;bool skill=false;bool isOugiID(){return skill;}};
'''
checks=r'''
int main(){
 dMeter2Draw_c meter;refresh_face_arrow_counts(&meter);
 assert(meter.value[0]==12&&meter.calls[1]==0);
 arrows=17;refresh_face_arrow_counts(&meter);assert(meter.value[0]==17);
 arrowMax=100;arrows=100;refresh_face_arrow_counts(&meter);
 assert(meter.value[0]==100&&meter.maximum[0]==100);
 selected[1]=dItemNo_BOMB_ARROW_e;counts[1]=8;
 refresh_face_arrow_counts(&meter);assert(meter.value[1]==8&&meter.maximum[1]==100);
 arrows=3;refresh_face_arrow_counts(&meter);assert(meter.value[1]==3);
 u8 count=0,maximum=0;assert(z_item_ammo_values(dItemNo_BOMB_ARROW_e,count,maximum));
 assert(count==3&&maximum==100);
 const int calls=meter.calls[0];wolf=true;refresh_face_arrow_counts(&meter);
 assert(meter.calls[0]==calls);wolf=false;
 // Normal bomb and skill cards now align action/back caps, not only Bomblings.
 for(int type:{0,1,5,6,7,67}) for(float scale:{0.5f,1.f,1.25f,1.5f}) {
  COutFontSet_c icon;icon.type=type;icon.mSizeX*=scale;icon.mSizeY*=scale;
  const float x=icon.mPosX,y=icon.mPosY,w=icon.mSizeX,h=icon.mSizeY;
  OutFont out;out.mpOfs[0]=&icon;dMsgScrnItem_c card;card.mpOutFont=&out;
  lift_item_get_assignment_icons(&card);assert(icon.mPosY<y);
  restore_item_get_assignment_icons();assert(icon.mPosX==x&&icon.mPosY==y&&icon.mSizeX==w&&icon.mSizeY==h);
  if (type==67) {
   card.mItemIndex=dItemNo_BOW_e;
   lift_item_get_assignment_icons(&card);
   assert(icon.mPosX==x&&icon.mPosY==y&&icon.mSizeX==w&&icon.mSizeY==h);
   restore_item_get_assignment_icons();
  }
 }
 // Adjacent skill lines must retain separation and restore their original geometry.
 for(float scale:{0.5f,1.f,1.25f,1.5f}) {
  COutFontSet_c shoulder,face;shoulder.type=3;face.type=0;
  shoulder.mSizeX=shoulder.mSizeY=face.mSizeX=face.mSizeY=20*scale;
  shoulder.mPosY=80;face.mPosY=80+24*scale;
  OutFont out;out.mpOfs[0]=&shoulder;out.mpOfs[1]=&face;
  dMsgScrnItem_c card;card.mpOutFont=&out;card.skill=true;
  lift_item_get_assignment_icons(&card);
  assert(shoulder.mPosY+shoulder.mSizeY < face.mPosY);
  assert(shoulder.mSizeY==20*scale && face.mSizeY<20*scale);
  restore_item_get_assignment_icons();
  assert(shoulder.mPosY==80 && face.mPosY==80+24*scale);
  assert(shoulder.mSizeY==20*scale && face.mSizeY==20*scale);
  card.skill=false;lift_item_get_assignment_icons(&card);
  assert(shoulder.mPosY==80 && shoulder.mSizeY==20*scale);
  restore_item_get_assignment_icons();
 }
 COutFontSet_c next;next.type=0x47;OutFont out;out.mpOfs[0]=&next;
 dMsgScrnItem_c card;card.mpOutFont=&out;lift_item_get_assignment_icons(&card);
 assert(next.mPosX==40&&next.mPosY==80);restore_item_get_assignment_icons();
}
'''
with tempfile.TemporaryDirectory(prefix='hud-item-card-ammo-') as directory:
 p=Path(directory);(p/'test.cpp').write_text(fixture+ammo+icons+checks)
 subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
assert 'refresh_face_arrow_counts(meter);' in section('HookAction before_meter_screen_draw(', 'void after_meter_draw(')
print('PASS: inline action baseline/restoration and live bow/combo ammo updates across slots')
