"""Exercise the production fallback with native and extended skill counts."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
start = source.index('bool hidden_skills_use_native_menu(')
end = source.index('\n}\n', start) + 3
fixture = r'''
#include <cassert>
#include <map>
#include <cstdint>
using u32=unsigned; using u64=uint64_t; using f32=float;
constexpr u64 tag(const char* p){u64 v=0;while(*p)v=v*131+*p++;return v;}
#define MULTI_CHAR(x) tag(#x)
#define JKR_NEW new
namespace JGeometry { template<class T> struct TBox2 {TBox2(T,T,T,T){}}; }
struct J2DPane {
    u64 tag; bool visible=true; std::map<u64,J2DPane*> children;
    J2DPane(u64 t=0,JGeometry::TBox2<float> b={0,0,0,0}):tag(t){}
    ~J2DPane(){for(auto& p:children)delete p.second;}
    J2DPane* search(u64 t){auto it=children.find(t);return it==children.end()?nullptr:it->second;}
    void hide(){visible=false;} void show(){visible=true;}
    void appendChild(J2DPane* p){children[p->tag]=p;}
};
struct dSv_event_flag_c { inline static u32 saveBitLabels[345]{}; };
unsigned nativeCount=0, eventCalls=0;
bool dComIfGs_isEventBit(u32){return eventCalls++ % 7 < nativeCount;}
struct dMenu_Skill_c {
    J2DPane *mpMenuScreen=nullptr, *mpParent=nullptr, *mpIconScreen=nullptr;
    J2DPane* mpLetterParent[7]{}; void* mpDrawCursor=nullptr;
    unsigned mTotalSkillNum=0, count=0, mProcess=0, mIndex=0;
    unsigned labels=0,cursors=0,queries=0;
    unsigned getSkillNum(){++queries;return count;}
    void setAButtonString(unsigned x){assert(x==0x40c);++labels;}
    void setCursorPos(){assert(mIndex<7);++cursors;}
};
struct Input {int value=0;} s_hiddenSkillInput;
'''
fixture += source[start:end]
fixture += r'''
int main(){
    assert(!hidden_skills_use_native_menu(nullptr));
    for(unsigned n=0;n<=7;++n){
        J2DPane screen,parent,icons;
        dMenu_Skill_c m; m.mpMenuScreen=&screen; m.mpParent=&parent; m.mpIconScreen=&icons;
        nativeCount=n;eventCalls=0;m.count=m.mTotalSkillNum=n;
        assert(!hidden_skills_use_native_menu(&m));
        assert(screen.children.empty());
        m.count=n+1; // Includes partially unlocked native skill sets.
        assert(hidden_skills_use_native_menu(&m));
        auto queries=m.queries;m.count=n;
        assert(hidden_skills_use_native_menu(&m));assert(m.queries==queries);
        assert(m.labels==0 && m.cursors==0); // Untouched when no HD overlay existed.
    }
    J2DPane screen,parent,icons;
    auto* overlay=new J2DPane(MULTI_CHAR('hd_hsov'));screen.appendChild(overlay);
    dMenu_Skill_c m;m.mpMenuScreen=&screen;m.mpParent=&parent;m.mpIconScreen=&icons;
    parent.hide();icons.hide();m.count=7;m.mTotalSkillNum=8;m.mIndex=7;
    nativeCount=7;eventCalls=0;s_hiddenSkillInput.value=1;
    assert(hidden_skills_use_native_menu(&m));
    assert(!overlay->visible && parent.visible && icons.visible);
    assert(m.labels==1 && m.cursors==0 && s_hiddenSkillInput.value==0);
}
'''
# Every custom Skills entry point must yield before changing native state.
for fn in ['ensure_hidden_skills_overlay','before_skill_wait_move','after_skill_create','after_skill_move','before_skill_draw']:
    body=source.split(fn+'(',1)[1].split('\n}\n',1)[0]
    assert 'hidden_skills_use_native_menu(menu)' in body, fn
with tempfile.TemporaryDirectory() as tmp:
    cpp=Path(tmp)/'test.cpp';cpp.write_text(fixture);exe=Path(tmp)/'test'
    subprocess.run(['c++','-std=c++20','-Wno-multichar',str(cpp),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('PASS: added-skill detection, partial unlocks, menu-session latch, and native restoration')
