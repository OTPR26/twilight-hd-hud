"""Multibyte item cards must retain native fonts and parser metrics."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
start = source.index('void style_item_get_text(')
end = source.index('\nvoid after_item_help_message(', start)
fixture = r'''
#include <cassert>
#include <array>
struct JUTFont {
    int type;
    int getFontType() const { return type; }
};
JUTFont native{2}, subtitle{0};
int subtitleRequests = 0;
JUTFont* mDoExt_getSubFont() { ++subtitleRequests; return &subtitle; }
constexpr float kItemHelpBodyFontSize=16, kItemHelpRubyFontSize=8,
    kItemHelpBodyLineSpace=20;
struct Metrics {
    JUTFont* font=&native;
    float x=24, y=24, ruby=12, line=30, spacing=2, rubySpacing=1;
    JUTFont* getFont() { return font; }
    void setFont(JUTFont* value) { font=value; }
    void setFontSize(float a, float b) { x=a; y=b; }
    void setFontSizeX(float value) { x=value; }
    void setFontSizeY(float value) { y=value; }
    void setRubySize(float value) { ruby=value; }
    void setLineSpace(float value) { line=value; }
    void setCharSpace(float value) { spacing=value; }
    void setRubyCharSpace(float value) { rubySpacing=value; }
    bool operator==(const Metrics&) const = default;
};
using J2DTextBox=Metrics;
using jmessage_tReference=Metrics;
struct Pane {
    Metrics text;
    Metrics* getPanePtr() { return &text; }
};
struct dMsgScrnItem_c {
    JUTFont* field_0x54=&native;
    struct Size { float mSizeX=24, mSizeY=24; } mFontSize;
    float mRubySize=12, mLineSpace=30, mCharSpace=2, mRubyCharSpace=1;
    Pane* mpTm_c[7]{};
    Pane* mpTmr_c[7]{};
};
'''
checks = r'''
int main() {
    std::array<Pane,7> body, ruby;
    dMsgScrnItem_c screen;
    for(int i=0;i<7;++i) { screen.mpTm_c[i]=&body[i]; screen.mpTmr_c[i]=&ruby[i]; }
    Metrics reference, expected;
    for(int encoding : {1,2}) {
        native.type=encoding;
        for(int frame=0;frame<100;++frame) style_item_get_text(&screen,&reference);
        assert(subtitleRequests==0);
        assert(reference==expected);
        for(int i=0;i<7;++i) { assert(body[i].text==expected); assert(ruby[i].text==expected); }
        assert(screen.field_0x54==&native);
        assert(screen.mFontSize.mSizeX==24 && screen.mFontSize.mSizeY==24);
        assert(screen.mRubySize==12 && screen.mLineSpace==30);
        assert(screen.mCharSpace==2 && screen.mRubyCharSpace==1);
    }
    body[0].text.font=nullptr;
    style_item_get_text(&screen,&reference);
    assert(subtitleRequests==0 && reference==expected);
    body[0].text.font=&native; native.type=0;
    style_item_get_text(&screen,&reference);
    assert(subtitleRequests==1 && screen.field_0x54==&subtitle);
    assert(reference.font==&subtitle && reference.x==kItemHelpBodyFontSize);
    for(int i=0;i<7;++i) {
        assert(body[i].text.font==&subtitle && body[i].text.x==kItemHelpBodyFontSize);
        assert(ruby[i].text.font==&subtitle && ruby[i].text.x==kItemHelpRubyFontSize);
    }
    style_item_get_text(nullptr,&reference);
    screen.mpTm_c[0]=nullptr; screen.mpTmr_c[0]=nullptr;
    style_item_get_text(&screen);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-item-font-test-') as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(fixture + source[start:end] + checks)
    subprocess.run(['c++', '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
print('PASS: multibyte item text/ruby/parser preserved; Latin styling and missing panes handled')
