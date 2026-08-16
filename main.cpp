#include "gameboy.h"
#include <iostream>
#include <cassert>


int main() {
    //test_decode_tile_basic();
	Gameboy gb;
    gb.load_rom("roms/tests/dmg-acid2.gb");
    bool log = false;
    gb.run(log);

	return 0;
}


