#pragma once

namespace twilight_hd_hud {
struct DualScreenHost {
    using HudQuery = bool (*)();
    using SlotQuery = unsigned (*)();
    HudQuery hudOnCompanion = nullptr;
    SlotQuery slotTriggerBits = nullptr;
    SlotQuery slotHoldBits = nullptr;

    bool active() const {
        return hudOnCompanion != nullptr && hudOnCompanion();
    }
    bool owns_item_slots() const {
        return slotTriggerBits != nullptr && slotHoldBits != nullptr;
    }
};
} // namespace twilight_hd_hud
