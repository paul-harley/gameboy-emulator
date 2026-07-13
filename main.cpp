#include "bus.h"
#include "registers.h"
#include <iostream>

int main() {

	Bus mybus;
	Registers regTest;

	byte F_TEST = 0xAF; 




	std::cout << std::hex<< static_cast<int>(F_TEST >> 4) << std::endl;

	return 0;
}