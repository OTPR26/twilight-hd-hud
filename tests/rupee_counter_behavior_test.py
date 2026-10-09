"""Draw the native animated rupee value, including gains and spending."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
start = source.index('bool ensure_rupee_digit_textures()')
end = source.index('\nenum class CompanionPrompt', start)
fixture = r'''
#include <algorithm>
#include <cassert>
#include <vector>
using f32=float;
#define MULTI_CHAR(x) #x
#define JKR_NEW new
struct Vec { float x,y; };
namespace JUtility { struct TColor { void set(int,int,int,int) {} }; }
struct ResTIMG { int number; } textures[10];
struct Digit { int number,alpha; };
std::vector<Digit> drawn;
struct J2DPicture {
    ResTIMG* texture=nullptr;
    int alpha=255;
    bool visible=true;
    J2DPicture()=default;
    J2DPicture(ResTIMG* t):texture(t){}
    bool isVisible(){return visible;}
    int getAlpha(){return alpha;}
    Vec getGlbVtx(int i){return i==0 ? Vec{0,0} : Vec{15,18};}
    void changeTexture(ResTIMG* t,int){texture=t;}
    void setBlackWhite(JUtility::TColor,JUtility::TColor){}
    void setCornerColor(JUtility::TColor){}
    void setAlpha(int a){alpha=a;}
    void draw(float,float,float,float,bool,bool,bool){drawn.push_back({texture->number,alpha});}
} icon;
struct Screen { J2DPicture* search(const char*) { return &icon; } } screen;
struct dMeter2Draw_c { Screen* mpScreen=&screen; } drawMeter;
struct NativeMeter { int mRupeeNum=100; } nativeMeter;
NativeMeter* activeMeter=&nativeMeter;
NativeMeter* dMeter2Info_getMeterClass(){return activeMeter;}
int saved=110;
int dComIfGs_getRupee(){return saved;}
J2DPicture* as_picture(J2DPicture* p){return p;}
struct Archive {
    void* getResource(int,const char* name){return &textures[name[0]-'0'];}
} archive;
Archive* dComIfGp_getMain2DArchive(){return &archive;}
const char* dMeter2Info_getNumberTextureName(int n){static char name[2];name[0]='0'+n;return name;}
J2DPicture* s_rupeeDigitTex[4]{};
struct Scales {float rupees=1;} scales;
Scales hud_scales(){return scales;}
float rupee_digit_size(float s){return 18*s;}
float rupee_digit_step(float s){return 14*s;}
float rupee_digit_gap(float s){return 2*s;}
'''
checks = r'''
int render() {
    drawn.clear();draw_uniform_rupee_digits(&drawMeter);
    int value=0;
    for(auto d:drawn){value=value*10+d.number;assert(d.alpha==icon.alpha);}
    return value;
}
int main() {
    for(int i=0;i<10;++i)textures[i].number=i;
    saved=110;
    for(int value=100;value<=110;++value){nativeMeter.mRupeeNum=value;assert(render()==value);}
    saved=90;icon.alpha=127;
    for(int value=110;value>=90;--value){nativeMeter.mRupeeNum=value;assert(render()==value);}
    saved=1010;nativeMeter.mRupeeNum=999;
    assert(render()==999 && drawn.size()==3);
    nativeMeter.mRupeeNum=1000;assert(render()==1000 && drawn.size()==4);
    saved=990;nativeMeter.mRupeeNum=1001;assert(render()==1001 && drawn.size()==4);
    nativeMeter.mRupeeNum=999;assert(render()==999 && drawn.size()==3);
    activeMeter=nullptr;assert(render()==saved);
    icon.visible=false;render();assert(drawn.empty());
    icon.visible=true;icon.alpha=0;render();assert(drawn.empty());
    draw_uniform_rupee_digits(nullptr);
    for(auto* digit:s_rupeeDigitTex)delete digit;
}
'''
with tempfile.TemporaryDirectory(prefix='hud-rupee-counter-') as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(fixture + source[start:end] + checks)
    subprocess.run(['c++', '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
print('PASS: native counting value, gains/spending, digit transitions, fades and missing HUD')
