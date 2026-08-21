#pragma once
#include "types.h"


class Joypad {
public:
    bool up = false, down = false, left = false, right = false;
    bool a = false, b = false, select = false, start = false;

    byte select_bits = 0x30; // which group is selected, written by CPU (bits 4-5)

    byte read();
    void write(byte data);
};