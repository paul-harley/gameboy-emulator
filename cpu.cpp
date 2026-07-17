#include "cpu.h"


void CPU::decode(byte instruction) {

	// the blocks are based on https://gbdev.io/pandocs/CPU_Instruction_Set.html
	byte block_decider = (instruction & 0b11000000) >> 6;


	switch (block_decider) {
	case 0:

		break;

	case 1:

		break;
	
	case 2:

		break;

	case 3:

		break;
	}

}


//HELPERS

bool CPU::carry_add_8(byte val1, byte val2) {
	byte real_result = val1 + val2;
	word full_result = val1 + val2;

	if (real_result == full_result) {
		return false;
	}
	return true;
}

bool CPU::carry_add_8(byte val1, byte val2, byte val3) {
	byte real_result = val1 + val2 + val3;
	word full_result = val1 + val2 + val3;

	if (real_result == full_result) {
		return false;
	}
	return true;
}

bool CPU::carry_add_16(word val1, word val2) {
	word real_result = val1 + val2;
	uint32_t full_result = val1 + val2;

	if (real_result == full_result) {
		return false;
	}
	return true;
}

bool CPU::half_carry_add_8(byte val1, byte val2) {
	
	byte lower_nibble_1 = val1 & 0x0F;
	byte lower_nibble_2 = val2 & 0x0F;

	byte full_result = lower_nibble_1 + lower_nibble_2;

	// if first byte of full_result is not 0 
	// it has been used and flag would be set

	byte important_bit = full_result & 0b00010000;
	
	if (important_bit == 0) {
		return false;
	}

	return true;

}

bool CPU::half_carry_add_8(byte val1, byte val2, byte val3) {

	byte lower_nibble_1 = val1 & 0x0F;
	byte lower_nibble_2 = val2 & 0x0F;
	byte lower_nibble_3 = val3 & 0x0F;

	byte full_result = lower_nibble_1 + lower_nibble_2 + lower_nibble_3;

	// if first byte of full_result is not 0 
	// it has been used and flag would be set

	byte important_bit = full_result & 0b00010000;

	if (important_bit == 0) {
		return false;
	}

	return true;
}

bool CPU::half_carry_add_16(word val1, word val2) {

	byte lower_nibble_1 = val1 & 0x0F;
	byte lower_nibble_2 = val2 & 0x0F;

	byte full_result = lower_nibble_1 + lower_nibble_2;

	// if first byte of full_result is not 0 
	// it has been used and flag would be set

	byte important_bit = full_result & 0b00010000;

	if (important_bit == 0) {
		return false;
	}

	return true;

}

Reg8 CPU::decode_reg8_bits(byte reg_3_bit_code) {
	switch (reg_3_bit_code) {
	case 0:
		return B;
	case 1:
		return C;
	case 2:
		return D;
	case 3:
		return E;
	case 4:
		return H;
	case 5:
		return L;
	case 6:
		return HL_LOC;
	default:
		return NONE;
	}
}

Reg16 CPU::decode_reg16_bits(byte reg_2_bit_code) {
	switch (reg_2_bit_code) {
	case 0:
		return BC;
	case 1:
		return DE;
	case 2:
		return HL;
	case 3:
		return SP;
	}
}

Reg16 CPU::decode_reg16_stk_bits(byte reg_2_bit_code) {
	switch (reg_2_bit_code) {
	case 0:
		return BC;
	case 1:
		return DE;
	case 2:
		return HL;
	case 3:
		return AF;
	}
}

//TODO: think about how to do the r16_mem decoding
// it has hl+ and hl- i havent figured out yet



// LOAD INSTRUCTIONS

void CPU::ld(Reg8 save_loc, Reg8 reg_to_copy) {
	regs.regs_8b[save_loc] = regs.regs_8b[reg_to_copy];
}

void CPU::ld(Reg8 save_loc, byte val) {
	regs.regs_8b[save_loc] = val;
}

void CPU::ld(Reg16 save_loc, word val) {
	regs.set_Reg16(save_loc, val);
}

