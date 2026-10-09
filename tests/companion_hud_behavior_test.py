"""Exercise companion shoulder artwork, scoped labels, and third-slot input routing."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
def callback(name):
    start = source.index('HookAction ' + name + '(')
    end = source.index('\n}\n', start) + 3
    return source[start:end]
fixture = r'''
#include "dual_screen.hpp"
#include "controller_prompts.hpp"
#include <string>
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
using namespace twilight_hd_hud;
using f32=float; using u32=unsigned; using u8=unsigned char;
struct ModContext {};
enum HookAction { HOOK_CONTINUE, HOOK_SKIP_ORIGINAL };
namespace mods {
template<class T> T arg(void* a,int i) {return *static_cast<T*>(static_cast<void**>(a)[i]);}
template<class T> T& arg_ref(void* a,int i) {return *static_cast<T*>(static_cast<void**>(a)[i]);}
}
ButtonLayout selected=ButtonLayout::Nintendo;
ButtonStyle style=ButtonStyle::Silver;
ButtonLayout twilight_hd_hud::button_layout(){return selected;}
ButtonStyle twilight_hd_hud::button_style(){return style;}
bool active=true, blocked=false, right=false;
bool query(){return active;}
DualScreenHost s_dualScreenHost{query};
struct {bool blocked(){return ::blocked;}} s_inputGate;
constexpr int kSdlRightShoulderButton=10;
constexpr u32 PAD_1=0;
enum class DusklightActionBind {UseSlotItem1=6};
bool physical_button_held(int b){assert(b==10);return right;}
struct dMeter2Draw_c {} draw;
struct Meter { dMeter2Draw_c* getMeterDrawPtr(){return &draw;} } meter;
Meter* dMeter2Info_getMeterClass(){return &meter;}

f32* s_companionWolfBlend=nullptr;
struct Log { void info(ModContext*,const char*){} } logger;
Log* svc_log=&logger;
ModContext* mod_ctx=nullptr;
struct ResTIMG{int digit;int width=96,height=64;} textures[10];
ResTIMG blankShoulder{-4},blankFace{-5,64,64};
ResTIMG collectionBackdrop{-6,512,512};bool backdropAvailable=true;
struct ResourceBuffer{bool background=false;int symbol=-1;};ResourceBuffer s_blankShoulderResources[5];
ResourceBuffer s_companionSymbolResources[2][4];
ResTIMG symbols[8];
ResourceBuffer s_collectBackgroundResource{true};
const ResTIMG* resource_texture(ResourceBuffer& r){return r.symbol>=0?&symbols[r.symbol]:r.background?(backdropAvailable?&collectionBackdrop:nullptr):&blankShoulder;}
const ResTIMG* styled_blank_face_button_texture(){return &blankFace;}
struct Port{void setup2D(){}} port;
Port* dComIfGp_getCurrentGrafPort(){return &port;}
struct Stamp{int digit,alpha;float x,width,height;bool flipX;};
std::vector<Stamp> stamps;
struct J2DPicture {
ResTIMG* texture;int alpha=0,uvResets=0,neutralResets=0,mirror=0;
J2DPicture(const ResTIMG* t):texture(const_cast<ResTIMG*>(t)){}
void changeTexture(const ResTIMG* t,int){texture=const_cast<ResTIMG*>(t);}
void* getTexture(int){return nullptr;}
void setTexCoord(void*,int,int m,bool){++uvResets;mirror=m;}
void setAlpha(u8 a){alpha=a;}
void draw(f32 x,f32,f32 w,f32 h,bool flipX,bool,bool){stamps.push_back({texture->digit,alpha,x,w,h,flipX});}
};
void set_neutral_picture_colors(J2DPicture* p){++p->neutralResets;}
#define JKR_NEW new
constexpr int BIND15=15,MIRROR0=0,J2DMirror_X=2;
std::array<J2DPicture*,2> s_companionShoulders{};
std::array<J2DPicture*,2> s_companionFaceBackings{};
J2DPicture* s_companionBackdrop=nullptr;
J2DPicture* s_companionSymbol=nullptr;
struct FontDrawContext {bool isTextureLoaded=true;};
struct JUTResFont {struct {u8 a=200;} mColor1;};
std::string nativeGlyphs;
struct ResFontDrawCharHook {
 static float g_orig(JUTResFont*,float,float,float,float,int c,bool,FontDrawContext*) {
  nativeGlyphs += static_cast<char>(c);return 5.0f;
 }
};
bool draw_companion_symbol(void*,void*,char);
bool draw_companion_shoulder_label(void*,void*,bool);
void configure_hd_picture(J2DPicture* p){p->setTexCoord(nullptr,BIND15,MIRROR0,false);set_neutral_picture_colors(p);p->setAlpha(255);}
'''
fixture += 'enum class CompanionPrompt' + source.split('enum class CompanionPrompt',1)[1].split('HookAction before_companion_item_hold(',1)[0]
fixture += callback('before_companion_item_hold')
for name in ('draw_companion_symbol', 'draw_companion_shoulder_label'):
    start=source.index('bool '+name+'(void* args',source.index('HookAction before_companion_item_hold('))
    end=source.index('\n}\n',start)+3
    fixture += source[start:end]
fixture += r'''
int main(){
 float x0=2,y0=3,x1=62,y1=63;bool enabled=true;int mask=15;
 void* plate[]={&x0,&y0,&x1,&y1,&enabled,&mask};
 dMeter2Draw_c* md=&draw;void* slots[]={&md};
 before_companion_slots(nullptr,slots,nullptr,nullptr);
 assert(before_companion_plate(nullptr,plate,nullptr,nullptr)==HOOK_SKIP_ORIGINAL);
 assert(stamps.size()==1&&stamps[0].digit==-4&&stamps[0].width==45);
 int character='I';float advance=10;void* fontArgs[8]{};fontArgs[5]=&character;
 assert(!companion_prompt_character(fontArgs,&advance)&&character=='R');
 assert(before_companion_plate(nullptr,plate,nullptr,nullptr)==HOOK_CONTINUE);
 for(int glyph=0;glyph<2;++glyph){character='I';advance=10;assert(companion_prompt_character(fontArgs,&advance)&&advance==0);}
 after_companion_prompt(nullptr,nullptr,nullptr,nullptr);
 before_companion_midna(nullptr,nullptr,nullptr,nullptr);
 before_companion_plate(nullptr,plate,nullptr,nullptr);
 assert(stamps.back().digit==-4&&stamps.back().width==45);
 assert(stamps.back().flipX&&!stamps[0].flipX);
 assert(stamps[0].width==stamps.back().width&&stamps[0].height==stamps.back().height);
 character='Z';assert(!companion_prompt_character(fontArgs,&advance)&&character=='L');
 float cx=0,cy=0,cw=60,ch=80;void* composite[]={nullptr,&cx,&cy,&cw,&ch};
 before_companion_composite(nullptr,composite,nullptr,nullptr);
 assert(cx>5.9f&&cx<6.1f&&cy>7.9f&&cy<8.1f&&cw==48&&ch==64);
 after_companion_prompt(nullptr,nullptr,nullptr,nullptr);
 character='Z';assert(!companion_prompt_character(fontArgs,&advance)&&character=='Z');
 float wolf=1;s_companionWolfBlend=&wolf;
 before_companion_slots(nullptr,slots,nullptr,nullptr);
 assert(s_companionPrompt==CompanionPrompt::None);
 s_companionWolfBlend=nullptr;active=false;
 before_companion_slots(nullptr,slots,nullptr,nullptr);
 assert(s_companionPrompt==CompanionPrompt::None);active=true;
 int action=6;u32 player=0;bool result=false;void* input[]={&action,&player};
 right=true;assert(before_companion_item_hold(nullptr,input,&result,nullptr)==HOOK_SKIP_ORIGINAL&&result);
 blocked=true;before_companion_item_hold(nullptr,input,&result,nullptr);assert(!result);
 blocked=false;action=7;result=true;
 assert(before_companion_item_hold(nullptr,input,&result,nullptr)==HOOK_CONTINUE&&result);
 int faceIndex=2;float faceX=10,faceY=20,faceSize=76;
 void* circleArgs[]={&md,&faceIndex,&faceX,&faceY,&faceSize};
 for(faceIndex=2;faceIndex<=3;++faceIndex){
 assert(before_companion_circle(nullptr,circleArgs,nullptr,nullptr)==HOOK_SKIP_ORIGINAL);
 assert(stamps.back().digit==-5&&stamps.back().width==76&&stamps.back().height==76);
 }
 faceIndex=0;assert(before_companion_circle(nullptr,circleArgs,nullptr,nullptr)==HOOK_CONTINUE);
 active=false;faceIndex=2;assert(before_companion_circle(nullptr,circleArgs,nullptr,nullptr)==HOOK_CONTINUE);
 float backdropWidth=480,backdropHeight=420;void* backdropArgs[]={&backdropWidth,&backdropHeight};
 auto count=stamps.size();
 assert(before_companion_backdrop(nullptr,backdropArgs,nullptr,nullptr)==HOOK_CONTINUE&&stamps.size()==count);
 active=true;
 for(int frame=0;frame<2;++frame){
 assert(before_companion_backdrop(nullptr,backdropArgs,nullptr,nullptr)==HOOK_SKIP_ORIGINAL);
 assert(stamps.back().digit==-6&&stamps.back().alpha==255&&stamps.back().x==0);
 assert(stamps.back().width==backdropWidth&&stamps.back().height==backdropHeight);
 backdropWidth=720;backdropHeight=480;
 }
 backdropAvailable=false;count=stamps.size();
 assert(before_companion_backdrop(nullptr,backdropArgs,nullptr,nullptr)==HOOK_CONTINUE&&stamps.size()==count);
 backdropAvailable=true;backdropWidth=0;
 assert(before_companion_backdrop(nullptr,backdropArgs,nullptr,nullptr)==HOOK_CONTINUE&&stamps.size()==count);
 JUTResFont font;JUTResFont* fontPtr=&font;
 float fx=10,fy=20,fw=11,fh=13;bool subsequent=true;
 FontDrawContext* context=nullptr;
 void* promptArgs[]={&fontPtr,&fx,&fy,&fw,&fh,&character,&subsequent,&context};
 for(int color=0;color<2;++color)for(int index=0;index<4;++index){
  s_companionSymbolResources[color][index].symbol=color*4+index;
  symbols[color*4+index]={100+color*4+index,32,32};
 }
 for(int layout=0;layout<=14;++layout){
  selected=static_cast<ButtonLayout>(layout);
  before_companion_face_labels(nullptr,nullptr,nullptr,nullptr);
  for(char action : {'X','Y'}){
   character=action;auto previous=stamps.size();
   bool handled=companion_prompt_character(promptArgs,&advance);
   if(is_universal_layout(selected))assert(handled&&advance==0&&stamps.size()==previous);
   else if(is_playstation_layout(selected)){
    assert(handled&&stamps.back().digit==100+companion_face_symbol_index(selected,action));
    assert(stamps.back().alpha==200);
   }else assert(!handled&&character==face_letter_for_action(selected,action));
  }
  after_companion_prompt(nullptr,nullptr,nullptr,nullptr);
  for(bool left : {false,true}){
   nativeGlyphs.clear();character=left?'Z':'I';
   bool handled=draw_companion_shoulder_label(promptArgs,&advance,left);
   const char* label=companion_shoulder_label(selected,left);
   if(label[1])assert(handled&&nativeGlyphs==label&&advance==10);
   else assert(!handled&&character==label[0]);
  }
 }
 selected=ButtonLayout::PlayStationFlippedBotw;style=ButtonStyle::PlayStationColors;
 before_companion_face_labels(nullptr,nullptr,nullptr,nullptr);
 character='X';assert(companion_prompt_character(promptArgs,&advance));
 assert(stamps.back().digit==104);
 after_companion_prompt(nullptr,nullptr,nullptr,nullptr);
 character='X';assert(!companion_prompt_character(promptArgs,&advance));
 delete s_companionSymbol;
 delete s_companionBackdrop;
 for(auto* p:s_companionShoulders)delete p;
 for(auto* p:s_companionFaceBackings)delete p;
}
'''
with tempfile.TemporaryDirectory() as directory:
    cpp=Path(directory)/'test.cpp';binary=Path(directory)/'test'
    cpp.write_text(fixture)
    subprocess.run(['c++','-std=c++20','-I',str(root/'src'),'-I',str(root.parent/'dusklight-sdk/sdk/include'),str(cpp),'-o',str(binary)],check=True)
    subprocess.run([str(binary)],check=True)
print('PASS: actual L/R artwork, prompt sizing, fourth-slot preservation, and normal-host fallback')
