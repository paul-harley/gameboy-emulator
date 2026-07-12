#include "cpu.h"


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
	uint16_t save_loc = regs.get_HL();
	bus.write_memory(save_loc, regs.regs_8b[val_loc]);
}


void CPU::ld_to_HL_loc(byte val) {
	uint16_t save_loc = regs.get_HL();
	bus.write_memory(save_loc, val);
}

void CPU::ld_to_reg_HL(Reg8 save_loc) {
	uint16_t data_loc = regs.get_HL();
	regs.regs_8b[save_loc] = bus.read_memory(data_loc);
}

void CPU::ld_to_mem_A(Reg16 save_loc) {
	uint8_t data = regs.regs_8b[A];
	uint16_t save_loc = regs.get_Reg16(save_loc);
	bus.write_memory(save_loc, data);
}


