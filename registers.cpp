#include "registers.h"


word Registers::get_AF() {

	return (regs_8b[A] << 8) | regs_8b[F];
}

word Registers::get_BC() {

	return(regs_8b[B] << 8) | regs_8b[C];
}

word Registers::get_DE() {

	return(regs_8b[D] << 8) | regs_8b[E];
}

word Registers::get_HL() {

	return(regs_8b[H] << 8) | regs_8b[L];
}

word Registers::get_Reg16(Reg16 reg) {
	switch (reg) {

	case Reg16::AF:
		return get_AF();
		break;

	case Reg16::BC:
		return get_BC();
		break;

	case Reg16::DE:
		return get_DE();
		break;

	case Reg16::HL:
		return get_HL();
		break;

	case Reg16::SP:
		return SP;
		break;

	case Reg16::PC:
		return PC;
		break;
	}
}

void Registers::set_Reg16(Reg16 save_loc, word val) {
	switch (save_loc) {
	case Reg16::AF:
		set_AF(val);
		break;
	case Reg16::BC:
		set_BC(val);
		break;
	case Reg16::DE:
		set_DE(val);
		break;
	case Reg16::HL:
		set_HL(val);
		break;
	case Reg16::SP:
		SP = val;
		break;
	case Reg16::PC:
		PC = val;
		break;
	}
}


void Registers::set_AF(word val) {
	regs_8b[A] = (val & 0xFF00) >> 8;
	regs_8b[F] = val & 0x00FF;
}


void Registers::set_BC(word val) {
	regs_8b[B] = (val & 0xFF00) >> 8;
	regs_8b[C] = val & 0x00FF;
}


void Registers::set_DE(word val) {
	regs_8b[D] = (val & 0xFF00) >> 8;
	regs_8b[E] = val & 0x00FF;
}


void Registers::set_HL(word val) {
	regs_8b[H] = (val & 0xFF00) >> 8;
	regs_8b[L] = val & 0x00FF;
}


bool Registers::z_flag_set() {
	
	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b10000000) {

		return true;
	}
	return false;
}


bool Registers::n_flag_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b01000000) {

		return true;
	}
	return false;
}


bool Registers::h_flag_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b00100000) {

		return true;
	}
	return false;
}


bool Registers::c_flag_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b00010000) {

		return true;
	}
	return false;
}
