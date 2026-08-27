#pragma once

#include <cstdint>

namespace twilight_hd_hud {

// Raw controller reads bypass Dusklight's filtered pad state. Apply the same
// menu block and require held controls to be released before gameplay resumes.
class InputGate {
public:
    void update(bool blocked, std::uint32_t held) {
        if (blocked || blocked_) {
            suppressed_ |= held;
        }
        suppressed_ &= held;
        blocked_ = blocked;
        held_ = blocked ? 0 : held & ~suppressed_;
    }

    bool blocked() const { return blocked_; }
    bool held(std::uint32_t mask) const { return (held_ & mask) != 0; }
    bool suppressed(std::uint32_t mask) const { return (suppressed_ & mask) != 0; }

private:
    bool blocked_ = false;
    std::uint32_t suppressed_ = 0;
    std::uint32_t held_ = 0;
};

}  // namespace twilight_hd_hud
