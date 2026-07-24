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
		decode_block_1(instruction);
		break;
	
	case 2:
		decode_block_2(instruction);
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
			jr(offset, cond);
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


void CPU::decode_block_1(byte instruction) {
	switch (instruction) {
	case 0x76:
		halt();
		break;
	default:
	{
		Reg8 dst = decode_reg8_bits(instruction & 0x38);
		Reg8 src = decode_reg8_bits(instruction & 0x07);

		ld(dst, src);
		break;
	}
	}
}

void CPU::decode_block_2(byte instruction) {

	byte operation_bits = instruction & 0xF8;
	Reg8 reg = decode_reg8_bits(instruction & 0x7);

	switch (operation_bits) {

	case 0x80:
		add_a(reg);
		break;
	case 0x81:
		adc_a(reg);
		break;
	case 0x90:
		sub_a(reg);
		break;
	case 0x91:
		sbc_a(reg);
		break;
	case 0xA0:
		and_a(reg);
		break;
	case 0xA1:
		xor_a(reg);
		break;
	case 0xB0:
		or_a(reg);
		break;
	case 0xB1:
		cp_a(reg);
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
	else if (reg == F) {
		value &= 0xF0; 
	}

	regs.regs_8b[reg] = value;
}




word CPU::fetch_16() {

	byte lower = fetch();
	byte higher = fetch();

	return (higher << 8) | lower;
}


bool CPU::check_conds(Cond cond) {

	switch (cond) {
	case nz:
		if (regs.get_z_flag() == 0) {
			return true;
		}
		break;
	case z:
		if (regs.get_z_flag() == 1) {
			return true;
		}
		break;
	case nc:
		if (regs.get_c_flag() == 0) {
			return true;
		}
		break;

	case c:
		if (regs.get_c_flag() == 1) {
			return  true;
		}
		break;
	}

	return false;

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



//INTERUPTS 

void CPU::di() {
	ime = false;
}

void CPU::ei() {
	ime = true;
}

void CPU::halt() {

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





