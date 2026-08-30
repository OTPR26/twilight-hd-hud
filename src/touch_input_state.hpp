#pragma once

#include <cstdint>

namespace twilight_hd_hud {

class TouchInputState {
public:
    void update(std::uint32_t buttons, std::uint32_t itemsMask,
        std::uint32_t collectionsMask, std::uint32_t zMask,
        std::uint32_t lMask, std::uint32_t rMask) noexcept {
        const bool itemsHeld = (buttons & itemsMask) != 0;
        const bool collectionsHeld = (buttons & collectionsMask) != 0;
        const bool zHeld = (buttons & zMask) != 0;
        const bool lHeld = (buttons & lMask) != 0;
        const bool rHeld = (buttons & rMask) != 0;

        mItemsTriggered = itemsHeld && !mItemsHeld;
        mCollectionsTriggered = collectionsHeld && !mCollectionsHeld;
        mZTriggered = zHeld && !mZHeld;
        mLTriggered = lHeld && !mLHeld;
        mRTriggered = rHeld && !mRHeld;
        mItemsHeld = itemsHeld;
        mCollectionsHeld = collectionsHeld;
        mZHeld = zHeld;
        mLHeld = lHeld;
        mRHeld = rHeld;
    }

    void clear() noexcept {
        mItemsHeld = false;
        mItemsTriggered = false;
        mCollectionsHeld = false;
        mCollectionsTriggered = false;
        mZHeld = false;
        mZTriggered = false;
        mLHeld = false;
        mLTriggered = false;
        mRHeld = false;
        mRTriggered = false;
    }

    bool items_held() const noexcept { return mItemsHeld; }
    bool items_triggered() const noexcept { return mItemsTriggered; }
    bool collections_held() const noexcept { return mCollectionsHeld; }
    bool collections_triggered() const noexcept { return mCollectionsTriggered; }
    bool z_held() const noexcept { return mZHeld; }
    bool z_triggered() const noexcept { return mZTriggered; }
    bool l_held() const noexcept { return mLHeld; }
    bool l_triggered() const noexcept { return mLTriggered; }
    bool r_held() const noexcept { return mRHeld; }
    bool r_triggered() const noexcept { return mRTriggered; }

private:
    bool mItemsHeld = false;
    bool mItemsTriggered = false;
    bool mCollectionsHeld = false;
    bool mCollectionsTriggered = false;
    bool mZHeld = false;
    bool mZTriggered = false;
    bool mLHeld = false;
    bool mLTriggered = false;
    bool mRHeld = false;
    bool mRTriggered = false;
};

}  // namespace twilight_hd_hud
