#pragma once
#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <fstream>

#include "ppu.h"

typedef uint8_t byte;
typedef uint16_t word;


struct MemoryRegion{	
	std::vector<uint8_t> memory;

	uint16_t start_address;
	uint16_t end_address;

	MemoryRegion(uint16_t start, uint16_t end)
		: start_address(start),
		end_address(end),
		memory(end - start + 1)
	{
	}

};


class Bus {

private:
								// START	END
	
	//fixed for cartridge
	MemoryRegion rom_bank0		{ 0x0000, 0x3FFF };
	//not sure what this ones for yet
	MemoryRegion rom_bank1		{ 0x4000, 0x7FFF};
	MemoryRegion vram			{ 0x8000, 0x9FFF };
	MemoryRegion external_ram	{ 0xA000, 0xBFFF};
	MemoryRegion wram			{ 0xC000, 0xDFFF };
	// object attribute memory, not sure what this is yet
	MemoryRegion oam			{ 0xFE00, 0xFE9F };
	//NOT USEABLE AREA			  0xFEA0   0xFEFF
	MemoryRegion io_registers	{ 0xFF00, 0xFF7F };
	MemoryRegion hram			{ 0xFF80, 0xFFFE };
	MemoryRegion ie_register	{ 0xFFFF, 0xFFFF };

	std::array<MemoryRegion*, 9> main_memory = { &rom_bank0, &rom_bank1, &vram, &external_ram,
		&wram, &oam, &io_registers, &hram, &ie_register };

	MemoryRegion* get_correct_memory(uint16_t address);

	word fix_echo_address(word address);

	PPU ppu;


public:								
	Bus();



	uint8_t read_memory(uint16_t address);
	void write_memory(uint16_t address, uint8_t data);
	void dump_memory(uint16_t start_loc, uint8_t num_bytes);
	void load_rom(const std::string filename);

};
