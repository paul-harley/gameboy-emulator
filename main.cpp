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


	std::cout << "*******************" << std::endl;
	word test_address = 0x10F0;
	mybus.dump_memory(test_address, 10);
	std::cout << "*******************" << std::endl;
	mybus.write_memory(test_address, 0xF9);
	std::cout<<static_cast<int>(mybus.read_memory(test_address)) << std::endl;
	std::cout << "*******************" << std::endl;
	mybus.dump_memory(test_address, 10);

	return 0;
}