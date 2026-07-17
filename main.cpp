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

	byte instruction = 0b00000001;
	std::cout << std::hex<< static_cast<int>(instruction) << std::endl;
	
	instruction = (instruction << 1);
	std::cout << std::hex<< static_cast<int>(instruction) << std::endl;


	byte old_a = 0b00000000;
	byte new_result = 0;

	print_bin(old_a);


	for (byte i = 0; i < 8; i++) {
		byte current_bit = old_a & 0x1;
		if (current_bit == 0) {
			new_result += 1 << i ;
		}
		old_a = old_a >> 1;
	}


	//std::cout << std::hex << static_cast<int>(old_a) << std::endl;
	print_bin(new_result);


	return 0;
}