void CPU::ld_to_HL_loc(Reg8 val_loc) {
	word save_loc = regs.get_HL();
	bus.write_memory(save_loc, regs.regs_8b[val_loc]);
}


void CPU::ld_to_HL_loc(byte val) {
	word save_loc = regs.get_HL();
	bus.write_memory(save_loc, val);
}

void CPU::ld_to_reg_HL(Reg8 save_loc) {
	word data_loc = regs.get_HL();
	regs.regs_8b[save_loc] = bus.read_memory(data_loc);
}

void CPU::ld_to_mem_A(Reg16 save_loc) {
	byte data = regs.regs_8b[A];
	word save_address = regs.get_Reg16(save_loc);
	bus.write_memory(save_address, data);
}

void CPU::ld_to_mem_A(word save_loc) {
	byte data = regs.regs_8b[A];
	bus.write_memory(save_loc, data);
}

void CPU::ld_to_ioC_A() {
	byte data = regs.regs_8b[A];
	word save_loc = 0xFF00 + regs.regs_8b[C];
	bus.write_memory(save_loc, data);
}

void CPU::ld_to_A_mem(Reg16 val_loc) {
	word data_address = regs.get_Reg16(val_loc);
	byte data = bus.read_memory(data_address);
	regs.regs_8b[A] = data;
}

void CPU::ld_to_A_mem(word val_loc) {
	byte data = bus.read_memory(val_loc);
	regs.regs_8b[A] = data;
}

void CPU::ld_to_A_C() {
	word data_address = 0xFF00 + regs.regs_8b[C];
	byte data = bus.read_memory(data_address);
	regs.regs_8b[A] = data;
}

void CPU::ld_to_HLI_loc_A() {
	byte data = regs.regs_8b[A];
	word address = regs.get_HL();
	bus.write_memory(address, data);
	regs.set_HL(address + 1);
}

void CPU::ld_to_HLD_loc_A() {
	byte data = regs.regs_8b[A];
	word address = regs.get_HL();
	bus.write_memory(address, data);
	regs.set_HL(address - 1);
}

void CPU::ld_to_A_HLI_loc() {
	word address = regs.get_HL();
	byte data = bus.read_memory(address);
	regs.regs_8b[A] = data;
	regs.set_HL(address + 1);
}

void CPU::ld_to_A_HLD_loc() {
	word address = regs.get_HL();
	byte data = bus.read_memory(address);
	regs.regs_8b[A] = data;
	regs.set_HL(address - 1);
}


void CPU::ld_sp(word val) {
	regs.SP = val;
}

void CPU::ld_to_mem_SP(word save_loc) {
	byte lower_half = regs.SP & 0xFF;
	byte upper_half = regs.SP >> 8;

	bus.write_memory(save_loc, lower_half);
	bus.write_memory(save_loc + 1, upper_half);
}



//bit of a whacky one, good chance it breaks laters
void CPU::ld_to_HL_SP(sbyte offset) {
	regs.set_HL(regs.SP + offset);

	regs.set_z_flag(0);
	regs.set_n_flag(0);


	byte half_carry = ((regs.SP & 0xF) + (offset & 0xF)) > 0xF;
	byte carry = ((regs.SP & 0xFF) + offset) > 0xFF;

	regs.set_h_flag(half_carry);
	regs.set_c_flag(carry);

}

void CPU::ld_to_SP_HL() {
	regs.SP = regs.get_HL();
}


//	ARITHMETIC INSTRUCTIONS

