#pragma once

#include "cpu.h"
#include "bus.h"
#include "interrupts.h"
#include "timer.h"
#include <fstream>
#include <iomanip>



class Gameboy {


public:
	Interrupts interrupts;
	Bus bus;      
	CPU cpu;   
	Timer timer;

	Gameboy()
		: bus(interrupts, timer)
		, cpu(bus, interrupts, timer)
		, timer(interrupts)
		, log_file("log.txt")
	{}


	void load_rom(const std::string& path);
	void run(bool log_state);

private:
	std::ofstream log_file;
	void log_state();
};