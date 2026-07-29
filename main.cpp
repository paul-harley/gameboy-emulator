#include "bus.h"
#include "registers.h"
#include "cpu.h"
#include <iostream>

void print_bin(byte val) {

	for (byte i = 0; i < 8; i++) {
		byte current_bit = val & 0x1;

		std::cout << std::hex << static_cast<int>(current_bit) << " ";

		val = val >> 1;
	}

	std::cout << std::endl;
}

void print_bin(word val) {

	for (word i = 0; i < 16; i++) {
		word current_bit = val & 0x1;

		std::cout << std::hex << static_cast<int>(current_bit) << " ";

		val = val >> 1;
	}

	std::cout << std::endl;
}


int main() {

	CPU my_cpu;


	my_cpu.bus.load_rom("roms/tests/cpu_instrs/cpu_instrs.gb");
	my_cpu.regs.PC = 0x100;

	static uint64_t count = 0;

	bool extra_debug = false;


	while (true) {

		count++;
		if (count > 5000000 && !extra_debug) {
			std::cout << count << '\n';
			extra_debug = true;
		}


		byte instruction = my_cpu.fetch();
		my_cpu.decode(instruction);

		if (extra_debug) {
			std::cout << std::hex << "BC=" << my_cpu.regs.get_BC() << std::endl;
			//std::cout << std::hex << int(instruction) << std::endl;
			//std::cout << "-----memory dump-----\n";
			//my_cpu.bus.dump_memory(0x73e, 12);
			//std::cout << "---------------------\n";
			//my_cpu.regs.dump_regs();
		}
	}


	return 0;
}


