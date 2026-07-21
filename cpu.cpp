#include "cpu.h"


byte CPU::fetch() {

	byte instruction = bus.read_memory(regs.PC);
	regs.PC ++;

	return instruction;
}



void CPU::decode(byte instruction) {

	// the blocks are based on https://gbdev.io/pandocs/CPU_Instruction_Set.html
	byte block_decider = (instruction & 0b11000000) >> 6;


	switch (block_decider) {
	case 0:
		decode_block_0(instruction);
		break;

	case 1:

		break;
	
	case 2:

		break;

	case 3:

		break;
	}

}

void CPU::decode_block_0(byte instruction) {

	// TODO: add nop and stop

	byte last_4_bits = instruction & 0xF;

	byte decider_bits = (instruction & 0x30) >> 4;
	switch (last_4_bits) {

	case 0x1:{
		Reg16 dst_reg = decode_reg16_bits(decider_bits);
		word val_to_ld = fetch_16();
		//ld r16, imm16
		ld(dst_reg, val_to_ld);
		return;
	}

	case 0x2: {
		//ld [r16mem], a
		Reg16 save_address_loc = decode_reg16_mem_bits(decider_bits);
		ld_to_mem_A(save_address_loc);
		return;
	}

	case 0xA: {
		//ld a, [r16mem]
		Reg16 data_address_loc = decode_reg16_mem_bits(decider_bits);
		ld_to_A_mem(data_address_loc);
		return;
	}

	case 0x8: {
		//ld[imm16], sp
		word address = fetch_16();
		ld_to_mem_SP(address);
		return;
	}


	case 0x3: {
		//inc r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		inc(reg);
		return;
	}
	case 0xB: {
		//dec r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		dec(reg);
		return;
	}

	case 0x9: {
		//add hl, r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		add_HL(reg);
		return;
	}

	}

	byte last_3_bits = instruction & 0x7;

	decider_bits = (instruction & 0x38) >> 3;

	switch (last_3_bits) {
	case 0x4: {
		//inc r8
		Reg8 reg = decode_reg8_bits(decider_bits);
		inc(reg);
		return;
	}

	case 0x5: {
		//dec r8
		Reg8 reg = decode_reg8_bits(decider_bits);
		dec(reg);
		return;
	}

	case 0x6: {
		//ld r8, imm8
		Reg8 reg = decode_reg8_bits(decider_bits);
		byte val_to_add = fetch();
		ld(reg, val_to_add);
		return;
	}

	case 0x0: {

		if (instruction & 0x20) {
			byte cond_bits = (instruction & 0x18) >> 3;
			Cond cond = decode_cond_bits(cond_bits);
			sbyte offset = fetch();
			//jr cond, imm8
			jr_cond(cond, offset);
			return;
		}
		//other ending in 000 dealt with below as
		//they have full set 8 bits all the time
		break;
	}
	}

	switch (instruction) {
	case 0x7:
		//rlca
		rlca();
		return;
	case 0xF:
		//rrca
		rrca();
		return;
	case 0x17:
		//rla
		rla();
		return;
	case 0x1F:
		//rra
		rra();
		return;
	case 0x27:
		//daa
		daa();
		return;
	case 0x2F:
		//cpl
		cpl();
		return;
	case 0x37:
		//scf
		scf();
		return;
	case 0x3F:
		//ccf
		ccf();
		return;


	
	case 0x18: {
		//jr imm8
		sbyte offset = fetch();
		jr(offset);
		return;
	}

	case 0x10:
		//stop
		break;

	}


}


//HELPERS

byte CPU::get_Reg8(Reg8 reg) {

	if (reg == HL_LOC) {
		return bus.read_memory(regs.get_HL());
	}
	return regs.regs_8b[reg];
}

void CPU::set_Reg8(Reg8 reg, byte value) {
	if (reg == HL_LOC)
	{
		bus.write_memory(regs.get_HL(), value);
		return;
	}

	regs.regs_8b[reg] = value;
}




word CPU::fetch_16() {

	byte lower = fetch();
	byte higher = fetch();

	return (higher << 8) | lower;
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

Reg16 CPU::decode_reg16_mem_bits(byte reg_2_bit_code) {
	
	switch (reg_2_bit_code) {
	case 0:
		return BC;
	case 1:
		return DE;
	case 2:
		return HLI;
	case 3:
		return HLD;
	}

}

//TODO: think about how to do the r16_mem decoding
// it has hl+ and hl- i havent figured out yet


Cond CPU::decode_cond_bits(byte cond_2_bit_code) {
	switch (cond_2_bit_code) {
	case 0:
		return nz;
	case 1:
		return z;
	case 2:
		return nc;
	case 3:
		return c;
	}
}



//	BITWISE OPERATIONS


void CPU::and_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 1;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::and_a_hl() {

	byte val_to_compare = bus.read_memory(regs.get_HL());
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 1;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::and_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a & val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
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

void CPU::or_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a | val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::or_a_hl() {

	byte val_to_compare = bus.read_memory(regs.get_HL());
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a | val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::or_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a | val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::xor_a(Reg8 reg_id) {

	byte val_to_compare = get_Reg8(reg_id);
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a ^ val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::xor_a_hl() {

	byte val_to_compare = bus.read_memory(regs.get_HL());
	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a ^ val_to_compare;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}

void CPU::xor_a(byte val) {

	byte original_a = regs.regs_8b[A];

	regs.regs_8b[A] = original_a ^ val;

	bool z, n, h, c;

	z = (regs.regs_8b[A] == 0);
	n = 0;
	h = 0;
	c = 0;
	regs.set_flags(z, n, h, c);

}


// BITFLAG INSTRUCTIONS
void CPU::bit(byte test_bit, byte value) {

	bool z = false;
	if (value & (1 << test_bit)){
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



