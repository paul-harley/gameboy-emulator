#include "gameboy.h"

void Gameboy::load_rom(const std::string& path) {
	bus.load_rom(path);
	cpu.regs.PC = 0x100;
}

void Gameboy::run(bool ls) {
	long long count = 0;
	const long long max_instructions = 7500000;



	while (count < max_instructions) {

        if (ls) log_state();

        byte cycles;

        if (cpu.halted) {
            cycles = 1; 

            if (cpu.interrupts.pending()) {
                cpu.halted = false; // wake up regardless of IME
            }
        }
        else {

            byte instruction = cpu.fetch();

            if (cpu.halted_bug) {
                cpu.halted_bug = false;
                cpu.regs.PC--; // read the same byte next fetch 
            }

            cycles = cpu.decode(instruction);
        }

        if (cpu.ime_pending) {
            cpu.ime = true;
            cpu.ime_pending = false;
        }

        cycles += cpu.interrupt_handler();

        word t_cycles = cycles * 4;
        bus.ppu.tick(t_cycles);
        timer.tick(t_cycles);

		count++;
	}

    if (ls) {
        log_file.close();
    }
}


void Gameboy::log_state() {
    log_file << std::hex << std::setfill('0');

    log_file << "A:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[A]) << " ";
    log_file << "F:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[F]) << " ";
    log_file << "B:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[B]) << " ";
    log_file << "C:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[C]) << " ";
    log_file << "D:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[D]) << " ";
    log_file << "E:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[E]) << " ";
    log_file << "H:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[H]) << " ";
    log_file << "L:" << std::setw(2) << static_cast<int>(cpu.regs.regs_8b[L]) << " ";

    log_file << "SP:" << std::setw(4) << static_cast<int>(cpu.regs.SP) << " ";

    word pc = cpu.regs.PC;
    log_file << "PC:" << std::setw(4) << static_cast<int>(pc) << " ";

    log_file << "PCMEM:"
        << std::setw(2) << static_cast<int>(cpu.bus.read_memory(pc)) << ","
        << std::setw(2) << static_cast<int>(cpu.bus.read_memory(pc + 1)) << ","
        << std::setw(2) << static_cast<int>(cpu.bus.read_memory(pc + 2)) << ","
        << std::setw(2) << static_cast<int>(cpu.bus.read_memory(pc + 3))
        << std::endl;
}