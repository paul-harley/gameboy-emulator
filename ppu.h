#pragma once
#include "interrupts.h"

class PPU {
public:
    PPU(Interrupts& interrupts) : interrupts(interrupts) {}

    void tick(int cycles);

    byte get_ly();
    Interrupts& interrupts;


private:
    byte ly = 0;
    int dot_counter = 0;
};