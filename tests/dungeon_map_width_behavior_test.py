"""Dungeon-map decoration covers the viewport without stretching the map."""
from pathlib import Path
import subprocess
import tempfile
import re
root=Path(__file__).resolve().parents[1]
source=(root/'src/dungeon_map_screen.inc').read_text().split('void extend_dmap_background(',1)[1]
source=re.sub(r"MULTI_CHAR\('([^']+)'\)", lambda m: '::tag("'+m[1]+'")', source)
fixture=r'''
#include <array>
#include <cassert>
#include <cmath>
using u64=unsigned long long;
constexpr u64 tag(const char* p){u64 t=0;while(*p)t=(t<<8)|static_cast<unsigned char>(*p++);return t;}
#define MULTI_CHAR(x) tag(x)
float viewport=800,safe=720,minX=-96,safeMin=-56;
struct mDoGph_gInf_c {
 static float hudAspectScaleUp;
 static float getMinXF(){return minX;}static float getSafeMinXF(){return safeMin;}
 static float getWidthF(){return viewport;}
};
float mDoGph_gInf_c::hudAspectScaleUp=1;
struct Pane {float x=0,y=10,w=608,h=448;
 void move(float a,float b){x=a;y=b;}void resize(float a,float b){w=a;h=b;}
 float getTranslateY(){return y;}float getHeight(){return h;}
};
struct Screen {std::array<Pane,7> p{};Pane* search(u64 t){
 if(t==tag("hd_dbg0"))return &p[0];if(t>=tag("hd_drl0")&&t<tag("hd_drl0")+6)return &p[t-tag("hd_drl0")+1];return nullptr;}};
struct dMenu_DmapBg_c {Screen* mBaseScreen;};
'''
checks=r'''
int main(){
 for(float aspect:{4.f/3,16.f/9,20.f/9,21.f/9,32.f/9}) {
  viewport=448*aspect;safe=viewport*0.9f;minX=(608-viewport)/2;safeMin=minX+(viewport-safe)/2;
  mDoGph_gInf_c::hudAspectScaleUp=safe/608;
  Screen screen;dMenu_DmapBg_c map{&screen};extend_dmap_background(&map);
  for(auto& p:screen.p){
   assert(std::abs(safeMin+p.x*safe/608-minX)<0.001f);
   assert(std::abs(p.w*safe/608-viewport)<0.001f);
   assert(p.y==10&&p.h==448);
  }
 }
 extend_dmap_background(nullptr);
}
'''
with tempfile.TemporaryDirectory(prefix='hud-map-width-') as directory:
 p=Path(directory);(p/'test.cpp').write_text(fixture+'void extend_dmap_background('+source+checks)
 subprocess.run(['c++','-std=c++20',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('PASS: dungeon-map background and rules cover 4:3 through 32:9 viewports')
