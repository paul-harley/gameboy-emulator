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
	void ld(Reg8 save_loc, Reg8 reg_to_copy);
	void ld(Reg8 save_loc, byte val);
	void ld(Reg16 save_loc, word val);
	void ld_to_HL_loc(Reg8 val_loc);
	void ld_to_HL_loc(byte val);
	void ld_to_reg_HL(Reg8 save_loc);
	void ld_to_mem_A(Reg16 save_loc);




};