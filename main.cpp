#include "gameboy.h"
#include <iostream>
#include <cassert>
#include <array>


int main() {
    std::array<std::string, 11> cpu_test_names = {"01-special.gb", "02-interrupts.gb", "03-op sp,hl.gb",
    "04-op r,imm.gb", "05-op rp.gb", "06-ld r,r.gb", "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb", 
    "09-op r,r.gb", "10-bit ops.gb", "11-op a,(hl).gb"};

    std::string test_base = "roms/tests/";
    std::string cpu_test_base = test_base + "cpu_instrs/individual/";
    std::string ppu_test_name = test_base + "dmg-acid2.gb";
    std::string halt_bug_test = test_base + "halt_bug.gb";


    std::string tetris_name = "roms/Tetris.gb";
    std::string alleyway_name = "roms/Alleyway.gb";
    std::string sm_world_name = "roms/Super Mario Land.gb";
    std::string zelda_la_name = "roms/Legend of Zelda, The - Link's Awakening.gb";
    std::string f1_name = "roms/F1Race.gb";
    std::string pokemon_Blue_name = "roms/PokemonBlue.gb";
    std::string pokemon_Gold_name = "roms/PokemonGold.gbc";
    std::string wario_land_name = "roms/WarioLand2.gb";



    std::unique_ptr<Gameboy> gb = std::make_unique<Gameboy>();
    gb->load_rom(f1_name);
    gb->run();

	return 0;
}


