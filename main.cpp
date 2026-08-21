#include "gameboy.h"
#include <iostream>
#include <cassert>


int main() {
    //test_decode_tile_basic();
    std::string cpu_test_name = "roms/tests/cpu_instrs/individual/01-special.gb";
    std::string ppu_test_name = "roms/tests/dmg-acid2.gb";
    std::string tetris_name = "roms/Tetris(Japan) (En).gb";

	Gameboy gb;
    gb.load_rom(ppu_test_name);
    bool log = false;
    gb.run(log);

	return 0;
}


