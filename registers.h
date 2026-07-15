#pragma once

#include <cstdint>
#include <array>

typedef uint8_t byte;
typedef uint16_t word;

typedef int8_t sbyte;
typedef int16_t sword;


enum Reg8
{
	A,
	F,
	B,
	C,
	D,
	E,
	H,
	L,
	HL_LOC
};

enum Reg16
{
	AF,
	BC,
	DE,
	HL,
	SP,
	PC
};




class Registers {

public:

	std::array<byte, 8> regs_8b;

	// stack pointer
	word SP;
	word PC;

	// get larger combined registers
	word get_AF();
	word get_BC();
	word get_DE();
	word get_HL();


	// set larger combined registers
	void set_AF(word val);
	void set_BC(word val);
	void set_DE(word val);
	void set_HL(word val);

	//more general if unknown which one to use
	word get_Reg16(Reg16 reg);
	void set_Reg16(Reg16 save_loc, word val);


	// all these in F register
	// F = znhc0000
	bool z_flag_is_set();
	bool n_flag_is_set();
	bool h_flag_is_set();
	bool c_flag_is_set();

	void set_z_flag(byte new_val);
	void set_n_flag(byte new_val);
	void set_h_flag(byte new_val);
	void set_c_flag(byte new_val);

	void set_flags(byte z, byte n, byte h, byte c);

	byte get_z_flag();
	byte get_n_flag();
	byte get_h_flag();
	byte get_c_flag();

};