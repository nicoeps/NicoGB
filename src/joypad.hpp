#pragma once

#include <cstdint>

enum Key {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    A,
    B,
    SELECT,
    START,
    NONE
};

class Joypad {
    private:
        uint8_t keys[2];
        uint8_t mode;

    public:
        bool interrupt;
        void reset();
        uint8_t read();
        void write(uint8_t n);
        void keyDown(Key key);
        void keyUp(Key key);
        Joypad();
};
