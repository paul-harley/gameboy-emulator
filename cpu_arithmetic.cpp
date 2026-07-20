#include "cpu.h"


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

bool CPU::carry_sub_8(byte a, byte val1) {

	if (val1 > a) {
		return true;
	}
	return false;
}


bool CPU::carry_sub_8(byte a, byte val1, byte val2) {

	if (val1 + val2 > a) {
		return true;
	}
	return false;
}

bool CPU::half_carry_sub_8(byte a, byte val1) {
	if ((a & 0xF) < (val1 & 0xF)) {
		return true;
	}
	return false;
}

bool CPU::half_carry_sub_8(byte a, byte val1, byte val2) {
	if ( (a & 0xF) < ((val1 & 0xF) + val2)) {
		return true;
	}
	return false;
}






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

void CPU::sub_a_values_set_flags(byte a, byte val1, byte val2) {

	regs.regs_8b[A] = a - val1 - val2;
	bool z, n, h, c;

	c = carry_sub_8(a, val1, val2);
	h = half_carry_sub_8(a, val1, val2);
	z = (regs.regs_8b[A] == 0);
	n = 1;
	regs.set_flags(z, n, h, c);
}

void CPU::sub_a_values_set_flags(byte a, byte val1) {

	regs.regs_8b[A] = a - val1;
	bool z, n, h, c;

	c = carry_sub_8(a, val1);
	h = half_carry_sub_8(a, val1);
	z = (regs.regs_8b[A] == 0);
	n = 1;
	regs.set_flags(z, n, h, c);
}


void CPU::adc_a(Reg8 val_loc) {
	byte c_flag = regs.get_c_flag();
	byte original_a = regs.regs_8b[A];
	byte val_to_add = get_Reg8(val_loc);

	add_a_values_set_flags(c_flag, original_a, val_to_add);

}

void CPU::adc_a_hl() {

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
	byte val_to_add = get_Reg8(val_loc);

	add_a_values_set_flags(original_a, val_to_add);

}

void CPU::add_a_hl() {

	byte original_a = regs.regs_8b[A];
	byte val_to_add = bus.read_memory(regs.get_HL());

	add_a_values_set_flags(original_a, val_to_add);

}

void CPU::add_a(byte val_to_add) {

	byte original_a = regs.regs_8b[A];

	add_a_values_set_flags(original_a, val_to_add);
}


void CPU::cp_a(Reg8 val_loc) {

	byte reg_val = get_Reg8(val_loc);
	byte original_a = regs.regs_8b[A];

	bool z = ((original_a - reg_val) == 0);
	regs.set_z_flag(z);
	regs.set_n_flag(1);

	bool c = (reg_val > original_a);
	regs.set_c_flag(c);

	bool h = (original_a & 0xF) < (reg_val & 0xF);
	regs.set_h_flag(h);

}

void CPU::cp_a_hl() {

	byte val = bus.read_memory(regs.get_HL());
	byte original_a = regs.regs_8b[A];

	bool z = ((original_a - val) == 0);
	regs.set_z_flag(z);
	regs.set_n_flag(1);

	bool c = (val > original_a);
	regs.set_c_flag(c);

	bool h = (original_a & 0xF) < (val & 0xF);
	regs.set_h_flag(h);

}

void CPU::cp_a(byte val) {

	byte original_a = regs.regs_8b[A];

	bool z = ((original_a - val) == 0);
	regs.set_z_flag(z);
	regs.set_n_flag(1);

	bool c = (val > original_a);
	regs.set_c_flag(c);

	bool h = (original_a & 0xF) < (val & 0xF);
	regs.set_h_flag(h);

}






void CPU::add_hl_sp() {
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
	byte new_val = (get_Reg8(val_loc)) - 1;
	set_Reg8(val_loc, new_val);
}

void CPU::inc(Reg8 val_loc) {
	byte new_val = (get_Reg8(val_loc)) - 1;
	set_Reg8(val_loc, new_val);
}


// might refactor these into a separate function like add later

void CPU::sbc_a(Reg8 val_loc) {

	byte original_a = regs.regs_8b[A];
	byte val_to_sub = get_Reg8(val_loc);
	byte c_flag = regs.get_c_flag();

	sub_a_values_set_flags(original_a, val_to_sub, c_flag);
}


void CPU::sbc_a_hl() {

	byte original_a = regs.regs_8b[A];
	byte val_to_sub = bus.read_memory(regs.get_HL());
	byte c_flag = regs.get_c_flag();

	sub_a_values_set_flags(original_a, val_to_sub, c_flag);
}

void CPU::sbc_a(byte val_to_sub) {

	byte original_a = regs.regs_8b[A];
	byte c_flag = regs.get_c_flag();

	sub_a_values_set_flags(original_a, val_to_sub, c_flag);
}

void CPU::sub_a(Reg8 val_loc) {

	byte original_a = regs.regs_8b[A];
	byte val_to_sub = get_Reg8(val_loc);

	sub_a_values_set_flags(original_a, val_to_sub);
}


void CPU::sub_a_hl() {
	byte original_a = regs.regs_8b[A];
	byte val_to_sub = bus.read_memory(regs.get_HL());

	sub_a_values_set_flags(original_a, val_to_sub);
}

void CPU::sub_a(byte val_to_sub) {
	byte original_a = regs.regs_8b[A];

	sub_a_values_set_flags(original_a, val_to_sub);
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
