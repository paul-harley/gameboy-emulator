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


//TODO: just did all the cycles added
// check each one for possible [hl] as it will be higher
// only on reg8 ones 


 byte CPU::decode(byte instruction) {

	// the blocks are based on https://gbdev.io/pandocs/CPU_Instruction_Set.html
	byte block_decider = (instruction & 0b11000000) >> 6;

	switch (block_decider) {
	case 0:
		return decode_block_0(instruction);
		break;

	case 1:
		return decode_block_1(instruction);
		break;
	
	case 2:
		return decode_block_2(instruction);
		break;

	case 3:
		return decode_block_3(instruction);
		break;
	}

}

byte CPU::decode_block_0(byte instruction) {

	// TODO: add nop and stop

	switch (instruction) {
	case 0:
		return 1;

	case 0x8: {
		//ld[imm16], sp
		word address = fetch_16();
		return ld_to_mem_sp(address);
	}

	case 0x7:
		//rlca
		return rlca();
	case 0xF:
		//rrca
		return rrca();
	case 0x17:
		//rla
		return rla();
	case 0x1F:
		//rra
		return rra();
	case 0x27:
		//daa
		return daa();
	case 0x2F:
		//cpl
		return cpl();
	case 0x37:
		//scf
		return scf();
	case 0x3F:
		//ccf
		return ccf();


	
	case 0x18: {
		//jr imm8
		sbyte offset = fetch();
		return jr(offset);
	}

	case 0x10:
		//stop
		return 0;

	}

	byte last_4_bits = instruction & 0xF;
	byte decider_bits = (instruction & 0x30) >> 4;
	switch (last_4_bits) {

	case 0x1:{
		Reg16 dst_reg = decode_reg16_bits(decider_bits);
		word val_to_ld = fetch_16();
		//ld r16, imm16
		return ld(dst_reg, val_to_ld);
	}

	case 0x2: {
		//ld [r16mem], a
		Reg16 save_address_loc = decode_reg16_mem_bits(decider_bits);
		return ld_to_mem_A(save_address_loc);
	}

	case 0xA: {
		//ld a, [r16mem]
		Reg16 data_address_loc = decode_reg16_mem_bits(decider_bits);
		return ld_to_A_mem(data_address_loc);
	}

	case 0x3: {
		//inc r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		return inc(reg);
		
	}
	case 0xB: {
		//dec r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		return dec(reg);
	}

	case 0x9: {
		//add hl, r16
		Reg16 reg = decode_reg16_bits(decider_bits);
		return add_HL(reg);
	}

	}

	byte last_3_bits = instruction & 0x7;

	decider_bits = (instruction & 0x38) >> 3;

	switch (last_3_bits) {
	case 0x4: {
		//inc r8
		Reg8 reg = decode_reg8_bits(decider_bits);
		return inc(reg);
		
	}

	case 0x5: {
		//dec r8
		Reg8 reg = decode_reg8_bits(decider_bits);
		return dec(reg);
		
	}

	case 0x6: {
		//ld r8, imm8
		Reg8 reg = decode_reg8_bits(decider_bits);
		byte val_to_add = fetch();
		return ld(reg, val_to_add);
	}

	case 0x0: {

		if (instruction & 0x20) {
			byte cond_bits = (instruction & 0x18) >> 3;
			Cond cond = decode_cond_bits(cond_bits);
			sbyte offset = fetch();
			//jr cond, imm8
			return jr(offset, cond);
			
		}
		//other ending in 000 dealt with below as
		//they have full set 8 bits all the time
		break;
	}
	}

}

byte CPU::decode_block_1(byte instruction) {
	switch (instruction) {
	case 0x76:
		halt();
		return 0;
	default:
	{
		Reg8 dst = decode_reg8_bits((instruction & 0x38) >> 3);
		Reg8 src = decode_reg8_bits(instruction & 0x07);

		return ld(dst, src);
	}
	}
}

byte CPU::decode_block_2(byte instruction) {

	byte operation_bits = instruction & 0xF8;
	Reg8 reg = decode_reg8_bits(instruction & 0x7);

	switch (operation_bits) {

	case 0x80:
		return add_a(reg);
	case 0x88:
		return adc_a(reg);
	case 0x90:
		return sub_a(reg);
	case 0x98:
		return sbc_a(reg);
	case 0xA0:
		return and_a(reg);
	case 0xA8:
		return xor_a(reg);
	case 0xB0:
		return or_a(reg);
	case 0xB8:
		return cp_a(reg);
	}


}

