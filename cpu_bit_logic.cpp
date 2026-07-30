#include "cpu.h"


//	BITWISE OPERATIONS


byte CPU::and_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 1;
	c = 0;
	regs.set_flags(z, n, h, c);

	if (reg_id == HL_LOC) {
		return 2;
	}
	return 1;

}


byte CPU::and_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 1;
	c = 0;
	regs.set_flags(z, n, h, c);

	return 2;

}



byte CPU::cpl() {

	byte old_a = regs.regs_8b[A];
	byte new_result = 0;

	for (byte i = 0; i < 8; i++) {

		byte current_bit = old_a & 0x1;

		if (current_bit == 0) {
			new_result += 1 << i;
		}

		old_a = old_a >> 1;
	}

	return 1;

}

byte CPU::or_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a | val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

	if (reg_id == HL_LOC) {
		return 2;
	}
	return 1;

}


byte CPU::or_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a | val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

	return 2;

}

byte CPU::xor_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a ^ val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

	if (reg_id == HL_LOC) {
		return 2;
	}
	return 1;

}


byte CPU::xor_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a ^ val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

	return 2;

}



// BITFLAG INSTRUCTIONS
void CPU::bit(byte test_bit, byte value) {

	bool z = false;
	if (value & (1 << test_bit)) {
		z = true;
	}
	regs.set_z_flag(z);
	regs.set_n_flag(0);
	regs.set_h_flag(1);

}

void CPU::res(byte test_bit, Reg8 val_loc) {

	byte value = get_Reg8(val_loc);
	value &= ~(1 << test_bit);

	set_Reg8(val_loc, value);
}


void CPU::set(byte test_bit, Reg8 val_loc) {

	byte value = get_Reg8(val_loc);
	value |= (1 << test_bit);

	set_Reg8(val_loc, value);
}



// BITSHIFT INSTRUCTIONS

byte CPU::rl(Reg8 reg_to_rotate) {

	byte old_val = get_Reg8(reg_to_rotate);
	bool old_c = regs.get_c_flag();
	byte left_bit = old_val & 0x80;

	bool new_c = (left_bit > 0);

	byte new_val = (old_val << 1) + old_c;
	bool z = (new_val == 0);

	set_Reg8(reg_to_rotate, new_val);

	regs.set_flags(z, 0, 0, new_c);

	return 2;
}

byte CPU::rla() {
	rl(A);
	regs.set_z_flag(0);

	return 1;
}


byte CPU::rlc(Reg8 reg_to_rotate) {

	byte old_val = get_Reg8(reg_to_rotate);
	byte left_bit = old_val & 0x80;

	bool c = (left_bit > 0);

	byte new_val = (old_val << 1) + c;
	bool z = (new_val == 0);
	set_Reg8(reg_to_rotate, new_val);

	regs.set_flags(z, 0, 0, c);

	return 2;
}


byte CPU::rlca() {
	rlc(A);
	regs.set_z_flag(0);
	return 1;
}


byte CPU::rr(Reg8 reg_to_rotate) {

	byte old_val = get_Reg8(reg_to_rotate);
	bool old_c = regs.get_c_flag();
	byte right_bit = old_val & 0x01;

	bool new_c = (right_bit > 0);

	byte new_val = (old_val >> 1) + (old_c << 7);
	bool z = (new_val == 0);
	set_Reg8(reg_to_rotate, new_val);

	regs.set_flags(z, 0, 0, new_c);

	return 2;

}

byte CPU::rra() {
	rr(A);
	regs.set_z_flag(0);

	return 1;
}

byte CPU::rrc(Reg8 reg_to_rotate) {

	byte old_val = get_Reg8(reg_to_rotate);
	byte right_bit = old_val & 0x01;

	bool c = (right_bit > 0);

	byte new_val = (old_val >> 1) + (c << 7);
	bool z = (new_val == 0);
	set_Reg8(reg_to_rotate, new_val);

	regs.set_flags(z, 0, 0, c);

	return 2;

}


byte CPU::rrca() {
	rrc(A);
	regs.set_z_flag(0);

	return 1;
}


byte CPU::sla(Reg8 reg_to_shift) {

	byte old_val = get_Reg8(reg_to_shift);
	byte left_bit = old_val & 0x80;
	bool c = (left_bit > 0);

	byte new_val = old_val << 1;
	bool z = (new_val == 0);
	set_Reg8(reg_to_shift, new_val);

	regs.set_flags(z, 0, 0, c);

	return 2;

}

byte CPU::sra(Reg8 reg_to_shift) {

	byte old_val = get_Reg8(reg_to_shift);
	byte right_bit = old_val & 0x01;
	byte left_bit = old_val & 0x80;
	bool c = (right_bit > 0);

	byte new_val = (old_val >> 1) + left_bit;
	bool z = (new_val == 0);
	set_Reg8(reg_to_shift, new_val);

	regs.set_flags(z, 0, 0, c);

	return 2;

}

byte CPU::srl(Reg8 reg_to_shift) {

	byte old_val = get_Reg8(reg_to_shift);
	byte right_bit = old_val & 0x01;
	bool c = (right_bit > 0);

	byte new_val = old_val >> 1;
	bool z = (new_val == 0);
	set_Reg8(reg_to_shift, new_val);

	regs.set_flags(z, 0, 0, c);

	return 2;
}

byte CPU::swap(Reg8 reg_to_shift) {

	byte old_val = get_Reg8(reg_to_shift);
	byte old_lower = old_val & 0x0F;
	byte old_upper = old_val & 0xF0;


	byte new_val = (old_lower << 4) | ((old_upper >> 4));
	bool z = (new_val == 0);
	set_Reg8(reg_to_shift, new_val);

	regs.set_flags(z, 0, 0, 0);

	return 2;
}
