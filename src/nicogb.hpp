#pragma once

#include "timer.hpp"
#include "cartridge.hpp"
#include "joypad.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "cpu.hpp"

class NicoGB {
    private:
        Timer timer;
        Cartridge cartridge;
        Joypad joypad;
        Memory memory;
        PPU ppu;
        CPU cpu;

        std::chrono::_V2::system_clock::time_point epoch;
        std::chrono::_V2::system_clock::time_point now();
        int64_t millis();
        int64_t last;

    public:
        bool speed;
        bool& loaded;
        std::string& title;
        std::vector<uint32_t>& framebuffer;

        void init();
        void load(std::string path);
        void tick();
        void keyDown(Key key);
        void keyUp(Key key);
        uint8_t serialDataRead();
        void serialDataWrite(uint8_t value);
        bool serialTransferRead();
        void serialTransferWrite(bool value);
        NicoGB();
};