byte CPU::decode_block_3(byte instruction) {

	switch (instruction) {
	case 0xC6:
		return add_a(fetch());
	case 0xCE:
		return adc_a(fetch());
	case 0xD6:
		return sub_a(fetch());
	case 0xDE:
		return sbc_a(fetch());
	case 0xE6:
		return and_a(fetch());
	case 0xEE:
		return xor_a(fetch());
	case 0xF6:
		return or_a(fetch());
	case 0xFE:
		return cp_a(fetch());


	case 0xC9:
		return ret();
	case 0xD9:
		return ret_i();
	case 0xC3:
		return jp(fetch_16());
	case 0xE9:
		return jp_hl();
	case 0xCD:
		return call(fetch_16());


	case 0xCB:
		return decode_prefixed(fetch());


	case 0xE2:
		return ldh_c_a();
	case 0xE0:
		ld_to_mem_A(0xFF00|fetch());
		return 3;
	case 0xEA:
		ld_to_mem_A(fetch_16());
		return 4;
	case 0xF2:
		ldh_a_c();
		return 2;
	case 0xF0:
		ld_to_A_mem(0xFF00 | fetch());
		return 3;
	case 0xFA:
		ld_to_A_mem(fetch_16());
		return 4;



	case 0xE8:
		return add_SP(fetch());
	case 0xF8:
		return ld_to_hl_spe8(fetch());
	case 0xF9:
		return ld_sp_hl();


	case 0xF3:
		return di();
	case 0xFB:
		return ei();
	}

	// other ones in this section have some other data within instruction

	byte last_3_bits = instruction & 0x7;

	switch (last_3_bits) {
	case 0x0: {
		byte cond_bits = (instruction & 0x18) >> 3;
		return ret(decode_cond_bits(cond_bits));
	}

	case 0x2: {
		byte cond_bits = (instruction & 0x18) >> 3;
		return jp(fetch_16(), decode_cond_bits(cond_bits));
	}

	case 0x4: {
		byte cond_bits = (instruction & 0x18) >> 3;
		return call(fetch_16(), decode_cond_bits(cond_bits));
	}

	case 0x7: {
		byte target_bits = (instruction & 0x18) >> 3;
		return rst(target_bits << 3); //multiply by 8
	}

	case 0x1: {
		byte reg_stack_bits = (instruction & 0x30) >> 4;
		return pop(decode_reg16_stk_bits(reg_stack_bits));
	}

	case 0x5: {
		byte reg_stack_bits = (instruction & 0x30) >> 4;
		return push(decode_reg16_stk_bits(reg_stack_bits));
	}
	}


}

byte CPU::decode_prefixed(byte instruction) {

	byte decider_bits = instruction & 0xF8;
	byte reg8_bits = instruction & 0x7;

	switch (decider_bits) {
	case 0x0:
		return rlc(decode_reg8_bits(reg8_bits));
	case 0x08:
		return rrc(decode_reg8_bits(reg8_bits));
	case 0x10:
		return rl(decode_reg8_bits(reg8_bits));
	case 0x18:
		return rr(decode_reg8_bits(reg8_bits));
	case 0x20:
		return sla(decode_reg8_bits(reg8_bits));
	case 0x28:
		return sra(decode_reg8_bits(reg8_bits));
	case 0x30:
		return swap(decode_reg8_bits(reg8_bits));
	case 0x38:
		return srl(decode_reg8_bits(reg8_bits));
	}

	decider_bits = instruction & 0xC0;
	byte bit_index = (instruction & 0x38) >> 3;


	switch (decider_bits) {
	case 0x40: {
		Reg8 reg = decode_reg8_bits(reg8_bits);
		byte val = get_Reg8(reg);
		bit(bit_index, val);
		if (reg == HL_LOC) {
			return 3;
		}
		return 2;
	}

	case 0x80: {
		Reg8 reg = decode_reg8_bits(reg8_bits);
		res(bit_index, reg);
		if (reg == HL_LOC) {
			return 4;
		}
		return 2;
	}

	case 0xC0: {
		Reg8 reg = decode_reg8_bits(reg8_bits);
		set(bit_index, reg);
		if (reg == HL_LOC) {
			return 4;
		}
		return 2;
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
	case 7:
		return A;
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

byte CPU::ccf() {

	regs.set_n_flag(0);
	regs.set_h_flag(0);

	if (regs.get_c_flag() > 0) {
		regs.set_c_flag(0);
	}
	else {
		regs.set_c_flag(1);
	}

	return 1;

}


byte CPU::scf() {

	regs.set_n_flag(0);
	regs.set_h_flag(0);
	regs.set_c_flag(1);

	return 1;

}



//INTERUPTS 

byte CPU::interrupt_handler() {
	if (!ime) return 0;
	if (!interrupts.pending()) return 0;

	for (byte i = 0; i < 5; i++) {
		byte current_mask = 1 << i;
		if ((interrupts.IF & interrupts.IE & current_mask) != 0) {
			ime = false;
			interrupts.IF &= ~current_mask;

			regs.SP -= 2;
			bus.write_memory(regs.SP, regs.PC & 0xFF);
			bus.write_memory(regs.SP + 1, (regs.PC >> 8) & 0xFF);

			word vector_addresses[5] = { 0x40, 0x48, 0x50, 0x58, 0x60 };
			regs.PC = vector_addresses[i];

			return 20;

		}
	}

	return 0;


}

byte CPU::di() {
	ime = false;
	ime_pending = false;
	return 1;
}

byte CPU::ei() {
	ime_pending = true;
	return 1;
}

byte CPU::halt() {
	if (!ime && interrupts.pending()) {
		// bug = don't actually halt, PC won't advance properly
		// apparantly built in an tested in test roms
		halted_bug = true;
	}
	else {
		halted = true;
	}
	return 1;
}


// MISC INSTRUCTIONS
byte CPU::daa() {

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
		if (h || ((regs.regs_8b[A] & 0xF) > 0x9)) {
			adjustment += 0x6;
		}
		if (c || regs.regs_8b[A] > 0x99 ){
			adjustment += 0x60;
			regs.set_c_flag(1);
		}

		regs.regs_8b[A] += adjustment;


	}

	bool z = (regs.regs_8b[A] == 0);
	regs.set_z_flag(z);
	regs.set_h_flag(0);

	return 1;
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







