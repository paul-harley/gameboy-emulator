#include "bus.h"
#include "registers.h"
#include <iostream>

int main() {

	Bus mybus;
	Registers regTest;

	byte F_TEST = 0xAF;
	byte instruction = 0b10000000;
	byte block_decider = (instruction & 0b11000000) >> 6;




	std::cout << std::hex<< static_cast<int>(block_decider) << std::endl;

	return 0;
}