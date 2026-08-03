#include "bus.h"
#include <iostream>


Bus::Bus() {

}

word Bus::fix_echo_address(word address) {

	if (address >= 0xE000 && address <= 0xFDFF) {
		return address - 0x2000; // mirrors into 0xC000-0xDDFF (wram)
	}
	return address;
}

MemoryRegion* Bus::get_correct_memory(uint16_t address) {

	for (MemoryRegion* region : main_memory) {

		if (address >= region->start_address && address <= region->end_address) {
			return region;
		}

	}

	std::cout << std::hex<< "Address: " << int(address) << " does not exist \n";
	throw std::runtime_error("Invalid memory address");
}


uint8_t Bus::read_memory(uint16_t address) {

	address = fix_echo_address(address);

	switch (address)
	{
	case 0xFF44:
		return 0x90; // just return this every time for now 
		//return ppu.get_ly();
	};


	const MemoryRegion* mem_region = get_correct_memory(address);

	uint16_t local_address = address - mem_region->start_address;
	return mem_region->memory[local_address];
	return 0;
}



void Bus::write_memory(uint16_t address, uint8_t data) {

	if (address <= 0x7FFF) {
		std::cout << "WARNING: write to ROM area! addr="
			<< std::hex << address << " data=" << (int)data << std::endl;
	}

	if (address == 0x4244)
	{
		printf("WRITE 4244 = %02X\n", data);
	}



	address = fix_echo_address(address);

	if (address == 0xFF02) {
		if (data & 0x80) {
			std::cout << (char)read_memory(0xFF01);
			data &= 0x7F; // clear transfer flag immediately - instant "transfer"
		}
	}

	MemoryRegion* mem_region = get_correct_memory(address);
	uint16_t local_address = address - mem_region->start_address;
	mem_region->memory[local_address] = data;

}


void Bus::dump_memory(uint16_t start_loc, uint8_t num_bytes) {

	MemoryRegion* mem_region = get_correct_memory(start_loc);


	uint16_t start_local_address = start_loc - mem_region->start_address;
	uint16_t end = std::min(mem_region->end_address, static_cast<uint16_t>(start_local_address + num_bytes));


	for (uint16_t i = start_local_address; i < end; i++) {
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
		uint16_t local_address = address - mem_region->start_address;
		mem_region->memory[local_address] = current_byte;

		address++;
	}

}

