#include "cpu.h"


byte CPU::fetch() {

	byte instruction = bus.read_memory(regs.PC);
	regs.PC ++;

	return instruction;
}

word CPU::fetch_16() {

	byte lower = fetch();
	byte higher = fetch();

	return (higher << 8) | lower;
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
		decode_block_3(instruction);
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
		ld_to_mem_sp(address);
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

void CPU::decode_block_3(byte instruction) {

	switch (instruction) {
	case 0xC6:
		add_a(fetch());
		return;
	case 0xCE:
		adc_a(fetch());
		return;
	case 0xD6:
		sub_a(fetch());
		return;
	case 0xDE:
		sbc_a(fetch());
		return;
	case 0xE6:
		and_a(fetch());
		return;
	case 0xEE:
		xor_a(fetch());
		return;
	case 0xF6:
		or_a(fetch());
		return;
	case 0xFE:
		cp_a(fetch());
		return;


	case 0xC9:
		ret();
		return;
	case 0xD9:
		ret_i();
		return;
	case 0xC3:
		jp(fetch_16());
		return;
	case 0xE9:
		jp_hl();
		return;
	case 0xCD:
		call(fetch_16());
		return;


	case 0xCB:
		decode_prefixed(fetch());
		return;


	case 0xE2:
		ldh_c_a();
		return;
	case 0xE0:
		ld_to_mem_A(0xFF|fetch());
		return;
	case 0xEA:
		ld_to_mem_A(fetch_16());
		return;
	case 0xF2:
		ldh_a_c();
		return;
	case 0xF0:
		ld_to_A_mem(0xFF | fetch());
		return;
	case 0xFA:
		ld_to_A_mem(0xFF | fetch_16());
		return;



	case 0xE8:
		add_SP(fetch());
		return;
	case 0xF8:
		ld_to_hl_spe8(fetch());
		return;
	case 0xF9:
		ld_sp_hl();
		return;


	case 0xF3:
		di();
		return;
	case 0xFB:
		ei();
		return;
	}

	// other ones in this section have some other data within instruction

	byte last_3_bits = instruction & 0x7;

	switch (last_3_bits) {
	case 0x0: {
		byte cond_bits = instruction & 0x18 >> 3;
		ret(decode_cond_bits(cond_bits));
		return;
	}

	case 0x2: {
		byte cond_bits = instruction & 0x18 >> 3;
		jp(fetch_16(), decode_cond_bits(cond_bits));
		return;
	}

	case 0x4: {
		byte cond_bits = instruction & 0x18 >> 3;
		call(fetch_16(), decode_cond_bits(cond_bits));
		return;
	}

	case 0x7: {
		byte target_bits = instruction & 0x38 >> 3;
		rst(target_bits << 3); //multiply by 8
		return;
	}

	case 0x1: {
		byte reg_stack_bits = instruction & 0x30 >> 4;
		pop(decode_reg16_stk_bits(reg_stack_bits));
		return;
	}

	case 0x5: {
		byte reg_stack_bits = instruction & 0x30 >> 4;
		push(decode_reg16_stk_bits(reg_stack_bits));
		return;
	}
	}


}

void CPU::decode_prefixed(byte instruction) {

	byte decider_bits = instruction & 0xF8;
	byte reg8_bits = instruction & 0x7;

	switch (decider_bits) {
	case 0x0:
		rlc(decode_reg8_bits(reg8_bits));
		return;
	case 0x08:
		rrc(decode_reg8_bits(reg8_bits));
		return;
	case 0x10:
		rl(decode_reg8_bits(reg8_bits));
		return;
	case 0x18:
		rr(decode_reg8_bits(reg8_bits));
		return;
	case 0x20:
		sla(decode_reg8_bits(reg8_bits));
		return;
	case 0x28:
		sra(decode_reg8_bits(reg8_bits));
		return;
	case 0x30:
		swap(decode_reg8_bits(reg8_bits));
		return;
	case 0x38:
		srl(decode_reg8_bits(reg8_bits));
		return;
	}

	decider_bits = instruction & 0xC0;
	byte bit_index = (instruction & 0x38) >> 3;


	switch (decider_bits) {
	case 0x40: {
		byte val = get_Reg8(decode_reg8_bits(reg8_bits));
		bit(bit_index, val);
		return;
	}

	case 0x80: {
		res(bit_index, decode_reg8_bits(reg8_bits));
		return;
	}

	case 0xC0: {
		set(bit_index, decode_reg8_bits(reg8_bits));
		return;
	}

	}

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







