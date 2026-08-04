#pragma once

#include "cpu.h"
#include "bus.h"
#include "interrupts.h"
#include <fstream>
#include <iomanip>



class Gameboy {


public:
	Interrupts interrupts;
	Bus bus;      
	CPU cpu;   

	Gameboy()
		: bus(interrupts)
		, cpu(bus, interrupts)
		, log_file("log.txt")
	{}


	void load_rom(const std::string& path);
	void run(bool log_state);

private:
	std::ofstream log_file;
	void log_state();
};