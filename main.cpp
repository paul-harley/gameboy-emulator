#include "gameboy.h"
#include <iostream>




int main() {

	Gameboy gb;
    std::array<std::string, 11> rom_names = {"01-special.gb", "02-interrupts.gb", "03-op sp,hl.gb", "04-op r,imm.gb", 
        "05-op rp.gb", "06-ld r,r.gb", "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb", "09-op r,r.gb",
        "10-bit ops.gb", "11-op a,(hl).gb"};

    //ROM 07 says passed but gameboy doctor says diversion around rst, 
    //instruction gbd reads is e7, my one reads c7
    // keep an eye in future if any weird bugs, check this again


	//my_cpu.bus.load_rom("roms/tests/cpu_instrs/cpu_instrs.gb");
    gb.load_rom("roms/tests/cpu_instrs/individual/" + rom_names[10]);
    bool ls = false;
    gb.run(ls);
	return 0;
}


