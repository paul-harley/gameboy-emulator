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


bool Registers::z_flag_is_set() {
	
	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b10000000) {

		return true;
	}
	return false;
}


bool Registers::n_flag_is_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b01000000) {

		return true;
	}
	return false;
}


bool Registers::h_flag_is_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b00100000) {

		return true;
	}
	return false;
}


bool Registers::c_flag_is_set() {

	word AF = get_AF();
	byte flags = AF & 0xFF;

	if (flags & 0b00010000) {

		return true;
	}
	return false;
}


void Registers::set_z_flag(byte new_val) {
	byte z;
	if (new_val > 0) {
		z = 1;
	}
	else {
		z = 0;
	}

	byte full_data = regs_8b[F];
	byte current_flags = full_data >> 4;

	byte new_flags = current_flags & 0b0111;
	new_flags += (8 * z);
	new_flags = new_flags << 4;

	regs_8b[F] = new_flags;
}


void Registers::set_n_flag(byte new_val) {

	byte n;
	if (new_val > 0) {
		n = 1;
	}
	else {
		n = 0;
	}

	byte full_data = regs_8b[F];
	byte current_flags = full_data >> 4;

	byte new_flags = current_flags & 0b1011;
	new_flags += (4 * n);
	new_flags = new_flags << 4;

	regs_8b[F] = new_flags;

}
void Registers::set_h_flag(byte new_val) {

	byte h;
	if (new_val > 0) {
		h = 1;
	}
	else {
		h = 0;
	}

	byte full_data = regs_8b[F];
	byte current_flags = full_data >> 4;

	byte new_flags = current_flags & 0b1011;
	new_flags += (2 * h);
	new_flags = new_flags << 4;

	regs_8b[F] = new_flags;

}
void Registers::set_c_flag(byte new_val) {

	byte c;
	if (new_val > 0) {
		c = 1;
	}
	else {
		c = 0;
	}

	byte full_data = regs_8b[F];
	byte current_flags = full_data >> 4;

	byte new_flags = current_flags & 0b1011;
	new_flags += (1 * c);
	new_flags = new_flags << 4;

	regs_8b[F] = new_flags;

}
