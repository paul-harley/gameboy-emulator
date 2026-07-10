#pragma once

#include <cstdint>

typedef uint8_t byte;
typedef uint16_t word;

class Registers {

private:
	byte A;
	byte F;

	byte B;
	byte C;

	byte D;
	byte E;

	byte H;
	byte L;

	// stack pointer
	word SP;
	word PC;


public:
	word getAF();
	word getBC();
	word getDE();
	word getHL();

};