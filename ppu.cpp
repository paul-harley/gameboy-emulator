#include "ppu.h"

void PPU::tick(int cycles)
{
    dot_counter += cycles;

    while (dot_counter >= 456)
    {
        dot_counter -= 456;

        ly++;

        if (ly == 144)
            interrupts.request(VBlank);
        else if (ly == 154)
            ly = 0;
    }
}

byte PPU::get_ly() {
    return ly;
}