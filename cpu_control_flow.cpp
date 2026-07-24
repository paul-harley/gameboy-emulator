#include "cpu.h"


// STACK MANIPULATION


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

void CPU::dec_sp() {
	regs.SP--;
}

void CPU::inc_sp() {
	regs.SP--;
}

void CPU::ld_sp(word val_to_load) {
	regs.SP = val_to_load;
}

void CPU::ld_to_mem_sp(word save_loc) {
	bus.write_memory(save_loc, regs.SP & 0xFF);
	bus.write_memory(save_loc + 1, regs.SP >> 8);

}

void CPU::ld_to_hl_spe8(sbyte val_to_add) {

	bool c = carry_add_16(regs.SP, val_to_add);
	bool h = half_carry_add_16(regs.SP, val_to_add);

	regs.SP += val_to_add;
	regs.set_HL(regs.SP);

	regs.set_flags(0, 0, h, c);
}


void CPU::ld_sp_hl() {
	regs.SP = regs.get_HL();
}



void CPU::pop(Reg16 save_loc) {

	byte low = bus.read_memory(regs.SP);
	inc_sp();
	byte high = bus.read_memory(regs.SP);
	inc_sp();

	word full_val = (high << 8) | low;

	regs.set_Reg16(save_loc, full_val);
}



void CPU::push(Reg16 data_loc) {
	word full_val = regs.get_Reg16(data_loc);
	push(full_val);
}


void CPU::push(word data) {

	byte high = ((data & 0xFF00) >> 8);
	byte low = data & 0xFF;

	dec_sp();
	bus.write_memory(regs.SP, high);
	dec_sp();
	bus.write_memory(regs.SP, low);
}



// JUMPS AND SUBROUTINES INSTRUCTIONS

void CPU::call(word jump_loc) {
	push(regs.PC);
	regs.PC = jump_loc;
}


void CPU::call(word jump_loc, Cond cond) {

	if (check_conds(cond)) {
		push(regs.PC);
		regs.PC = jump_loc;
	}
}

void CPU::jp_hl() {
	regs.PC = regs.get_HL();
}

void CPU::jp(word jump_loc) {
	regs.PC = jump_loc;
}

void CPU::jp(word jump_loc, Cond cond) {
	if (check_conds(cond)) {
		regs.PC = jump_loc;
	}
}


void CPU::jr(sbyte offset) {
	regs.PC += offset;
}

void CPU::jr(sbyte offset, Cond condition) {

	if (check_conds(condition)) {
		regs.PC += offset;
	}

}

void CPU::ret() {
	pop(PC);
}


void CPU::ret(Cond cond) {
	if (check_conds(cond)) {
		pop(PC);
	}
}

void CPU::ret_i() {
	ei();
	ret();
}

void CPU::rst(word vector) {
	call(vector);
}