#include "timer.hpp"

Timer::Timer() {
    init();
}

void Timer::init() {
    counter = 0;
    divider = 0;
    tima = 0;
    tma = 0;
    tac = 0;
    reload = 0;
    oldEdge = 0;
}

bool Timer::currentEdge() {
    const int freq[4] = {9, 3, 5, 7};
    return (counter & (1 << freq[tac & 0x3])) && (tac & 0x4) != 0;
}

bool Timer::fallingEdge() {
    return !currentEdge() && oldEdge;
}
