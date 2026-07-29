#include "cpu.h"


void CPU::ld(Reg8 save_loc, Reg8 reg_to_copy) {
	set_Reg8(save_loc, get_Reg8(reg_to_copy));
}

void CPU::ld(Reg8 save_loc, byte val) {
	set_Reg8(save_loc, val);
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
	set_Reg8(save_loc, bus.read_memory(data_loc));
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

void CPU::ldh_c_a() {
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

void CPU::ldh_a_c() {
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
