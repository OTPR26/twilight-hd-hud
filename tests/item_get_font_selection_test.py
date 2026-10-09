"""Exercise item-card font selection without changing native text decoding."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/font_override.cpp').read_text()
start = source.index('bool draw_font_override(')
end = source.index('\n}  // namespace twilight_hd_hud', start)
fixture = r'''
#include "font_atlas.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
using namespace twilight_hd_hud;
using f32 = float;
struct ModContext {};
struct ResFONT {};
struct JKRHeap { static JKRHeap* getRootHeap() { return nullptr; } };
#define JKR_NEW_ARGS(heap, alignment) new
#define JKR_DELETE(value) delete value
namespace JUtility {
struct TColor { unsigned char r,g,b,a; TColor(int r=1,int g=2,int b=3,int a=255):r(r),g(g),b(b),a(a){} };
}
struct JUTFont { struct TWidth { int field_0x0=0, field_0x1=64; }; };
struct JUTResFont : JUTFont {
    int type=0;
    bool mFixed=false;
    int mFixedWidth=64;
    JUtility::TColor mColor1,mColor2,mColor3,mColor4;
    JUTResFont()=default;
    JUTResFont(const ResFONT*, JKRHeap*) {}
    bool isValid() { return true; }
    int getFontType() { return type; }
    int getCellWidth() { return 128; }
    int getCellHeight() { return 128; }
    int getAscent() { return 104; }
    int getDescent() { return 24; }
    void getWidthEntry(int, TWidth* width) { *width={0,64}; }
};
struct FontDrawContext { bool isTextureLoaded=true; };
using FontDrawOriginal = f32 (*)(JUTResFont*,f32,f32,f32,f32,int,bool,FontDrawContext*);
enum class TextFont { Original,ZenKakuGothicNew,MPlus2,FiraSans,AlegreyaSansMedium };
namespace mods {
template<class T> T arg(void* args,int index) { return *static_cast<T*>(static_cast<void**>(args)[index]); }
}
TextFont s_activeFont=TextFont::Original;
JUTResFont message,subtitle,selected,ruby;
JUTResFont *s_messageFont=&message,*s_replacement=&selected;
int s_itemPromptDepth=0,s_mapDepth=0;
bool s_loggedDraw=true,available=true;
bool s_preserveMapFont=false;
bool ensure_replacement() { return available; }
JUTResFont* mDoExt_getMesgFont() { return &message; }
JUTResFont* mDoExt_getRubyFont() { return &ruby; }
struct Log { void info(ModContext*,const char*) {} } logService;
Log* svc_log=&logService;
ModContext* mod_ctx=nullptr;
JUTResFont* drawn=nullptr;
float drawnScaleY=0;
f32 draw(JUTResFont* font,f32,f32,f32,f32 sy,int,bool,FontDrawContext*) {
    drawn=font;drawnScaleY=sy;return 0;
}
'''
checks = r'''
int main() {
    JUTResFont* input=&subtitle;
    float x=0,y=0,sx=16,sy=16,result=-1;
    int code='A';bool subsequent=true;FontDrawContext context;auto* contextPtr=&context;
    void* args[]={&input,&x,&y,&sx,&sy,&code,&subsequent,&contextPtr};
    s_itemPromptDepth=1;
    for(auto choice : {TextFont::ZenKakuGothicNew,TextFont::MPlus2,
                       TextFont::FiraSans,TextFont::AlegreyaSansMedium}) {
        s_activeFont=choice;drawn=nullptr;context.isTextureLoaded=true;
        assert(draw_font_override(args,&result,draw));
        assert(drawn==&selected && !context.isTextureLoaded);
        float scale=choice==TextFont::FiraSans ? font_atlas::firaOpticalScale :
            choice==TextFont::AlegreyaSansMedium ? font_atlas::alegreyaOpticalScale : font_atlas::opticalScale;
        assert(std::fabs(drawnScaleY-sy*scale)<0.001f);
        assert(result==8);
        code=0xe9;assert(draw_font_override(args,&result,draw)); // French accented text.
        code='A';
        for(int encoding : {1,2}) {
            subtitle.type=encoding;drawn=nullptr;
            assert(!draw_font_override(args,&result,draw) && drawn==nullptr);
        }
        subtitle.type=0;
    }
    s_activeFont=TextFont::Original;
    assert(!draw_font_override(args,&result,draw));
    s_activeFont=TextFont::FiraSans;available=false;
    assert(!draw_font_override(args,&result,draw));
    available=true;s_itemPromptDepth=0;
    assert(!draw_font_override(args,&result,draw)); // Subtitle outside item cards.
    input=&message;assert(draw_font_override(args,&result,draw));
    s_mapDepth=1;
    for(auto choice : {TextFont::ZenKakuGothicNew,TextFont::MPlus2,
                       TextFont::FiraSans,TextFont::AlegreyaSansMedium}) {
        s_activeFont=choice;
        for(auto* nativeFont : {&message,&ruby}) {
            input=nativeFont;
            for(int glyph : {int('A'),0xe9,int('i')}) {
                code=glyph;drawn=nullptr;
                assert(draw_font_override(args,&result,draw) && drawn==&selected);
                assert(result==8); // Native advances remain unchanged on maps.
            }
            for(int encoding : {1,2}) {
                nativeFont->type=encoding;
                assert(!draw_font_override(args,&result,draw));
            }
            nativeFont->type=0;
        }
        available=false;assert(!draw_font_override(args,&result,draw));available=true;
    }
    s_activeFont=TextFont::Original;
    for(auto* nativeFont : {&message,&ruby}) {
        input=nativeFont;assert(!draw_font_override(args,&result,draw));
    }
    s_activeFont=TextFont::FiraSans;s_preserveMapFont=true;code='A';
    for(auto* nativeFont : {&message,&ruby}) {
        input=nativeFont;drawn=nullptr;
        assert(!draw_font_override(args,&result,draw) && drawn==nullptr);
    }
    s_mapDepth=0;input=&message;
    assert(draw_font_override(args,&result,draw));
    s_preserveMapFont=false;
    s_mapDepth=0;s_itemPromptDepth=1;s_activeFont=TextFont::FiraSans;
    for(int nativeCode : {0xb2,0xb3,0x81,0x3042}) {
        code=nativeCode;assert(!draw_font_override(args,&result,draw));
    }
    code='A';input=nullptr;assert(!draw_font_override(args,&result,draw));
}
'''
with tempfile.TemporaryDirectory(prefix='hud-item-font-selection-') as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(fixture + source[start:end] + checks)
    subprocess.run(['c++', '-std=c++20', '-I', str(root / 'src'),
                    str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
print('PASS: selected item/map fonts, French accents and native Japanese fallback')
