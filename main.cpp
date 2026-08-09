#include "gameboy.h"
#include "graphics.h"
#include <iostream>
#include <cassert>


void test_decode_tile_basic() {
    std::array<byte, 16> tile_data = {
            0b10110010, 0b01100101, // row 0 
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // rows 1-7, irrelevant here
    };
    Graphics g;

    auto pixels = g.decode_tile(tile_data);

    assert(pixels[0][0] == 1); // pixel 0 of row 0
    assert(pixels[0][1] == 2); // pixel 1 of row 0

    std::cout << "test_decode_tile_basic passed\n";
}

int main() {
    test_decode_tile_basic();
	Gameboy gb;


    gb.load_rom("");
    bool ls = false;
    //gb.run(ls);
	return 0;
}


