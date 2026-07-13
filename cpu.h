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
	void ld_to_mem_A(word save_loc); //covers ldh also 
	void ld_to_ioC_A();
	void ld_to_A_mem(Reg16 val_loc);
	void ld_to_A_mem(word val_loc); //covers ldh also 
	void ld_to_A_C();
	void ld_to_HLI_loc_A(); // inc HL after ld, hl above stays the same
	void ld_to_HLD_loc_A(); // dec HL
	void ld_to_A_HLI_loc();
	void ld_to_A_HLD_loc();
	void ld_sp(word val);
	void ld_to_mem_SP(word save_loc);
	void ld_to_HL_SP(sbyte offset);
	void ld_to_SP_HL();

};