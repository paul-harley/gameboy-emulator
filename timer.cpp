#include "timer.h"
#include <iostream>

/*
void Timer::tick(byte cycles) {

	div_counter += cycles;

	if (div_counter >= 256) {
		div_counter -= 256;
		DIV++;
	}

	word inc_period = get_tac_period();
	if (inc_period == 0) return;


	tima_counter += cycles;

	if (tima_counter >= inc_period) {
		tima_counter -= inc_period;
		TIMA++;

		if (TIMA > 0xFF) {
			TIMA = TMA;
			interrupts.request(Timer_i);
		}
	}

}
*/

void Timer::tick(byte cycles) {
	for (int i = 0; i < cycles; i++) { 

		// handle a pending overflow from last cycle, BEFORE anything else this cycle
		if (overflow_pending) {
			overflow_pending = false;
			TIMA = TMA;
			interrupts.request(Timer_i);
		}

		word old_counter = system_counter;
		system_counter++;
		DIV = system_counter >> 8;

		check_falling_edge(old_counter);
	}
}

void Timer::check_falling_edge(word old_counter) {
	byte bit_index = get_tac_bit(); // which bit TAC selects (varies by TAC.freq)
	bool enabled = TAC & 0x4;

	bool old_bit = (old_counter >> bit_index) & 1;
	bool new_bit = (system_counter >> bit_index) & 1;

	if (enabled && old_bit && !new_bit) {
		increment_tima();
	}
}

void Timer::increment_tima() {
	if (TIMA == 0xFF) {
		TIMA = 0x00;           // sits at 0 for exactly one cycle, as the docs describe
		overflow_pending = true; // reload + interrupt happens next tick, not now
	}
	else {
		TIMA++;
	}
}

byte Timer::get_tac_bit() {
	switch (TAC & 0x3) {
	case 0: 
		return 9;
	case 1: 
		return 3;
	case 2: 
		return 5;
	case 3: 
		return 7;
	}
	return 9;
}

void Timer::reset_sys_counter() {
	system_counter = 0;
}