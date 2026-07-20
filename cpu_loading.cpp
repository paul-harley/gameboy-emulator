#include "cpu.h"


void CPU::ld(Reg8 save_loc, Reg8 reg_to_copy) {
	if (save_loc == HL_LOC && reg_to_copy == HL_LOC) {
		halt();
	}

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

void CPU::ld_to_mem_A(Reg16 save_address_loc) {
	byte data = regs.regs_8b[A];
	word save_address = regs.get_Reg16(save_address_loc);
	bus.write_memory(save_address, data);

	if (save_address_loc == HLI) {
		regs.set_HL(regs.get_HL() + 1);
	}
	else if (save_address_loc == HLD) {
		regs.set_HL(regs.get_HL() - 1);
	}
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

	if (val_loc == HLI) {
		regs.set_HL(regs.get_HL() + 1);
	}
	else if (val_loc == HLD) {
		regs.set_HL(regs.get_HL() - 1);
	}
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