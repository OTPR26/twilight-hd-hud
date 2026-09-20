#include "localized_labels.hpp"
#include <cassert>
#include <iostream>

using namespace twilight_hd_hud;

int main() {
    assert(single_line_label(" Asignar\nObjeto ") == "Asignar Objeto");
    assert(single_line_label("A\r\n\t B") == "A B");
    assert(single_line_label("") == "");
    // Preserve native accented and multibyte glyph bytes, not just ASCII.
    assert(single_line_label("Colecci\xf3n") == "Colecci\xf3n");
    assert(single_line_label("\x83\x41\x83\x43") == "\x83\x41\x83\x43");
    assert(collection_shortcut_label("Colecci\xf3n", "Guardar") == "Colecci\xf3n/\nGuardar");
    assert(collection_shortcut_label("Sammlung", "Speichern") == "Sammlung/\nSpeichern");
    assert(static_cast<unsigned>(MenuLabel::Confirm) == 0x40c);
    assert(static_cast<unsigned>(MenuLabel::Back) == 0x3f9);
    assert(static_cast<unsigned>(MenuLabel::Collection) == 0x3e1);
    std::cout << "PASS: localized labels preserve encoding and normalize prompt line breaks\n";
}
