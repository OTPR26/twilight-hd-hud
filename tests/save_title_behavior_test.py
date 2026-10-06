"""Run the save-heading aspect correction across repeated layout transitions."""
from pathlib import Path
import subprocess
import tempfile

root=Path(__file__).resolve().parents[1]
source=(root/'src/save_screen.inc').read_text()
start=source.index('void position_save_question_title(')
end=source.index('\n}\n',start)+3
fixture=r'''
#include <cassert>
#include <cmath>
#include <initializer_list>
using f32=float;
struct Vec {float x,y;};
struct J2DPane {
    J2DPane* parent=nullptr;
    float sx=1,sy=1,tx=0,ty=0,width=608,height=78;
    J2DPane* getParentPane() {return parent;}
    float getScaleX() {return sx;} float getScaleY() {return sy;}
    void scale(float x,float y) {sx=x;sy=y;}
};
using J2DScreen=J2DPane;
Vec center(J2DPane* pane) {
    Vec point{pane->width*0.5f,pane->height*0.5f};
    for(auto* p=pane;p;p=p->parent) {point.x=point.x*p->sx+p->tx;point.y=point.y*p->sy+p->ty;}
    return point;
}
struct CPaneMgr {Vec getGlobalVtxCenter(J2DPane* p,bool,int) {return center(p);}};
void position_dmap_global_center(J2DPane* p,float x,float y) {
    auto old=center(p);float sx=1,sy=1;
    for(auto* parent=p->parent;parent;parent=parent->parent) {sx*=parent->sx;sy*=parent->sy;}
    p->tx+=(x-old.x)/sx;p->ty+=(y-old.y)/sy;
}
'''
checks=r'''
int main() {
    for(float resolution:{0.5f,1.0f,2.4f,3.0f}) {
        J2DPane outer;outer.sx=outer.sy=resolution;
        J2DScreen screen;screen.parent=&outer;screen.height=448;
        J2DPane title;title.parent=&screen;
        for(float aspect:{1.0f,1.31f,1.75f,1.0f,0.85f,1.31f}) {
            screen.sx=aspect;screen.tx=-608*(aspect-1)*0.5f;
            const float expectedY=center(&title).y;
            for(int frame=0;frame<100;++frame) {
                position_save_question_title(&screen,&title);
                assert(std::abs(center(&title).x-center(&screen).x)<0.001f);
                assert(std::abs(center(&title).y-expectedY)<0.001f);
                assert(std::abs(resolution*aspect*title.sx-resolution*title.sy)<0.001f);
            }
        }
        const float oldX=title.sx;
        screen.sx=0;
        position_save_question_title(&screen,&title);
        assert(title.sx==oldX);
    }
    position_save_question_title(nullptr,nullptr);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-save-title-test-') as directory:
    path=Path(directory);(path/'test.cpp').write_text(fixture+source[start:end]+checks)
    subprocess.run(['c++','-std=c++20',str(path/'test.cpp'),'-o',str(path/'test')],check=True)
    subprocess.run([str(path/'test')],check=True)
print('PASS: save heading proportions, centering, transition stability and zero-scale guard')
