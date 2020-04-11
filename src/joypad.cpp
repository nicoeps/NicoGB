#include "joypad.hpp"

Joypad::Joypad() {
    reset();
}

void Joypad::reset() {
    interrupt = false;
    keys[0] = 0x0F;
    keys[1] = 0x0F;
    mode = 0;
}

uint8_t Joypad::read() {
    switch (mode) {
        case 0x00: return 0xC0 | mode | (keys[0] & keys[1]);
        case 0x10: return 0xC0 | mode | keys[0];
        case 0x20: return 0xC0 | mode | keys[1];
        default: return 0xCF;
    }
}

void Joypad::write(uint8_t n) {
    mode = n & 0x30;
}

void Joypad::keyDown(Key key) {
    switch (key) {
        case RIGHT:  keys[1] |= 0x2; keys[1] &= 0xE; interrupt = true; break;
        case LEFT:   keys[1] |= 0x1; keys[1] &= 0xD; interrupt = true; break;
        case UP:     keys[1] |= 0x8; keys[1] &= 0xB; interrupt = true; break;
        case DOWN:   keys[1] |= 0x4; keys[1] &= 0x7; interrupt = true; break;
        case A:      keys[0] &= 0xE; interrupt = true; break;
        case B:      keys[0] &= 0xD; interrupt = true; break;
        case SELECT: keys[0] &= 0xB; interrupt = true; break;
        case START:  keys[0] &= 0x7; interrupt = true; break;
        default: break;
    }
}

void Joypad::keyUp(Key key) {
    switch (key) {
        case RIGHT:  keys[1] |= 0x1; break;
        case LEFT:   keys[1] |= 0x2; break;
        case UP:     keys[1] |= 0x4; break;
        case DOWN:   keys[1] |= 0x8; break;
        case A:      keys[0] |= 0x1; break;
        case B:      keys[0] |= 0x2; break;
        case SELECT: keys[0] |= 0x4; break;
        case START:  keys[0] |= 0x8; break;
        default: break;
    } 
}
