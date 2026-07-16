#pragma once

#include "bus.h"
#include "registers.h"

class CPU {

public:
	Bus bus;
	Registers regs;

	void fetch();
	void decode(byte instruction);



private:

	//OPCODES

	//helpers
	bool carry_add_8(byte val1, byte val2);
	bool carry_add_8(byte val1, byte val2, byte val3);
	bool carry_add_16(word val1, word val2);
	bool half_carry_add_8(byte val1, byte val2);
	bool half_carry_add_8(byte val1, byte val2, byte val3);
	bool half_carry_add_16(word val1, word val2);


	Reg8 decode_reg8_bits(byte reg_3_bit_code);
	Reg16 decode_reg16_bits(byte reg_2_bit_code);
	Reg16 decode_reg16_stk_bits(byte reg_2_bit_code);

	//TODO: think about how to do the r16_mem decoding
	// it has hl+ and hl- i havent figured out yet

	// LOAD INSTRUCTIONS

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

	//8 BIT ARITHMETIC

	//general ones called from all add instructions
	void add_a_values_set_flags(byte val1, byte val2);
	void add_a_values_set_flags(byte val1, byte val2, byte val3);

	void adc_a(Reg8 val_loc); // c = carry flag added 
	void adc_a_hl(Reg8 val_loc);
	void adc_a(byte val_to_add);
	void add_a(Reg8 val_loc);
	void add_a_HL();
	void add_HL(Reg16 val_loc);
	void add_HL_SP();
	void add_SP(sbyte val_to_add);

	void dec(Reg8 val_loc);
	void inc(Reg8 val_loc);

	// 16 BIT ARITHMETIC
	void add_HL(Reg16 val_loc);
	void dec(Reg16 val_loc);
	void inc(Reg16 val_loc);


	// BITWISE LOGIC INSTRUCTIONS
	void and_a(Reg8 reg_id);

};