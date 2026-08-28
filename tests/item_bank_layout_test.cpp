#include "item_bank_layout.hpp"
#include "collection_layout.hpp"
#include <cassert>
#include <iostream>
#include <queue>

using namespace twilight_hd_hud;

int main() {
    for (int i = 0; i < 3; ++i) {
        for (const auto size : {std::array<float, 2>{83, 83}, {64, 64}, {128, 64}, {32, 64}}) {
            const auto prompt = bank_assignment_prompt(i, size[0], size[1]);
            assert(std::fabs(prompt.width / prompt.height - size[0] / size[1]) < 0.0001f);
            assert(prompt.y + prompt.height / 2 == 139);
            assert(prompt.x + prompt.width / 2 == (i == 0 ? 364 : i == 1 ? 398 : 441));
            assert(std::fabs(std::fmax(prompt.width, prompt.height) - (i == 2 ? 32 : 24)) < 0.0001f);
        }
    }
    const auto r = bank_assignment_prompt(2, 83, 83);
    assert(std::fabs(r.width - 32) < 0.0001f && std::fabs(r.height - 32) < 0.0001f);
    assert(std::fabs(r.x - 425) < 0.0001f && std::fabs(r.y - 123) < 0.0001f);
    // Full-surface layout still adapts to genuine aspect-ratio changes.
    for (const auto bounds : {std::array<float, 4>{0, 0, 796, 448},
            {-94, 0, 702, 448}, {0, 0, 608, 448}, {-180, -20, 788, 468}}) {
        const auto viewport = collection_viewport(bounds[0], bounds[1], bounds[2], bounds[3]);
        assert(viewport.scale > 0);
        assert(std::fabs(viewport.left + 398 * viewport.scale - (bounds[0] + bounds[2]) / 2) < 0.001f);
        assert(std::fabs(viewport.top + 224 * viewport.scale - (bounds[1] + bounds[3]) / 2) < 0.001f);
    }
    for (int cell = 0; cell < 24; ++cell) {
        for (float scale : {0.5f, 1.0f, 1.25f, 2.0f}) {
            for (float phase : {0.0f, 5.0f, 10.0f, 15.0f, 20.0f}) {
                const float extent = bank_cursor_half_extent(cell, scale, phase);
                assert(extent >= (bank_cell_size(cell) * 0.5f - 3.01f) * scale);
                assert(extent <= (bank_cell_size(cell) * 0.5f - 1.99f) * scale);
                assert(extent * 2 < 66 * scale); // Never reaches the next cell.
            }
        }
    }
    assert(bank_combo_button_alpha(false, false) == 0);
    assert(bank_combo_button_alpha(false, true) == 0);
    assert(bank_combo_button_alpha(true, false) == 95);
    assert(bank_combo_button_alpha(true, true) == 255);
    assert(kBankComboX == bank_x(0) - bank_cell_size(0) * 0.5f);
    assert(kBankComboY > bank_y(14) + bank_cell_size(14) * 0.5f);
    for (int cell = 21; cell < 24; ++cell)
        assert(bank_x(cell) - bank_cell_size(cell) * 0.5f > kBankComboX + 43 + 240);
    for (unsigned slot : {20u, 1u, 0u, 9u, 10u, 6u, 8u})
        assert(bank_icon_extent(slot, bank_cell(slot), false) > 54.0f);
    for (unsigned slot : {11u, 12u, 13u, 14u, 15u, 16u, 17u, 2u, 5u})
        assert(bank_icon_extent(slot, bank_cell(slot), true) < 54.0f);
    assert(bank_icon_extent(19, 21, true) < bank_cell_size(21));
    std::array<bool, 24> seen{};
    for (auto slot : kBankSlots) {
        assert(slot < 24 && !seen[slot]);
        seen[slot] = true;
    }
    assert(bank_cell(255) == -1);
    assert(bank_cell(20) == 0); // Fishing rod.
    assert(bank_cell(18) == kBankOoccooCell);
    assert(kBankOoccooCell == kBankColumns * kBankRows - 1);
    assert(bank_x(kBankOoccooCell) == bank_x(6)); // Rightmost column.
    assert(bank_y(kBankOoccooCell) == bank_y(14)); // Bottom main row.
    assert(bank_icon_extent(18, kBankOoccooCell, true) < bank_cell_size(kBankOoccooCell));
    assert(bank_cell(17) == 4 && bank_cell(16) == 11 && bank_cell(15) == 18);
    assert(bank_cell(11) == 5 && bank_cell(12) == 6);
    assert(bank_cell(13) == 12 && bank_cell(14) == 13);
    assert(bank_cell(9) == bank_cell(10)); // Upgrade stays in place.
    assert(bank_cell(10, true) == 23); // Both variants remain accessible.

    unsigned char entries[24]{};
    for (unsigned i = 0; i < 24; ++i) entries[i] = kBankSlots[23 - i];
    for (int cell = 0; cell < 24; ++cell)
        assert(bank_entry(cell, entries, 24) == 23 - cell);
    assert(bank_entry(-1, entries, 24) == -1);
    assert(bank_entry(24, entries, 24) == -1);
    assert(bank_entry(0, entries, 25) == -1);
    unsigned char upgraded[] = {20, 4, 10, 13, 19};
    assert(bank_entry(9, upgraded, 5) == 2);
    assert(bank_entry(23, upgraded, 5) == -1);
    assert(bank_entry(21, upgraded, 5) == 4);

    constexpr BankDirection dirs[] = {BankDirection::Left, BankDirection::Right,
        BankDirection::Up, BankDirection::Down};
    // All items must remain reachable, including sparse early-game saves and
    // overflow quest cells. Exhaust every possible two-item configuration.
    const auto reachable = [&](const std::array<bool, 24>& occupied, int start) {
        std::array<bool, 24> visited{};
        std::queue<int> pending;
        pending.push(start);
        visited[start] = true;
        while (!pending.empty()) {
            const int current = pending.front();
            pending.pop();
            for (auto dir : dirs) {
                const int next = bank_neighbor(current, dir, occupied);
                assert(next >= 0 && next < 24 && occupied[next]);
                if (!visited[next]) { visited[next] = true; pending.push(next); }
            }
        }
        for (int i = 0; i < 24; ++i) {
            if (occupied[i] && !visited[i]) {
                std::cerr << "Unreachable cell " << i << " from " << start << "; inventory:";
                for (int j = 0; j < 24; ++j) if (occupied[j]) std::cerr << ' ' << j;
                std::cerr << '\n';
            }
            assert(!occupied[i] || visited[i]);
        }
    };
    std::array<bool, 24> occupied{};
    for (auto dir : dirs) assert(bank_neighbor(0, dir, occupied) == 0);
    for (int a = 0; a < 24; ++a) {
        for (int b = 0; b < 24; ++b) {
            occupied.fill(false);
            occupied[a] = occupied[b] = true;
            reachable(occupied, a);
        }
    }
    occupied.fill(true);
    assert(bank_neighbor(kBankOoccooCell, BankDirection::Left, occupied) == 19);
    assert(bank_neighbor(kBankOoccooCell, BankDirection::Up, occupied) == 13);
    assert(bank_neighbor(19, BankDirection::Right, occupied) == kBankOoccooCell);
    assert(bank_neighbor(13, BankDirection::Down, occupied) == kBankOoccooCell);
    for (int i = 0; i < 24; ++i) reachable(occupied, i);
    assert(bank_neighbor(0, BankDirection::Right, occupied) == 1);
    assert(bank_neighbor(0, BankDirection::Down, occupied) == 7);
    assert(bank_neighbor(20, BankDirection::Right, occupied) == 20);
    // Deterministic sparse inventories.
    unsigned random = 1;
    for (int sample = 0; sample < 1000; ++sample) {
        for (auto& cell : occupied) {
            random = random * 1664525u + 1013904223u;
            cell = (random >> 28) < 6;
        }
        for (int i = 0; i < 24; ++i) if (occupied[i]) { reachable(occupied, i); break; }
    }
    std::cout << "PASS: fixed slots, upgrades, overflow, sparse navigation and safe indices\n";
}
