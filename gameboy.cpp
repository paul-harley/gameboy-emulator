#include "gameboy.h"

void Gameboy::load_rom(const std::string& path) {

    bus.load_boot_rom("roms/boot_rom.bin");
    cpu.regs.PC = 0x0000;         // start at the real reset vector, not 0x0100
    cpu.regs.SP = 0x0000;         // boot rom sets this itself
    cpu.regs.regs_8b[A] = cpu.regs.regs_8b[F] = cpu.regs.regs_8b[B] =
        cpu.regs.regs_8b[C] = cpu.regs.regs_8b[D] = cpu.regs.regs_8b[E] = cpu.regs.regs_8b[H]
        = cpu.regs.regs_8b[L] = 0x0000;

	bus.load_rom(path);
	//cpu.regs.PC = 0x100;
}

void Gameboy::run(bool ls) {

    long long count = 0;

    bool running = true;
    SDL_Event event;

    static int frame_count = 0;



	while (running) {

        // handle events
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                        interrupts.request(Joypad_i);
                }

                bool pressed = (event.type == SDL_EVENT_KEY_DOWN);

                switch (event.key.key) {
                case SDLK_UP:    joypad.up = pressed; break;
                case SDLK_DOWN:  joypad.down = pressed; break;
                case SDLK_LEFT:  joypad.left = pressed; break;
                case SDLK_RIGHT: joypad.right = pressed; break;
                case SDLK_Z:     joypad.a = pressed; break;
                case SDLK_X:     joypad.b = pressed; break;
                case SDLK_RETURN: joypad.start = pressed; break;
                case SDLK_BACKSPACE: joypad.select = pressed; break;
                }
            }

        }

        if (ls) log_state();

        if (cpu.ime_pending) {
            cpu.ime = true;
            cpu.ime_pending = false;
        }

        byte cycles;

        if (cpu.halted) {
            cycles = 1; 

            if (cpu.interrupts.pending()) {
                cpu.halted = false; // wake up regardless of IME
            }
        }
        else {
            cpu.bus.last_pc = cpu.regs.PC;


            byte instruction = cpu.fetch();

            if (cpu.halted_bug) {
                cpu.halted_bug = false;
                cpu.regs.PC--; // read the same byte next fetch 
            }

            cycles = cpu.decode(instruction);
        }

        cycles += cpu.interrupt_handler();

        word t_cycles = cycles * 4;
        bus.ppu.tick(t_cycles);
        timer.tick(t_cycles);
        bus.serial_tick(t_cycles);

		count++;

	}

    if (ls) {
        log_file.close();
    }
}


void Gameboy::log_state() {
    if (bus.boot_rom_enabled) {
        return;
    }

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