"""Wolf cutscene scent writes cannot discard an equipped third-slot item."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
fixture = r'''
#include <cassert>
using u8=unsigned char;
struct ModContext{};
enum HookAction {HOOK_CONTINUE,HOOK_SKIP_ORIGINAL};
constexpr int kZItemSlot=2;
constexpr u8 dItemNo_NONE_e=255,dItemNo_SMELL_YELIA_POUCH_e=176,dItemNo_SMELL_MEDICINE_e=181;
bool wolf=false,event=false;u8 previous=4,item=43;
struct daPy_py_c {static bool checkNowWolf(){return wolf;}};
bool dComIfGp_event_runCheck(){return event;}
u8 dComIfGs_getSelectItemIndex(int){return previous;}
u8 dComIfGs_getItem(u8,bool){return item;}
struct Args {int index;u8 slot;};
namespace mods {template<class T>T arg(void* p,int i){auto* a=static_cast<Args*>(p);return static_cast<T>(i?a->slot:a->index);}}
'''
checks = r'''
int main(){
 for(bool w:{false,true}) for(bool e:{false,true}) {
  wolf=w;event=e;
  for(int index:{0,1,2,3}) for(u8 value:{u8(255),u8(176),u8(181),u8(4),u8(20)}) {
   Args args{index,value};previous=4;item=43;
   bool block=w&&e&&index==2&&(value==255||(value>=176&&value<=181));
   assert(before_set_select_item_index(nullptr,&args,nullptr,nullptr)==(block?HOOK_SKIP_ORIGINAL:HOOK_CONTINUE));
   previous=255;
   assert(before_set_select_item_index(nullptr,&args,nullptr,nullptr)==HOOK_CONTINUE);
   previous=4;item=255;
   assert(before_set_select_item_index(nullptr,&args,nullptr,nullptr)==HOOK_CONTINUE);
  }
 }
}
'''
with tempfile.TemporaryDirectory(prefix='hud-wolf-slot-') as directory:
 p=Path(directory);(p/'test.cpp').write_text('#include <initializer_list>\n'+fixture+(root/'src/third_slot_cutscene.inc').read_text()+checks)
 subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: wolf-event third-slot protection and ordinary assignment/unequip paths')
