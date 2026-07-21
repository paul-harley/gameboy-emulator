#pragma once

#include "bus.h"
#include "registers.h"

//TODO: CLEAN THIS FILE BY MAKING MORE GENERAL FUNCTIONS
// DONT NEED SEPERATE ONES FOR REG, RAW VAL, HL ETC..
// ONE FOR EACH, ANY THAT NEED TO SAVE CAN STAY SEPERATE

enum Cond {
	nz,
	z,
	nc,
	c
};


class CPU {

public:
	Bus bus;
	Registers regs;

	byte fetch();
	void decode(byte instruction);


private:

	void decode_block_0(byte instruction);

	//helpers
	byte get_Reg8(Reg8 reg);	//these are here because they need to see main memory for [hl]
	void set_Reg8(Reg8 reg, byte value);

	bool carry_add_8(byte val1, byte val2);
	bool carry_add_8(byte val1, byte val2, byte val3);
	bool carry_add_16(word val1, word val2);
	bool half_carry_add_8(byte val1, byte val2);
	bool half_carry_add_8(byte val1, byte val2, byte val3);
	bool half_carry_add_16(word val1, word val2);

	bool carry_sub_8(byte a, byte val1);
	bool carry_sub_8(byte a, byte val1, byte val2);
	bool half_carry_sub_8(byte a, byte val1);
	bool half_carry_sub_8(byte a, byte val1, byte val2);

	word fetch_16();


	Reg8 decode_reg8_bits(byte reg_3_bit_code);
	Reg16 decode_reg16_bits(byte reg_2_bit_code);
	Reg16 decode_reg16_stk_bits(byte reg_2_bit_code);
	Reg16 decode_reg16_mem_bits(byte reg_2_bit_code);
	Cond decode_cond_bits(byte cond_2_bit_code);


	//TODO: think about how to do the r16_mem decoding
	// it has hl+ and hl- i havent figured out yet
	// all loads with reg8 might need looked at again 
	// cause of [hl]

	//OPCODES
	

	// LOAD INSTRUCTIONS

	void ld(Reg8 save_loc, Reg8 reg_to_copy); //Exception: [hl], [hl] gives halt
	void ld(Reg8 save_loc, byte val);
	void ld(Reg16 save_loc, word val);
	void ld_to_HL_loc(Reg8 val_loc);
	void ld_to_HL_loc(byte val);
	void ld_to_reg_HL(Reg8 save_loc);
	void ld_to_mem_A(Reg16 save_address_loc);
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
	void sub_a_values_set_flags(byte val1, byte val2);
	void sub_a_values_set_flags(byte val1, byte val2, byte val3);

	void adc_a(Reg8 val_loc); // c = carry flag added 
	void adc_a_hl();
	void adc_a(byte val_to_add);
	void add_a(Reg8 val_loc);
	void add_a_hl();
	void add_a(byte val_to_add);
	void cp_a(Reg8 val_loc);
	void cp_a_hl();
	void cp_a(byte val);
	void dec(Reg8 val_loc);
	void inc(Reg8 val_loc);
	void sbc_a(Reg8 val_loc);
	void sbc_a_hl();
	void sbc_a(byte val_to_sub);
	void sub_a(Reg8 val_loc);
	void sub_a_hl();
	void sub_a(byte val_to_sub);


	void add_hl_sp();
	void add_SP(sbyte val_to_add);


	// 16 BIT ARITHMETIC
	void add_HL(Reg16 val_loc);
	void dec(Reg16 val_loc);
	void inc(Reg16 val_loc);


	// BITWISE LOGIC INSTRUCTIONS
	void and_a(Reg8 reg_id);
	void and_a_hl();
	void and_a(byte val);
	void cpl();
	void or_a(Reg8 reg_id);
	void or_a_hl();
	void or_a(byte val);
	void xor_a(Reg8 reg_id);
	void xor_a_hl();
	void xor_a(byte val);


	// BITFLAG INSTRUCTIONS
	void bit(byte test_bit, byte value);
	void res(byte test_bit, Reg8 val_loc);
 	void set(byte test_bit, Reg8 val_loc);





	// BITSHIFT INSTRUCTIONS
	void rlca();
	void rrca();
	void rla();
	void rra();


	// JUMPS AND SUBROUTINE INSTRUCTIONS
	void jr(sbyte offset);
	void jr_cond(Cond condition, sbyte offset);


	// CARRY FLAG INSTRUCTIONS
	void ccf();
	void scf();


	//INTERUPTS
	void halt();

	// MISC INSTRUCTIONS
	void daa();

};