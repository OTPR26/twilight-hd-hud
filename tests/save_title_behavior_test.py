"""Check native title geometry through save-header transitions."""
from pathlib import Path
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
source=(root/'src/save_screen.inc').read_text()
start=source.index('void style_save_select_title(')
end=source.index('\n}\n',start)+3
fixture=r'''
#include <cassert>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#define MULTI_CHAR(x) #x
using u64=const char*;
struct J2DPane {
    float x=304,y=52,sx=1,sy=1;
    bool visible=true;
    void hide(){visible=false;}
    void show(){visible=true;}
};
struct J2DScreen {
    J2DPane decorations[4];
    J2DPane shadow;
    J2DPane* search(const char* tag){
        if(std::strstr(tag,"w_mgkage")) return &shadow;
        const char* names[]={"w_ti_w00","w_ti_w01","w_ti_w02","w_ti_w03"};
        for(unsigned i=0;i<4;++i) if(std::strstr(tag,names[i])) return &decorations[i];
        return nullptr;
    }
};
struct CPaneMgrAlpha {int alpha=255;J2DPane pane;void setAlpha(int a){alpha=a;}};
struct SaveSel {J2DScreen* Scr;};
struct dMenu_save_c {SaveSel mSaveSel;unsigned mHeaderTxtType=0;bool mHeaderAnmComplete=true;CPaneMgrAlpha* mpHeaderTxtPane[2];};
int frames=0;
void add_save_title_rules(J2DScreen*){++frames;}
'''
checks=r'''
int main(){
    J2DScreen screen;CPaneMgrAlpha first,second;
    dMenu_save_c menu{{&screen},0,true,{&first,&second}};
    for(unsigned current=0;current<2;++current){
        menu.mHeaderTxtType=current;
        for(int complete=0;complete<2;++complete){
            menu.mHeaderAnmComplete=complete;
            for(int frame=0;frame<100;++frame){
                for(auto& decoration:screen.decorations) decoration.hide();
                style_save_select_title(&menu);
                for(auto& decoration:screen.decorations) assert(decoration.visible);
                assert(!screen.shadow.visible);
                unsigned visible=complete?current:current^1;
                assert(first.alpha==(visible==0?255:0));
                assert(second.alpha==(visible==1?255:0));
                assert(first.pane.x==304 && first.pane.y==52);
                assert(second.pane.x==304 && second.pane.y==52);
                assert(first.pane.sx==1 && first.pane.sy==1);
                assert(second.pane.sx==1 && second.pane.sy==1);
            }
        }
    }
    menu.mHeaderTxtType=2;
    style_save_select_title(&menu);
    menu.mpHeaderTxtPane[0]=nullptr;menu.mHeaderTxtType=0;
    style_save_select_title(&menu);
    int before=frames;
    menu.mSaveSel.Scr=nullptr;style_save_select_title(&menu);
    style_save_select_title(nullptr);
    assert(frames==before);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-save-title-test-') as directory:
    path=Path(directory);(path/'test.cpp').write_text(fixture+source[start:end]+checks)
    subprocess.run(['c++','-std=c++20',str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS: native save-title geometry, header decoration visibility, cross-fades and missing panes')
