#pragma once

#include <cstdint>

class Memory;

class PPU {
    public:
        Memory& memory;
        std::vector<uint32_t> framebuffer;
        void init();
        void update();
        PPU(Memory& memory);

    private:
        int totalCycles;
        bool interrupt;
        uint8_t& lcdc;
        uint8_t& stat;
        uint8_t& scy;
        uint8_t& scx;
        uint8_t& ly;
        uint8_t& lyc;
        uint8_t& bgp;
        uint8_t& obp0;
        uint8_t& obp1;
        uint8_t& wy;
        uint8_t& wx;

        void checkInterrupt(uint8_t mode);
        void updateScanLine();
        std::vector<uint32_t> getPalette(uint8_t palette);
        void drawBackground();
        void drawSprites();
        void drawWindow();
};
