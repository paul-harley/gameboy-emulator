#pragma once
#include "registers.h"

class PPU {
public:
    void tick(int cycles);

    byte get_ly();

private:
    byte ly = 0;
    int dot_counter = 0;
};