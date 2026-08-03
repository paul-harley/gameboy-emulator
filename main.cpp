#include "bus.h"
#include "registers.h"
#include "cpu.h"
#include <iostream>
#include <iomanip>
#include <fstream>

void print_bin(byte val) {

	for (byte i = 0; i < 8; i++) {
		byte current_bit = val & 0x1;

		std::cout << std::hex << static_cast<int>(current_bit) << " ";

		val = val >> 1;
	}

	std::cout << std::endl;
}



std::ofstream log_file("log.txt");

static void log_state(CPU& c) {
    log_file << std::hex << std::setfill('0');

    log_file << "A:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[A]) << " ";
    log_file << "F:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[F]) << " ";
    log_file << "B:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[B]) << " ";
    log_file << "C:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[C]) << " ";
    log_file << "D:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[D]) << " ";
    log_file << "E:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[E]) << " ";
    log_file << "H:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[H]) << " ";
    log_file << "L:" << std::setw(2) << static_cast<int>(c.regs.regs_8b[L]) << " ";

    log_file << "SP:" << std::setw(4) << static_cast<int>(c.regs.SP) << " ";

    word pc = c.regs.PC;
    log_file << "PC:" << std::setw(4) << static_cast<int>(pc) << " ";

    log_file << "PCMEM:"
        << std::setw(2) << static_cast<int>(c.bus.read_memory(pc)) << ","
        << std::setw(2) << static_cast<int>(c.bus.read_memory(pc + 1)) << ","
        << std::setw(2) << static_cast<int>(c.bus.read_memory(pc + 2)) << ","
        << std::setw(2) << static_cast<int>(c.bus.read_memory(pc + 3))
        << std::endl;
}

int main() {

	CPU my_cpu;
    std::array<std::string, 11> rom_names = {"01-special.gb", "02-interrupts.gb", "03-op sp,hl.gb", "04-op r,imm.gb", 
        "05-op rp.gb", "06-ld r,r.gb", "07-jr,jp,call,ret,rst.gb", "08-misc instrs.gb", "09-op r,r.gb",
        "10-bit ops.gb", "11-op a,(hl).gb"};

    //ROM 07 says passed but gameboy doctor says diversion around rst, 
    //instruction gbd reads is e7, my one reads c7
    // keep an eye in future if any weird bugs, check this again


	//my_cpu.bus.load_rom("roms/tests/cpu_instrs/cpu_instrs.gb");
    my_cpu.bus.load_rom("roms/tests/cpu_instrs/individual/" + rom_names[10]);


    my_cpu.bus.dump_memory(0x100, 16);

	my_cpu.regs.PC = 0x100;

    long long count = 0;
    const long long max_instructions = 7500000;



	while (count < max_instructions) {

        log_state(my_cpu);

		byte instruction = my_cpu.fetch();
		byte cycles = my_cpu.decode(instruction);
		my_cpu.bus.ppu.tick(cycles);

        count++;

	}

    log_file.close();
	return 0;
}


