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


void Interrupts::serialize(std::ofstream& out) {
	out.write(reinterpret_cast<char*>(&IE), sizeof(IE));
	out.write(reinterpret_cast<char*>(&IF), sizeof(IF));
}

void Interrupts::deserialize(std::ifstream& in) {
	in.read(reinterpret_cast<char*>(&IE), sizeof(IE));
	in.read(reinterpret_cast<char*>(&IF), sizeof(IF));
}