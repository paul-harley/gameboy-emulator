#pragma once
#include "types.h"
#include "interrupts.h"


class Timer {

public:
	byte DIV = 0; //FF04
	word TIMA = 0; //FF05 word makes it easier to find overflow
	byte TMA = 0; //FF06
	byte TAC = 0; //FF07


	Timer(Interrupts& interrupts): interrupts(interrupts) {};
	Interrupts& interrupts;

	void tick(byte cycles);
	void reset_sys_counter();

private:
	bool overflow_pending = false;

	word system_counter = 0;
	word get_tac_period();
	byte get_tac_bit();
	void check_falling_edge(word old_counter);
	void increment_tima();
};