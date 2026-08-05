#pragma once
#include <vector>
#include <array>
#include <iostream>
#include <string>
#include <fstream>

#include "types.h"
#include "ppu.h"
#include "interrupts.h"
#include "timer.h"


struct MemoryRegion{	
	std::vector<uint8_t> memory;

	word start_address;
	word  end_address;

	MemoryRegion(word start, word end)
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

	MemoryRegion* get_correct_memory(word address);

	word fix_echo_address(word address);



public:

	Interrupts& interrupts;
	Timer& timer;
	Bus(Interrupts& interrupts, Timer& timer) : 
		interrupts(interrupts),
		ppu(interrupts),
		timer(timer)
	{}

	PPU ppu;


	byte read_memory(word address);
	void write_memory(word address, byte data);
	void dump_memory(word start_loc, byte num_bytes);
	void load_rom(const std::string filename);

};
