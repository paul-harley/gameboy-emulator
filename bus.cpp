#include "bus.h"
#include <iostream>


word Bus::fix_echo_address(word address) {

	if (address >= 0xE000 && address <= 0xFDFF) {
		return address - 0x2000; // mirrors into 0xC000-0xDDFF (wram)
	}
	return address;
}

MemoryRegion* Bus::get_correct_memory(word address) {

	for (MemoryRegion* region : main_memory) {

		if (address >= region->start_address && address <= region->end_address) {
			return region;
		}

	}

	std::cout << std::hex<< "Address: " << int(address) << " does not exist \n";
	throw std::runtime_error("Invalid memory address");
}


byte Bus::read_memory(word address) {

	if (address >= 0xFEA0 && address <= 0xFEFF) {
		return 0;
	}


	switch (address) {
	case 0xFF00:
	{
		byte value = joypad.read();
		return value;
	}

	case 0xFF01:
		return serial_data;

	case 0xFF02:
		return serial_control;

	case 0xFF04:
		return timer.DIV;
	case 0xFF05:
		return timer.TIMA;
	case 0xFF06:
		return timer.TMA;
	case 0xFF07:
		return timer.TAC;

	case 0xFF40:
		return ppu.LCDC;
	case 0xFF41: 
		return ppu.STAT;
	case 0xFF42:
		return ppu.SCY;
	case 0xFF43:
		return ppu.SCX;
	case 0xFF44:
		return ppu.get_ly();
	case 0xFF45: 
		return ppu.LYC;

	case 0xFF47: 
		return ppu.BGP;
	case 0xFF48: 
		return ppu.OBP0;
	case 0xFF49: 
		return ppu.OBP1;

	case 0xFF4A:
		return ppu.WY;
	case 0xFF4B:
		return ppu.WX;

	case 0xFF0F:
		return interrupts.IF | 0xE0;
	case 0xFFFF:
		return interrupts.IE;

	}

	address = fix_echo_address(address);


	const MemoryRegion* mem_region = get_correct_memory(address);

	word local_address = address - mem_region->start_address;
	return mem_region->memory[local_address];
	return 0;
}


void Bus::write_memory(word address, byte data) {
	
	if (address >= 0xFEA0 && address <= 0xFEFF) {
		return;
	}


	if (address <= 0x7FFF) {
		//std::cout << "WARNING: write to ROM area! addr="
		//	<< std::hex << address << " data=" << (int)data << std::endl;
		return;
	}


	switch (address) {
	case 0xFF00:
		joypad.write(data);
		return;

	case 0xFF01:
		serial_data = data;
		break;

	case 0xFF02:
		serial_control = data;
		if (data & 0x80) {
			serial_data = 0xFF;
			serial_control &= ~0x80;
			interrupts.request(Serial_i);
		}
		break;

	case 0xFF04:
		timer.DIV = 0;
		timer.reset_sys_counter();
		return;
	case 0xFF05:
		timer.TIMA = data;
		return;
	case 0xFF06:
		timer.TMA = data;
		return;
	case 0xFF07:
		timer.TAC = data;
		return;

	case 0xFF40:
		ppu.LCDC = data;
		return;
	case 0xFF41: 
		ppu.STAT = (ppu.STAT & 0x07) | (data & 0xF8); //ensure bottom 3 bits are read only
		return;
	case 0xFF42:
		ppu.SCY = data;
		return;
	case 0xFF43:
		ppu.SCX = data;
		return;
	case 0xFF45: 
		ppu.LYC = data; 
		return;

	case 0xFF46: {
		word source_base = data << 8;
		for (byte i = 0; i < 0xA0; i++) {
			byte value = read_memory(source_base + i);
			write_memory(0xFE00 + i, value);
		}
		return;
	}

	case 0xFF47:
		ppu.set_palette(address, data);
		return;
	case 0xFF48:
		ppu.set_palette(address, data);
		return;
	case 0xFF49:
		ppu.set_palette(address, data);
		return;

	case 0xFF4A:
		ppu.WY = data;
		return;
	case 0xFF4B:
		ppu.WX = data;
		return;

	case 0xFF0F:
		interrupts.IF = data & 0x1F;
		return;
	case 0xFFFF:
		interrupts.IE = data;
		return;
	}


	address = fix_echo_address(address);


	MemoryRegion* mem_region = get_correct_memory(address);
	word local_address = address - mem_region->start_address;
	mem_region->memory[local_address] = data;

}


void Bus::dump_memory(word start_loc, byte num_bytes) {

	MemoryRegion* mem_region = get_correct_memory(start_loc);


	word start_local_address = start_loc - mem_region->start_address;
	word end = std::min(mem_region->end_address, static_cast<word>(start_local_address + num_bytes));


	for (word i = start_local_address; i < end; i++) {
		std::cout
			<< "0x" << std::hex << i << " : "
			<< std::dec << static_cast<int>(mem_region->memory[i]) << "\n";

	}
}




void Bus::load_rom(const std::string filename) {

	std::ifstream rom(filename, std::ios::binary);

	if (!rom) {
		std::cout << "ROM not found..." << std::endl;
		return;
	}

	byte current_byte;
	word address = 0;

	rom.seekg(0, std::ios::end);
	std::cout << "ROM size: " << rom.tellg() << " bytes\n";
	rom.seekg(0, std::ios::beg);

	while (rom.read(reinterpret_cast<char*> (&current_byte), 1)) {
		
		if (address >= 0x8000)
		{
			std::cout << "ROM too large\n";
			break;
		}


		//write_memory(address, current_byte);
		MemoryRegion* mem_region = get_correct_memory(address);
		word local_address = address - mem_region->start_address;
		mem_region->memory[local_address] = current_byte;

		address++;
	}

}

