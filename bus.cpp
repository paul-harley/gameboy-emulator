#include "bus.h"
#include <iostream>


Bus::Bus() {

}

MemoryRegion* Bus::get_correct_memory(uint16_t address) {

	for (MemoryRegion* region : main_memory) {

		if (address >= region->start_address && address <= region->end_address) {
			return region;
		}

	}
}


uint8_t Bus::read_memory(uint16_t address) {

	const MemoryRegion* mem_region = get_correct_memory(address);

	uint16_t local_address = address - mem_region->start_address;
	return mem_region->memory[local_address];
	return 0;
}



void Bus::write_memory(uint16_t address, uint8_t data) {

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

