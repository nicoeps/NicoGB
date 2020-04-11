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
        uint8_t& lcdc = memory.lcd.lcdc;
        uint8_t& stat = memory.lcd.stat;
        uint8_t& scy = memory.lcd.scy;
        uint8_t& scx = memory.lcd.scx;
        uint8_t& ly = memory.lcd.ly;
        uint8_t& lyc = memory.lcd.lyc;
        uint8_t& bgp = memory.lcd.bgp;
        uint8_t& obp0 = memory.lcd.obp0;
        uint8_t& obp1 = memory.lcd.obp1;
        uint8_t& wy = memory.lcd.wy;
        uint8_t& wx = memory.lcd.wx;

        void checkInterrupt(uint8_t mode);
        void updateScanLine();
        std::vector<uint32_t> getPalette(uint8_t palette);
        void drawBackground();
        void drawSprites();
        void drawWindow();
};
