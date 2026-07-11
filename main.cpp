#include "bus.h"
#include "registers.h"
#include <iostream>

int main() {

	Bus mybus;
	Registers regTest;


	word AF = 0xAD7B;
	regTest.set_AF(AF);

	std::cout << std::hex<< static_cast<int>(regTest.regs_8b[A]) << std::endl;
	std::cout << std::hex << static_cast<int>(regTest.regs_8b[F]) << std::endl;
	std::cout << std::hex << regTest.get_AF() << std::endl;

	byte flags = AF & 0xFF;
	std::cout << std::hex << static_cast<int>(flags) << std::endl;

	
	if (regTest.z_flag_set()) {
		std::cout << "SET" << std::endl;
	}
	else {
		std::cout << "NOT SET" << std::endl;
	}



	return 0;
}