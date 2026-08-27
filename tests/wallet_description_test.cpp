#include "wallet_description.hpp"

#include <cassert>
#include <string>

using namespace twilight_hd_hud;

int main() {
    const auto message = [](const std::string& text) { return text + '\0'; };
    const unsigned normal[] = {300, 600, 1000};
    const unsigned bigger[] = {500, 1000, 2000};
    for (int tier = 0; tier < 3; ++tier) {
        const auto stock = "It can hold " + std::to_string(normal[tier]) + " Rupees.";
        const auto hd = "It can hold " + std::to_string(bigger[tier]) + " Rupees.";
        assert(wallet_description_capacity(message(stock), bigger[tier]) == hd);
        assert(wallet_description_capacity(message(stock), normal[tier]).empty());
        // Supports already-patched text, toggling off, and repeated reads.
        assert(wallet_description_capacity(message(hd), normal[tier]) == stock);
        assert(wallet_description_capacity(message(hd), bigger[tier]).empty());
    }
    const std::string color("\x1a\x06\0\0\0\x02", 6);
    const std::string reset("\x1a\x06\0\0\0\0", 6);
    const auto tagged = "The wallet given to you by Agitha,\nprincess of the insect kingdom.\nIt can hold " +
        color + "600" + reset + " Rupees.";
    const auto corrected = "The wallet given to you by Agitha,\nprincess of the insect kingdom.\nIt can hold " +
        color + "1000" + reset + " Rupees.";
    assert(wallet_description_capacity(message(tagged), 1000) == corrected);
    const std::string numberInTag("\x1a\x08\0\0" "600!", 8);
    assert(wallet_description_capacity(message(numberInTag + "600 Rupees"), 1000) ==
        numberInTag + "1000 Rupees");
    for (char separator : {',', '.', ' ', static_cast<char>(0xa0)}) {
        const auto grouped = std::string("1") + separator + "000 Rubine.";
        assert(wallet_description_capacity(message(grouped), 2000) ==
            std::string("2") + separator + "000 Rubine.");
        assert(wallet_description_capacity(message(grouped), 600) == "600 Rubine.");
    }
    assert(wallet_description_capacity(message("Peut contenir 600 rubis."), 1000) ==
        "Peut contenir 1000 rubis.");
    assert(wallet_description_capacity(message("24 insects; 600 Rupees."), 1000) ==
        "24 insects; 1000 Rupees.");
    for (const auto text : {"No capacity.", "300 or 600 Rupees.", "16000 Rupees.",
            "9999999999999999999999999 Rupees."})
        assert(wallet_description_capacity(message(text), 1000).empty());
    assert(wallet_description_capacity(message("600 Rupees"), 0).empty());
    assert(wallet_description_capacity(message("600 Rupees"), 65536).empty());
    assert(wallet_description_capacity(message("600 Rupees"), 9999) == "9999 Rupees");
    // A missing terminator or malformed tag must never overrun the DAT1 block.
    const auto complete = message(tagged);
    for (std::size_t length = 0; length < complete.size(); ++length)
        assert(wallet_description_capacity(std::string_view(complete).substr(0, length), 1000).empty());
    assert(wallet_description_capacity(message("600\x1a\x04"), 1000).empty());
    assert(wallet_description_capacity(message("600\x1a\xff"), 1000).empty());
    assert(wallet_description_capacity(message(std::string(1024, 'x') + "600"), 1000).empty());
    assert(wallet_description_capacity(message("600 Rupees") + message("1000 Rupees"), 500) ==
        "500 Rupees");
}
