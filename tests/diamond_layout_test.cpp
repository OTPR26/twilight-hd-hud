#include "diamond_layout.hpp"
#include <cassert>
#include <iostream>

using namespace twilight_hd_hud;
struct Pane {
    Pane* parent = nullptr;
    float x = 0, y = 0, sx = 1, sy = 1;
    Pane* getParentPane() { return parent; }
    float getScaleX() { return sx; }
    float getScaleY() { return sy; }
    void add(float dx, float dy) { x += dx; y += dy; }
};
static void close(float a, float b) { assert(std::fabs(a - b) < 0.001f); }

int main() {
    for (int percent = 50; percent <= 125; ++percent) {
        const float scale = percent / 100.0f;
        Pane root{nullptr, 0, 0, 1.7f, 1.7f};
        Pane hud{&root, 0, 0, scale, scale};
        Pane action{&hud, 60, 10}, attack{&hud, 20, 50};
        Pane actionLabel{&action, 5, 7};
        Pane sword{&hud, 23, 35};
        const float dx = -40 * scale * root.sx, dy = 40 * scale * root.sy;
        // Model the per-draw restoration before applying the optional preset.
        for (int frame = 0; frame < 100; ++frame) {
            action.x = 60; action.y = 10; attack.x = 20; attack.y = 50;
            sword.x = 23; sword.y = 35;
            offset_diamond_group<Pane, 3>({&action, &actionLabel, nullptr}, dx, dy);
            offset_diamond_group<Pane, 3>({&attack, &sword, nullptr}, -dx, -dy);
            close(action.x, 20); close(action.y, 50);
            close(attack.x, 60); close(attack.y, 10);
            close(sword.x, 63); close(sword.y, -5);
            close(actionLabel.x, 5); close(actionLabel.y, 7);
        }
    }
    Pane zero{nullptr, 0, 0, 0, 0}, child{&zero, 5, 9};
    offset_diamond_group<Pane, 1>({&child}, 100, 100);
    close(child.x, 5); close(child.y, 9);
    for (float scale : {0.5f, 1.0f, 1.25f, 2.0f}) {
        Pane root{nullptr, 0, 0, scale, scale};
        Pane action{&root}, attack{&root}, item{&root};
        Pane sword{&attack, 2, -10}, ammoAnchor{&item, 1, -4};
        Pane north{&root, 20, -30}, shoulder{&root, 60, -40};
        for (int frame = 0; frame < 100; ++frame) {
            action.x = 60; action.y = 10;
            attack.x = 20; attack.y = 50;
            item.x = -20; item.y = 10;
            // Snapshot the authored centers first; then cycle E -> S -> W -> E.
            const float ax = action.x, ay = action.y, bx = attack.x, by = attack.y;
            const float ix = item.x, iy = item.y;
            offset_diamond_group<Pane, 1>({&action}, (bx-ax)*scale, (by-ay)*scale);
            offset_diamond_group<Pane, 2>({&attack, &sword}, (ix-bx)*scale, (iy-by)*scale);
            offset_diamond_group<Pane, 2>({&item, &ammoAnchor}, (ax-ix)*scale, (ay-iy)*scale);
            close(action.x, 20); close(action.y, 50);
            close(attack.x, -20); close(attack.y, 10);
            close(item.x, 60); close(item.y, 10);
            close(sword.x, 2); close(sword.y, -10);
            close(ammoAnchor.x, 1); close(ammoAnchor.y, -4);
            close(north.x, 20); close(north.y, -30);
            close(shoulder.x, 60); close(shoulder.y, -40);
        }
    }
    std::cout << "PASS: diamond subtree swap across 50-125%, nested artwork, repeated restoration, zero-scale guard\n";
}
