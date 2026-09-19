from source_helpers import read_hook_source
"""Windows must never make optional touch-input observation a load dependency."""
from pathlib import Path

source = read_hook_source()

declarations = """#if !defined(_WIN32)
DEFINE_HOOK(&PADSetVirtualStatus, PadSetVirtualStatusHook);
DEFINE_HOOK(&PADClearVirtualStatus, PadClearVirtualStatusHook);
#endif"""
callbacks = """#if !defined(_WIN32)
HookAction before_pad_set_virtual_status"""
registrations = """#if !defined(_WIN32)
    // Windows' runtime detour backend cannot patch these small host input
    // helpers reliably."""

assert declarations in source
assert callbacks in source
assert registrations in source

registration_scope = source.split(registrations, 1)[1].split("#endif", 1)[0]
assert "ADD_PRE(PadSetVirtualStatusHook" in registration_scope
assert "ADD_PRE(PadClearVirtualStatusHook" in registration_scope

print("PASS: Windows excludes optional touch virtual-input hooks from mod initialization")

assert '#if !defined(__APPLE__) || !TARGET_OS_IPHONE\nDEFINE_HOOK_SYMBOL("dusk::ui::midna_icon_source"' in source
assert '"dusk::ui::get_equip_target", &touchTargetAddress' in source
assert 's_getTouchEquipTarget(slot, target) && target.valid' in source
print("PASS: Apple mobile queries touch targets instead of patching the icon source")
