#include "gameboy.h"
#include "rom_picker.h"


void Gameboy::load_rom(const std::string& path) {

    bus.load_boot_rom("roms/boot_rom.bin");
    cpu.regs.PC = 0x0000;         // start at the real reset vector, not 0x0100
    cpu.regs.SP = 0x0000;         // boot rom sets this itself
    cpu.regs.regs_8b[A] = cpu.regs.regs_8b[F] = cpu.regs.regs_8b[B] =
        cpu.regs.regs_8b[C] = cpu.regs.regs_8b[D] = cpu.regs.regs_8b[E] = cpu.regs.regs_8b[H]
        = cpu.regs.regs_8b[L] = 0x0000;

    bus.ppu.reset();
    timer.reset();
    interrupts.reset();
    apu.reset();
    cpu.reset();

	bus.load_rom(path);
}

void Gameboy::run() {

    long long count = 0;
    bool logging_enabled = false;


    bool running = true;
    SDL_Event event;

    const double target_fps = 59.7275;
    const auto frame_duration = std::chrono::duration<double>(1.0 / target_fps);
    auto next_frame_time = std::chrono::steady_clock::now();

    auto last_battery_save = std::chrono::steady_clock::now();
    const std::chrono::seconds battery_save_interval(25);



	while (running) {

        poll_events(running, event, logging_enabled);

        {
            std::lock_guard<std::mutex> lock(rom_load_mutex);
            if (pending_rom_path.has_value()) {
                load_rom(pending_rom_path.value());
                pending_rom_path.reset();
                rom_loaded = true;

                // TODO: reset CPU/PPU/timer/interrupts to power-on state here,
                // same as whatever your constructor/initial load already does,
                // so swapping ROMs mid-session behaves like a fresh launch
            }
        }

        if (!rom_loaded) {
            continue;
        }


        if (logging_enabled) log_state();

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
        apu.tick(t_cycles);

        if (bus.ppu.frame_ready) {
            bus.ppu.frame_ready = false;

            if (!apu.sample_buffer.empty()) {

                // if backlog exceeds 3 frames drop it and resync
                Uint32 queued = SDL_GetAudioStreamQueued(apu.audio_stream);
                const Uint32 max_queued_bytes = 44100 * 2 * sizeof(float) * 3 / 60;
                if (queued > max_queued_bytes) {
                    SDL_ClearAudioStream(apu.audio_stream);
                }

                SDL_PutAudioStreamData(apu.audio_stream, apu.sample_buffer.data(),
                    apu.sample_buffer.size() * sizeof(float));
                apu.sample_buffer.clear();
            }

            next_frame_time += std::chrono::duration_cast<std::chrono::steady_clock::duration>(frame_duration);
            std::this_thread::sleep_until(next_frame_time);

            auto now = std::chrono::steady_clock::now();
            if (now - last_battery_save >= battery_save_interval) {
                bus.mbc->save_battery_save();
                last_battery_save = now;
            }

        }

		count++;

	}

    if (logging_enabled) {
        log_file.close();
    }
}

void Gameboy::poll_events(bool& running, SDL_Event& event,  bool& logging_enabled) {

    // handle events
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
            bus.mbc->save_battery_save();
        }
            

      
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

            case SDLK_F5: f5_held = pressed; break;
            case SDLK_F9: f9_held = pressed; break;
            
            case SDLK_F2: {
                if (pressed && !event.key.repeat) {
                    show_rom_picker(this, bus.ppu.window);
                }
                break;
            }

            case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
            case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9: case SDLK_0:
                if (pressed && !event.key.repeat) {
                    int slot = event.key.key - SDLK_0;

                    if (f5_held) {
                        save_state(get_save_state_path(slot));
                    }
                    else if (f9_held) {
                        load_state(get_save_state_path(slot));
                    }
                    else {
                        std::cout << "Hold F5 (save) or F9 (load) and press a number key\n";
                    }
                }
                break;


            case SDLK_F1: logging_enabled = !logging_enabled;
                std::cout << "Logging " << (logging_enabled ? "ON" : "OFF") << "\n"; break;

            }
        }

    }


}

sbyte Gameboy::get_save_slot(SDL_Event& event) {
    switch (event.key.key) {
        case SDLK_1: case SDLK_2: case SDLK_3: case SDLK_4: case SDLK_5:
        case SDLK_6: case SDLK_7: case SDLK_8: case SDLK_9: case SDLK_0:{
            return event.key.key - SDLK_0;
        }

    }
    return -1;
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

std::string Gameboy::get_save_state_path(int slot) {

    namespace fs = std::filesystem;

    fs::path saves_dir = "saves";
    if (!fs::exists(saves_dir)) {
        fs::create_directory(saves_dir);
    }

    fs::path rom_path(current_rom_filename);
    std::string base_name = rom_path.stem().string();

    fs::path full_path = saves_dir / (base_name + "_slot" + std::to_string(slot) + ".sav");
    return full_path.string();

}

void Gameboy::save_state(const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        std::cout << "Failed to open save state file (writing)\n";
        return;
    }

    cpu.serialize(out);
    bus.serialize(out);
    bus.ppu.serialize(out);
    timer.serialize(out);
    interrupts.serialize(out);
}

void Gameboy::load_state(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cout << "Failed to open save state file (loading)\n";
        return;
    }

    cpu.deserialize(in);
    bus.deserialize(in);
    bus.ppu.deserialize(in);
    timer.deserialize(in);
    interrupts.deserialize(in);
}