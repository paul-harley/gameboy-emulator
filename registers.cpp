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
