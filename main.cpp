#include "bus.h"
#include "registers.h"
#include <iostream>

void print_bin(byte val) {

	for (byte i = 0; i < 8; i++) {
		byte current_bit = val & 0x1;

		std::cout << std::hex << static_cast<int>(current_bit) << " ";

		val = val >> 1;
	}

	std::cout << std::endl;
}

int main() {

	Bus mybus;
	Registers regTest;

	byte F_TEST = 0xAF;

	byte instruction = 0b00010001;
	std::cout << std::hex<< static_cast<int>(instruction) << std::endl;
	
	instruction = (instruction << 1);
	std::cout << std::hex<< static_cast<int>(instruction) << std::endl;


	bool new_c = 1;
	byte new_val = (new_c << 7);
	std::cout << "***********" << std::endl;
	std::cout << std::hex << static_cast<int>(new_val) << std::endl;



	return 0;
}