void CPU::add_a_values_set_flags(byte val1, byte val2) {

	bool z, n, h, c;

	h = half_carry_add_8(val1, val2);
	c = carry_add_8(val1, val2);

	regs.regs_8b[A] = val1 + val2;

	z = (regs.regs_8b[A] == 0);

	n = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::add_a_values_set_flags(byte val1, byte val2, byte val3) {

	bool z, n, h, c;

	h = half_carry_add_8(val1, val2, val3);
	c = carry_add_8(val1, val2, val3);

	regs.regs_8b[A] = val1 + val2 + val3;

	z = (regs.regs_8b[A] == 0);

	n = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::adc_a(Reg8 val_loc) {
	byte c_flag = regs.get_c_flag();
	byte original_a = regs.regs_8b[A];
	byte val_to_add = regs.regs_8b[val_loc];

	add_a_values_set_flags(c_flag, original_a, val_to_add);

}

void CPU::adc_a_hl(Reg8 val_loc) {

	byte val_to_add = bus.read_memory(regs.get_HL());
	byte original_a = regs.regs_8b[A];
	byte c_flag = regs.get_c_flag();

	add_a_values_set_flags(c_flag, original_a, val_to_add);

}

void CPU::adc_a(byte val_to_add) {

	byte original_a = regs.regs_8b[A];
	byte c_flag = regs.get_c_flag();

	add_a_values_set_flags(c_flag, original_a, val_to_add);

}

void CPU::add_a(Reg8 val_loc) {

	byte original_a = regs.regs_8b[A];
	byte val_to_add = regs.regs_8b[val_loc];

	add_a_values_set_flags(original_a, val_to_add);

}

void CPU::add_a_HL() {

	byte original_a = regs.regs_8b[A];
	byte val_to_add = bus.read_memory(regs.get_HL());

	add_a_values_set_flags(original_a, val_to_add);

}



void CPU::add_HL_SP() {
	word val_to_add = regs.SP;
	word orginal_hl = regs.get_HL();


	word real_result = orginal_hl + val_to_add;
	uint32_t full_result = orginal_hl + val_to_add;

	bool c = (real_result != full_result);

	//checking overfllow on bit 11 so need first 3 nibbles
	word lower_section_1 = orginal_hl & 0x0FFF;
	word lower_section_2 = val_to_add & 0x0FFF;

	full_result = lower_section_1 + lower_section_2;

	// if first byte of full_result is not 0 
	// it has been used and flag would be set

	0x1000;

	word important_bit = full_result & 0x1000;

	bool h = (important_bit != 0);

	regs.set_HL(orginal_hl + val_to_add);
	regs.set_n_flag(0);
	regs.set_c_flag(c);
	regs.set_h_flag(h);
}

void CPU::add_SP(sbyte val_to_add) {
	word original_sp = regs.SP;

	bool z, n, h, c;

	h = half_carry_add_16(original_sp, val_to_add);
	c = carry_add_16(original_sp, val_to_add);

	regs.SP = original_sp + val_to_add;

	z = 0;

	n = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::dec(Reg8 val_loc) {
	regs.regs_8b[val_loc]--;
}

void CPU::inc(Reg8 val_loc) {
	regs.regs_8b[val_loc]++ ;
}



// 16 BIT ARITHMETIC

void CPU::add_HL(Reg16 val_loc) {
	word val_to_add = regs.get_Reg16(val_loc);
	word orginal_hl = regs.get_HL();


	word real_result = orginal_hl + val_to_add;
	uint32_t full_result = orginal_hl + val_to_add;

	bool c = (real_result != full_result);

	//checking overfllow on bit 11 so need first 3 nibbles
	word lower_section_1 = orginal_hl & 0x0FFF;
	word lower_section_2 = val_to_add & 0x0FFF;

	full_result = lower_section_1 + lower_section_2;

	// if first byte of full_result is not 0 
	// it has been used and flag would be set


	word important_bit = full_result & 0x1000;

	bool h = (important_bit != 0);

	regs.set_HL(orginal_hl + val_to_add);

	regs.set_n_flag(0);
	regs.set_c_flag(c);
	regs.set_h_flag(h);
}


void CPU::dec(Reg16 val_loc) {
	regs.set_Reg16(val_loc, regs.get_Reg16(val_loc) - 1);
}

void CPU::inc(Reg16 val_loc) {
	regs.set_Reg16(val_loc, regs.get_Reg16(val_loc) + 1);
}


//	BITWISE OPERATIONS


void CPU::and_a(Reg8 reg_id) {

	byte val_to_compare = regs.regs_8b[reg_id];
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] != 0);
	n = 0;
	h = 1;
	c = 0;
	regs.set_flags(z, n, h, c);

}



void CPU::cpl() {

	byte old_a = regs.regs_8b[A];
	byte new_result = 0;

	for (byte i = 0; i < 8; i++) {

		byte current_bit = old_a & 0x1;

		if (current_bit == 0) {
			new_result += 1 << i;
		}

		old_a = old_a >> 1;
	}

}




// BITSHIFT INSTRUCTIONS

void CPU::rlca() {
	
	byte old_a = regs.regs_8b[A];
	byte left_bit = old_a & 0x80;
	
	bool c = (left_bit > 0);

	regs.regs_8b[A] = (old_a << 1) + c;

	regs.set_flags(0,0,0,c);
}

void CPU::rrca() {

	byte old_a = regs.regs_8b[A];
	byte right_bit = old_a & 0x01;

	bool c = (right_bit > 0);

	regs.regs_8b[A] = (old_a >> 1) + (c * 0x80);

	regs.set_flags(0, 0, 0, c);
}

void CPU::rla() {

	byte old_a = regs.regs_8b[A];
	bool old_c = regs.get_c_flag();
	byte left_bit = old_a & 0x80;


	bool new_c = (left_bit > 0);

	regs.regs_8b[A] = (old_a << 1) + old_c;

	regs.set_flags(0, 0, 0, new_c);
}

void CPU::rra() {

	byte old_a = regs.regs_8b[A];
	bool old_c = regs.get_c_flag();
	byte right_bit = old_a & 0x01;

	bool new_c = (right_bit > 0);

	regs.regs_8b[A] = (old_a >> 1) + (old_c * 0x80);

	regs.set_flags(0, 0, 0, new_c);

}

// JUMPS AND SUBROUTINES INSTRUCTIONS

void CPU::jr(sbyte offset) {
	regs.PC += offset;
}

void CPU::jr_cond(Cond condition, sbyte offset) {
	
	bool condition_met = false;

	switch (condition) {
	case nz:
		if (regs.get_z_flag() == 0) {
			condition_met = true;
		}
		break;
	case z:
		if (regs.get_z_flag() == 1) {
			condition_met = true;
		}
		break;
	case nc:
		if (regs.get_c_flag() == 0) {
			condition_met = true;
		}
		break;

	case c:
		if (regs.get_c_flag() == 1) {
			condition_met = true;
		}
		break;
	}
	
	if (condition_met) {
		regs.PC += offset;
	}

}

// CARRY FLAG INSTRUCTIONS

void CPU::ccf() {

	regs.set_n_flag(0);
	regs.set_h_flag(0);

	if (regs.get_c_flag() > 0) {
		regs.set_c_flag(0);
	}
	else {
		regs.set_c_flag(1);
	}


}


void CPU::scf() {

	regs.set_n_flag(0);
	regs.set_h_flag(0);
	regs.set_c_flag(1);

}




// MISC INSTRUCTIONS
void CPU::daa() {

	byte adjustment = 0;
	bool n = regs.get_n_flag();
	bool h = regs.get_h_flag();
	bool c = regs.get_c_flag();

	if (n) {
		if (h) {
			adjustment += 0x6;
		}
		if (c) {
			adjustment += 0x60;
		}

		regs.regs_8b[A] -= adjustment;
	}
	else {
		if (h || (regs.regs_8b[A] & 0xF) > 0x9) {
			adjustment += 0x6;
		}
		if (c || regs.regs_8b[A] > 0x99 ){
			adjustment += 60;
			regs.set_c_flag(1);
		}

		regs.regs_8b[A] += adjustment;


	}

	bool z = (regs.regs_8b[A] != 0);
	regs.set_z_flag(z);
	regs.set_h_flag(0);
}



