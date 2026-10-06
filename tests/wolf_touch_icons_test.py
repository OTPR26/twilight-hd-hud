"""Exercise touch-style transitions, fallback, and teardown without the game."""
from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
source = (root / 'src/wolf_touch_icons.cpp').read_text()
start = source.index('void update_wolf_touch_icons()')
end = source.rfind('\n}')
fixture = r'''
#include <cassert>
#include <string>
using UiStyleHandle=unsigned;
constexpr int MOD_OK=0, UI_SCOPE_TOUCH_CONTROLS=4;
void* mod_ctx=nullptr;
UiStyleHandle s_style=0;
std::string s_stylesheet;
bool s_failed=false;
bool enabled=true, player=true, wolf=true, event=false, pause=false, talk=false;
bool available=true;
int message=0, registrations=0, removals=0, warnings=0;
bool wolf_touch_icons_enabled(){return enabled;}
void* dComIfGp_getLinkPlayer(){return player ? &player : nullptr;}
struct daPy_py_c { static bool checkNowWolf(){return wolf;} };
bool dComIfGp_event_runCheck(){return event;}
bool dComIfGp_isPauseFlag(){return pause;}
int dComIfGp_getMesgStatus(){return message;}
bool dMsgObject_isTalkNowCheck(){return talk;}
bool prepare_icons(){s_stylesheet="icons";return available;}
struct Ui {
 int register_styles(void*,int scope,const char*,UiStyleHandle* handle){
  assert(scope==UI_SCOPE_TOUCH_CONTROLS);++registrations;*handle=1;return MOD_OK;
 }
 int unregister_styles(void*,UiStyleHandle handle){assert(handle==1);++removals;return MOD_OK;}
} ui;
auto* svc_ui=&ui;
struct Log {void warn(void*,const char*){++warnings;}} logService;
auto* svc_log=&logService;
'''
checks = r'''
int main(){
 update_wolf_touch_icons();assert(wolf_touch_icons_active());
 for(int frame=0;frame<100;++frame) update_wolf_touch_icons();
 assert(registrations==1);
 for(bool* suppression : {&event,&pause,&talk}) {
  *suppression=true;update_wolf_touch_icons();assert(!wolf_touch_icons_active());
  *suppression=false;update_wolf_touch_icons();assert(wolf_touch_icons_active());
 }
 message=1;update_wolf_touch_icons();assert(!wolf_touch_icons_active());
 message=0;update_wolf_touch_icons();assert(wolf_touch_icons_active());
 for(bool* requirement : {&enabled,&player,&wolf}) {
  *requirement=false;update_wolf_touch_icons();assert(!wolf_touch_icons_active());
  *requirement=true;update_wolf_touch_icons();assert(wolf_touch_icons_active());
 }
 shutdown_wolf_touch_icons();assert(!wolf_touch_icons_active());
 assert(removals==registrations && s_stylesheet.empty());
 available=false;
 for(int frame=0;frame<100;++frame) update_wolf_touch_icons();
 assert(!wolf_touch_icons_active() && warnings==1);
 shutdown_wolf_touch_icons();available=true;
 update_wolf_touch_icons();assert(wolf_touch_icons_active());
 shutdown_wolf_touch_icons();
}
'''
with tempfile.TemporaryDirectory(prefix='hud-wolf-touch-test-') as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(fixture + source[start:end] + checks)
    subprocess.run(['c++', '-std=c++20', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
print('PASS: Wolf/human, dialogue, pause, settings, failure fallback, and reload transitions')
