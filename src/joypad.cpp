#include "joypad.hpp"

void Joypad::reset() {
    keys[0] = 0x0F;
    keys[1] = 0x0F;
    mode = 0;
}

uint8_t Joypad::read() {
    switch (mode) {
        case 0x10: return keys[0];
        case 0x20: return keys[1];
        default: return 0xCF;
    }
}

void Joypad::write(uint8_t n) {
    mode = n & 0x30;
}

void Joypad::keyDown(SDL_Keysym key) {
    switch (key.sym) {
        case SDLK_RIGHT:  keys[1] &= 0xE; interrupt = true; break;
        case SDLK_LEFT:   keys[1] &= 0xD; interrupt = true; break;
        case SDLK_UP:     keys[1] &= 0xB; interrupt = true; break;
        case SDLK_DOWN:   keys[1] &= 0x7; interrupt = true; break;
        case SDLK_z:      keys[0] &= 0xE; interrupt = true; break;
        case SDLK_x:      keys[0] &= 0xD; interrupt = true; break;
        case SDLK_RSHIFT: keys[0] &= 0xB; interrupt = true; break;
        case SDLK_RETURN: keys[0] &= 0x7; interrupt = true; break;
    }
}

void Joypad::keyUp(SDL_Keysym key) {
    switch (key.sym) {
        case SDLK_RIGHT:  keys[1] |= 0x1; break;
        case SDLK_LEFT:   keys[1] |= 0x2; break;
        case SDLK_UP:     keys[1] |= 0x4; break;
        case SDLK_DOWN:   keys[1] |= 0x8; break;
        case SDLK_z:      keys[0] |= 0x1; break;
        case SDLK_x:      keys[0] |= 0x2; break;
        case SDLK_RSHIFT: keys[0] |= 0x4; break;
        case SDLK_RETURN: keys[0] |= 0x8; break;
    } 
}
