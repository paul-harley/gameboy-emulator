#include "gameboy.h"
#include "rom_picker.h"

#include <iostream>
#include <cassert>
#include <array>


int main() {

    std::unique_ptr<Gameboy> gb = std::make_unique<Gameboy>();
    show_rom_picker(gb.get(), gb->bus.ppu.window); 
    gb->run();


	return 0;
}


