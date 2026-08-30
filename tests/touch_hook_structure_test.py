"""Windows must never make optional touch-input observation a load dependency."""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / "src/item_slot_hooks.cpp").read_text()

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
