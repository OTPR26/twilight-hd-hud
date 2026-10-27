"""Exercise the production Game Over choice scope without changing game state."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
start = source.index('dMsgScrn3Select_c* s_gameOverChoices =')
end = source.index('void refresh_item_bank_cursor(', start)
production = source[start:end]
fixture = r'''
#include <cassert>
using HookAction=int;constexpr int HOOK_CONTINUE=0;struct ModContext{};
struct dMsgScrn3Select_c {void translate(float,float){}};
struct dMsgScrnExplain_c {int field_0x66;dMsgScrn3Select_c* mpSelect_c;};
namespace mods {template<class T>T arg(void* p,int){return static_cast<T>(p);}}
void* s_activeThreeSelectCursor=nullptr;void* s_activeThreeSelectFrame=nullptr;
bool s_goldenBugsChoiceDraw=false;
struct Insect {dMsgScrn3Select_c* mpSelect_c;};Insect* s_activeGoldenBugsDrawMenu=nullptr;
bool golden_bugs_explanation_active(Insect* p){return p!=nullptr;}
struct {struct {float mConfirmOptionPosX_4x3=0,mConfirmOptionPosY_4x3=0;}mInsectListScreen;}g_drawHIO;
int styled=0;void style_three_select_prompt(dMsgScrn3Select_c*){++styled;}
'''
checks = r'''
int main(){
 dMsgScrn3Select_c retry,other;
 // Both Retry and Return to Title use the same type-2 dialog. Repeat draws
 // and ensure the scope ends so subsequent Midna/map/Agitha choices work.
 for(int frame=0;frame<100;++frame){
  dMsgScrnExplain_c screen{2,&retry};
  before_game_over_explain_draw(nullptr,&screen,nullptr,nullptr);
  s_activeThreeSelectCursor=&retry;s_activeThreeSelectFrame=&retry;
  int count=styled;
  before_three_select_draw(nullptr,&retry,nullptr,nullptr);
  assert(styled==count&&!s_activeThreeSelectCursor&&!s_activeThreeSelectFrame);
  before_three_select_draw(nullptr,&other,nullptr,nullptr);assert(styled==count+1);
  after_game_over_explain_draw(nullptr,&screen,nullptr,nullptr);
  assert(s_gameOverChoices==nullptr);
  before_three_select_draw(nullptr,&retry,nullptr,nullptr);assert(styled==count+2);
 }
 for(int type:{0,1,3,4}){
  dMsgScrnExplain_c screen{type,&other};int count=styled;
  before_game_over_explain_draw(nullptr,&screen,nullptr,nullptr);
  before_three_select_draw(nullptr,&other,nullptr,nullptr);assert(styled==count+1);
  after_game_over_explain_draw(nullptr,&screen,nullptr,nullptr);
 }
 before_game_over_explain_draw(nullptr,nullptr,nullptr,nullptr);
 assert(s_gameOverChoices==nullptr);
}
'''
with tempfile.TemporaryDirectory(prefix='game-over-choice-') as directory:
    cpp=Path(directory)/'test.cpp';exe=Path(directory)/'test'
    cpp.write_text('#include <initializer_list>\n'+fixture+production+checks)
    subprocess.run(['c++','-std=c++17',str(cpp),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('PASS: Game Over choices remain native; other dialogs and subsequent draws retain styling')
