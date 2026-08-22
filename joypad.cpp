#include "joypad.h"
#include <iostream>

byte Joypad::read() {

    byte result = select_bits | 0x0F; // all buttons released

    // dpad
    if (!(select_bits & 0x10)) { 
        if (down)  result &= ~0x08;
        if (up)    result &= ~0x04;
        if (left)  result &= ~0x02;
        if (right) result &= ~0x01;
    }

    // buttons 
    if (!(select_bits & 0x20)) { 
        if (start)  result &= ~0x08;
        if (select) result &= ~0x04;
        if (b)      result &= ~0x02;
        if (a)      result &= ~0x01;
    }

    return result;
}

void Joypad::write(byte data) {
    select_bits = data & 0x30;
}