#include "dual_screen.hpp"
#include <cassert>
using twilight_hd_hud::DualScreenHost;
namespace {
bool companion = false;
bool hud_on_companion() { return companion; }
unsigned slot_bits() { return 0; }
}
int main() {
    DualScreenHost host;
    assert(!host.active());
    assert(!host.owns_item_slots());
    host.hudOnCompanion = hud_on_companion;
    assert(!host.active());
    companion = true;
    assert(host.active());
    host.slotTriggerBits = slot_bits;
    assert(!host.owns_item_slots());
    host.slotHoldBits = slot_bits;
    assert(host.owns_item_slots());
    companion = false;
    assert(!host.active());
    assert(host.owns_item_slots());
    companion = true;
    assert(host.active());
}
