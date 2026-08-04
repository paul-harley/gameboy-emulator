#include "interrupts.h"

void Interrupts::request(Interrupt_Type type) {
	IF |= (1 << type);
}

bool Interrupts::pending() {
	if ((IF & IE) == 0) {
		return false;
	}
	return true;
}