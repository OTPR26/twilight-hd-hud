"""Verify opting out avoids host queries and clears prior companion routing."""
from pathlib import Path
import subprocess
import tempfile
root = Path(__file__).resolve().parents[1]
source = (root / 'src/item_slot_hooks.cpp').read_text()
start = source.index('void resolve_dual_screen_host() {')
end = source.index('\n}\n', start) + 3
fixture = r'''
#include "dual_screen.hpp"
#include <cassert>
#include <string>
using namespace twilight_hd_hud;
using f32 = float;
struct dMeter2Draw_c {};
DualScreenHost s_dualScreenHost;
bool (*s_companionItemUsable)(dMeter2Draw_c*, int) = nullptr;
f32 (*s_companionMeasureText)(f32, const char*) = nullptr;
f32* s_companionWolfBlend = nullptr;
bool enabled = true;
bool dual_screen_compatibility_enabled() { return enabled; }
int queries = 0;
bool host_active() { return false; }
unsigned bits() { return 0; }
void* resolve_host_symbol(const char* name) {
    ++queries;
    std::string n(name);
    if (n == "dusk::dualscreen::hudOnCompanion") return reinterpret_cast<void*>(host_active);
    if (n == "dusk::companion::slotTriggerBits" || n == "dusk::companion::slotHoldBits")
        return reinterpret_cast<void*>(bits);
    return nullptr;
}
struct Log { void info(void*, const char*) {} } logger;
Log* svc_log = &logger;
void* mod_ctx = nullptr;
'''
fixture += source[start:end]
fixture += r'''
int main() {
    resolve_dual_screen_host();
    assert(queries > 0 && s_dualScreenHost.owns_item_slots());
    assert(!s_dualScreenHost.active());
    enabled = false; queries = 0;
    resolve_dual_screen_host();
    assert(queries == 0);
    assert(!s_dualScreenHost.owns_item_slots() && !s_dualScreenHost.active());
    assert(!s_companionItemUsable && !s_companionMeasureText && !s_companionWolfBlend);
    enabled = true;
    resolve_dual_screen_host();
    assert(queries > 0 && s_dualScreenHost.owns_item_slots());
}
'''
with tempfile.TemporaryDirectory() as d:
    cpp = Path(d) / 'test.cpp'
    cpp.write_text(fixture)
    exe = Path(d) / 'test'
    subprocess.run(['c++', '-std=c++20', '-I', str(root / 'src'), str(cpp), '-o', str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
print('PASS: dual-screen opt-out skips host resolution and clears companion ownership')
