#include "gameboy.h"
#include "graphics.h"
#include <iostream>
#include <cassert>


int main() {
    //test_decode_tile_basic();
	Gameboy gb;
    gb.load_rom("");
    bool ls = false;
    //gb.run(ls);


    while (true) {
        gb.bus.ppu.draw_background();
    }

	return 0;
}


