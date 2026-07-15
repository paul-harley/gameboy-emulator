#include "cpu.h"

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
		return;
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






