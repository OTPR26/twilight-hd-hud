"""Universal item instructions preserve native reader layout and bounded buffers."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
source=(root/'src/companion_item_reader.inc').read_text()
fixture=r'''
#include "companion_item_help.hpp"
#include <cassert>
#include <cstring>
using namespace twilight_hd_hud;
using u32=unsigned;
struct ModContext{};enum HookAction{HOOK_CONTINUE};
namespace mods{template<class T>T arg(void* a,int i){return *static_cast<T*>(static_cast<void**>(a)[i]);}}
struct Host{bool enabled=true;bool active(){return enabled;}}s_dualScreenHost;
'''
checks=r'''
int main(){
 const char* original="Set it to \x02\x07 or \x02\x06 and press \x02\x01. Hold \x02\x03 down.";
 assert(companion_item_help(original)=="Equip it in any item slot and press the action button. Hold the right stick down.");
 assert(companion_item_help("Set it to \x02\x06 or \x02\x07.")=="Equip it in any item slot.");
 assert(companion_item_help("Set this to and\ndrink it with \x02\x07 or \x02\x06 to restore 8 hearts.")=="Equip this in any item slot, then drink it to restore 8 hearts.");
 assert(companion_item_help("Normal bombs.\n\n\x02\x2b Lift one with \x02\x06 or \x02\x07 and press \x02\x01 to\n  set it down. Press \x02\x01 while running\n to throw.\n\x02\x2b Set the bow.")=="Normal bombs.\n\x02\x2b Use the equipped item to lift one and press the action button to set it down. Press the action button while running to throw.\n\x02\x2b Set the bow.");
 assert(companion_item_help("Normal bombs. You can carry 30 bombs.\n\x02\x2b Lift one with \x02\x06 or \x02\x07 and press \x02\x01 to\n\x02\x2c set it down. Press \x02\x01 while running\n\x02\x2c to throw.\n\x02\x2b Set the Hero's Bow and press \x02\x05 on the\n\x02\x2c Items screen to make bomb arrows.")=="Normal bombs. You can carry 30 bombs.\n\x02\x2b Use the equipped item to lift one and press the action button to set it down. Press the action button while running to throw.\n\x02\x2b Set the Hero's Bow and press the shoulder button on the Items screen to make bomb arrows.");
 assert(companion_item_help("Press \x02\x01. \x02\x20")=="Press the action button. \x02\x20");
 assert(companion_item_help("Iron boots that make you heavier.\n\n\x02\x2b Wear them with \x02\x07 or \x02\x06.")=="Iron boots that make you heavier.\n\x02\x2b Equip them in any item slot to wear them.");
 assert(companion_item_help("Utilisez \x02\x06 ou \x02\x07.").empty());
 assert(companion_item_help("Set it to \x02\x06. \x02\x22")=="Set it to an item button. \x02\x22");
 char buffer[256];strcpy(buffer,original);char* text=buffer;int cap=sizeof(buffer);u32 id=0x265;void* receiver=nullptr;
 void* args[]={&receiver,&id,&text,&cap};
 after_companion_item_text(nullptr,args,nullptr,nullptr);assert(!strcmp(buffer,original));
 before_companion_item_info(nullptr,nullptr,nullptr,nullptr);
 after_companion_item_text(nullptr,args,nullptr,nullptr);assert(!strcmp(buffer,companion_item_help(original).c_str()));
 strcpy(buffer,original);cap=strlen(original)+1;
 after_companion_item_text(nullptr,args,nullptr,nullptr);assert(!strcmp(buffer,original));
 cap=sizeof(buffer);id=0x165;
 after_companion_item_text(nullptr,args,nullptr,nullptr);assert(!strcmp(buffer,original));
 memset(buffer,'a',sizeof(buffer));id=0x265;
 after_companion_item_text(nullptr,args,nullptr,nullptr);assert(buffer[255]=='a');
 after_companion_item_info(nullptr,nullptr,nullptr,nullptr);assert(s_companionItemInfoDepth==0);
 s_dualScreenHost.enabled=false;before_companion_item_info(nullptr,nullptr,nullptr,nullptr);assert(s_companionItemInfoDepth==0);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-item-reader-') as directory:
 p=Path(directory);(p/'test.cpp').write_text(fixture+source+checks)
 subprocess.run(['c++','-std=c++20','-I',str(root/'src'),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: universal instructions, other languages/icons, bounds and reader-only scope')
