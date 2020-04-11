#pragma once

#include <cstdint>

class Timer {
    public:
        union {
            struct {
                uint8_t low;
                uint8_t divider;
            };
            struct {
                uint16_t counter;
            };
        };
        uint8_t tima;
        uint8_t tma;
        uint8_t tac;
        uint8_t reload;
        bool oldEdge;
        bool currentEdge();
        bool fallingEdge();
        void init();
        Timer();
};
