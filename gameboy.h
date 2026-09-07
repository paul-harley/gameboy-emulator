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



class Gameboy {


public:
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
	void log_state();
	void save_state(const std::string& path);
	void load_state(const std::string& path);
};