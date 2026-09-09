#pragma once

#include "cpu.h"
#include "joypad.h"
#include "bus.h"
#include "interrupts.h"
#include "timer.h"
#include "apu.h"

#include <chrono> // these 2 are used to keep everything at correct timings
#include <thread>

#include <fstream>
#include <iomanip>

#include <filesystem>

#include <mutex>
#include <optional>



class Gameboy {


public:
	std::mutex rom_load_mutex;
	std::optional<std::string> pending_rom_path;

	Interrupts interrupts;
	Joypad joypad;
	Bus bus;      
	CPU cpu;   
	Timer timer;
	APU apu;

	Gameboy()
		: bus(joypad, interrupts, timer, apu)
		, cpu(bus, interrupts, timer)
		, timer(interrupts)
		, apu(timer)
		, log_file("log.txt")
	{}


	void load_rom(const std::string& path);
	void run();

private:
	std::ofstream log_file;
	std::string current_rom_filename;
	bool f5_held = false;
	bool f9_held = false;

	void log_state();
	void poll_events(bool& running, SDL_Event& event, bool& logging_enabled);
	void save_state(const std::string& path);
	void load_state(const std::string& path);
	std::string get_save_state_path(int slot);
	sbyte get_save_slot(SDL_Event& event);

	bool rom_loaded = false;
};