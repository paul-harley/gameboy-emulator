#include "gameboy.h"
#include <iostream>
#include <cassert>
#include <array>


int main() {
    //test_decode_tile_basic();
    std::array<std::string, 11> cpu_test_names = {"01-special.gb", "02-interrupts.gb", "03-op sp,hl.gb",
    "04-op r,imm.gb", "05-op rp.gb", "06-ld r,r.gb", "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb", 
    "09-op r,r.gb", "10-bit ops.gb", "11-op a,(hl).gb"};

    std::string test_base = "roms/tests/";
    std::string cpu_test_base = test_base + "cpu_instrs/individual/";
    std::string ppu_test_name = test_base + "dmg-acid2.gb";
    std::string halt_bug_test = test_base + "halt_bug.gb";


    std::string tetris_name = "roms/Tetris.gb";
    std::string alleyway_name = "roms/Alleyway.gb";


	Gameboy gb;
    gb.load_rom(tetris_name);
    bool log = true;
    gb.run(log);

	return 0;
}


