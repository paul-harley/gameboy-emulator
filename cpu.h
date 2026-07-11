#pragma once

#include "bus.h"
#include "registers.h"

class CPU {

public:
	Bus bus;
	Registers regs;

	void fetch();
	void decode();



private:

	//OPCODES

	// load instructions
	



};