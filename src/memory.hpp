#pragma once

#include <vector>
#include <string>

class Cartridge;
class Joypad;
class Timer;

class Memory {
    public:
        Cartridge& cartridge;
        Joypad& joypad;
        Timer& timer;

        struct {
            uint8_t lcdc;
            uint8_t stat;
            uint8_t scy;
            uint8_t scx;
            uint8_t ly;
            uint8_t lyc;
            uint8_t bgp;
            uint8_t obp0;
            uint8_t obp1;
            uint8_t wy;
            uint8_t wx;
        } lcd;

        bool bootEnabled;
        void init();
        void load(std::string path);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        uint8_t readInternal(uint16_t address);
        void writeInternal(uint16_t address, uint8_t n);
        void interrupt(uint8_t IRQ);

        uint16_t dmaAddress;
        uint8_t dmaCycle;
        void transfer();

        Memory(Cartridge& cartridge, Joypad& joypad, Timer& timer);

    private:
        std::vector<uint8_t> vram;
        std::vector<uint8_t> wram;
        std::vector<uint8_t> oam;
        std::vector<uint8_t> io;
        std::vector<uint8_t> hram;
        std::vector<uint8_t> boot;
};
