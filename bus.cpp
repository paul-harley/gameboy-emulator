#include "bus.h"
#include <iostream>


Bus::Bus() {

}


uint8_t Bus::read_memory(uint16_t address) {
	
	for (const auto* region : main_memory){

		if (address >= region->start_address && address <= region->end_address) {
			std::cout << "WE FOUND IT" << std::endl;
		}

	}

	return 0;
}