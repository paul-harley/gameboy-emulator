#pragma once

#include "types.h"

enum Interrupt_Type {
    VBlank = 0, // bit 0, vector 0x40
    STAT = 1, // bit 1, vector 0x48
    Timer_i = 2, // bit 2, vector 0x50
    Serial = 3, // bit 3, vector 0x58
    Joypad = 4  // bit 4, vector 0x60

};

struct Interrupts {
                    // real address
    byte IE = 0xFF; // 0xFFFF
    byte IF = 0xFF; // 0xFF0F


    void request(Interrupt_Type type);
    bool pending();
};