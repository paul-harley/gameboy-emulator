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

	CPU(Bus& bus, Interrupts& interrupts, Timer& timer)
		: bus(bus), 
		interrupts(interrupts),
		timer(timer)
	{}

	Bus& bus;
	Interrupts& interrupts;
	Timer& timer;
	Registers regs;

	byte fetch();
	byte decode(byte instruction);

	bool ime = false; //inretupts based on this
	bool ime_pending = false; // needed cause ei has a one intruction delay on setting the flag
	bool halted = false;
	bool halted_bug = false;


	byte interrupt_handler();

private:


	word fetch_16();

	//DECODING they all return number of cycles

	byte decode_block_0(byte instruction);
	byte decode_block_1(byte instruction);
	byte decode_block_2(byte instruction);
	byte decode_block_3(byte instruction);
	byte decode_prefixed(byte instruction);
	Reg8 decode_reg8_bits(byte reg_3_bit_code);
	Reg16 decode_reg16_bits(byte reg_2_bit_code);
	Reg16 decode_reg16_stk_bits(byte reg_2_bit_code);
	Reg16 decode_reg16_mem_bits(byte reg_2_bit_code);
	Cond decode_cond_bits(byte cond_2_bit_code);

	//helpers
	byte get_Reg8(Reg8 reg);	//these are here because they need to see main memory for [hl]
	void set_Reg8(Reg8 reg, byte value);	// reg16 versions are in registers.cpp

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


	bool check_conds(Cond cond);



	//OPCODES
	

	// LOAD INSTRUCTIONS

	byte ld(Reg8 save_loc, Reg8 reg_to_copy); //Exception: [hl], [hl] gives halt
	byte ld(Reg8 save_loc, byte val);
	byte ld(Reg16 save_loc, word val);
	void ld_to_HL_loc(Reg8 val_loc);
	void ld_to_HL_loc(byte val);
	void ld_to_reg_HL(Reg8 save_loc);
	byte ld_to_mem_A(Reg16 save_address_loc);
	byte ld_to_mem_A(word save_loc); //covers ldh also 
	byte ldh_c_a();
	byte ld_to_A_mem(Reg16 val_loc);
	void ld_to_A_mem(word val_loc); //covers ldh also 
	void ldh_a_c();
	void ld_to_HLI_loc_A(); // inc HL after ld, hl above stays the same
	void ld_to_HLD_loc_A(); // dec HL
	void ld_to_A_HLI_loc();
	void ld_to_A_HLD_loc();


	//8 BIT ARITHMETIC

	//general ones called from all add instructions
	void add_a_values_set_flags(byte val1, byte val2);
	void add_a_values_set_flags(byte val1, byte val2, byte val3);
	void sub_a_values_set_flags(byte val1, byte val2);
	void sub_a_values_set_flags(byte val1, byte val2, byte val3);

	byte adc_a(Reg8 val_loc); // c = carry flag added 
	byte adc_a(byte val_to_add);
	byte add_a(Reg8 val_loc);
	byte add_a(byte val_to_add);
	byte cp_a(Reg8 val_loc);
	byte cp_a(byte val);
	byte dec(Reg8 val_loc);
	byte inc(Reg8 val_loc);
	byte sbc_a(Reg8 val_loc);
	byte sbc_a(byte val_to_sub);
	byte sub_a(Reg8 val_loc);
	byte sub_a(byte val_to_sub);




	// 16 BIT ARITHMETIC
	byte add_HL(Reg16 val_loc);
	byte dec(Reg16 val_loc);
	byte inc(Reg16 val_loc);


	// BITWISE LOGIC INSTRUCTIONS
	byte and_a(Reg8 reg_id);
	byte and_a(byte val);
	byte cpl();
	byte or_a(Reg8 reg_id);
	byte or_a(byte val);
	byte xor_a(Reg8 reg_id);
	byte xor_a(byte val);


	// BITFLAG INSTRUCTIONS
	void bit(byte test_bit, byte value);
	void res(byte test_bit, Reg8 val_loc);
 	void set(byte test_bit, Reg8 val_loc);


	// BITSHIFT INSTRUCTIONS
	//have to keep a funcs cause they have different flags
	byte rla();	
	byte rl(Reg8 reg_to_rotate);
	byte rlc(Reg8 reg_to_rotate);
	byte rlca(); 
	byte rr(Reg8 reg_to_rotate);
	byte rra();
	byte rrc(Reg8 reg_to_rotate);
	byte rrca();

	byte sla(Reg8 reg_to_shift);
	byte sra(Reg8 reg_to_shift);
	byte srl(Reg8 reg_to_shift);
	byte swap(Reg8 reg_to_shift);



	// JUMPS AND SUBROUTINE INSTRUCTIONS
	byte call(word jump_loc);
	byte call(word jump_loc, Cond cond);
	byte jp_hl();
	byte jp(word jump_loc);
	byte jp(word jump_loc, Cond cond);
	byte jr(sbyte offset);
	byte jr(sbyte offset, Cond condition);
	byte ret();
	byte ret(Cond cond);
	byte ret_i();
	byte rst(word vector);



	// CARRY FLAG INSTRUCTIONS
	byte ccf();
	byte scf();


	//STACK MANIPULATION
	void add_hl_sp();
	byte add_SP(sbyte val_to_add);
	void dec_sp();
	void inc_sp();
	void ld_sp(word val_to_load);
	byte ld_to_mem_sp(word save_loc);
	byte ld_to_hl_spe8(sbyte val_to_add);
	byte ld_sp_hl();
	byte pop(Reg16 save_loc);
	byte push(Reg16 data_loc);
	byte push(word data);


	//INTERUPTS
	byte di();
	byte ei();
	byte halt();


	// MISC INSTRUCTIONS
	byte daa();

};