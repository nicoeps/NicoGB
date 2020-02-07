#include <string>
#include <cstdint>
#include <fstream>

#include "cartridge.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

void PPU::update() {
    totalCycles += cpu.cycles;

    stat = cpu.memory.read(statAddr);
    ly = cpu.memory.read(lyAddr);

    if (ly >= 144 && totalCycles >= 114) { // V-Blank
        stat = (stat & 0xFC) | 0x01; 
        if (ly == 144) {
            drawBackground();
            drawWindow();
            drawSprites();
            cpu.memory.interrupt(0x01);
        }
        updateScanLine();
    } else {
        if (totalCycles < 20) { // OAM Search
            stat = (stat & 0xFC) | 0x02; 
        } else if (totalCycles < 63) { // Pixel Transfer
            stat = (stat & 0xFC) | 0x03; 
        } else if (totalCycles < 114) { // H-Blank
            stat = (stat & 0xFC) | 0x00; 
        } else if (totalCycles >= 114) { // End of Line
            stat = (stat & 0xFC) | 0x02;
            updateScanLine();
        }
    }
    cpu.memory.write(statAddr, stat);
}

void PPU::updateScanLine() {
    ++ly;
    if (ly > 153) {
        ly = 0;
    }

    cpu.memory.write(lyAddr, ly);
    
    // LYC = LY
    if (cpu.memory.read(0xFF45) == ly) {
        cpu.memory.interrupt(0x02);
    }

    totalCycles = 0;
}

uint32_t PALETTE[4][4] = {
    {0xfffff6d3, 0xfff9a875, 0xffeb6b6f, 0xff7c3f58},
    {0xffffefff, 0xfff7b58c, 0xff84739c, 0xff181010},
    {0xffcecece, 0xff6f9edf, 0xff42678e, 0xff102533},
    {0xffe0f8d0, 0xff88c070, 0xff346856, 0xff081820},
};

int paletteNr = 0;

void getPalette(uint32_t * col, uint8_t palette) {
    for (int i = 0; i <= 6; i += 2) {
        switch ((palette >> i) & 0x3) {
            case 0:
                col[i/2] = PALETTE[paletteNr][0];
                break;
            case 1:
                col[i/2] = PALETTE[paletteNr][1];
                break;
            case 2:
                col[i/2] = PALETTE[paletteNr][2];
                break;
            case 3:
                col[i/2] = PALETTE[paletteNr][3];
                break;
        }
    }
}

void PPU::drawWindow() {
    uint8_t LCDC = cpu.memory.read(0xFF40);
    uint8_t windowY = cpu.memory.read(0xFF4A);
    uint8_t windowX = cpu.memory.read(0xFF4B);

    if ((LCDC & 0x20) && windowX <= 166 && windowY <= 143) { // Window Enabled
        uint16_t tileSelect = (((LCDC & 0x40) >> 6)) ? 0x9C00 : 0x9800;
        uint16_t tileData = ((((LCDC & 0x10) >> 4)) ? 0x8000 : 0x9000);

        uint8_t palette = cpu.memory.read(0xFF47);
        uint32_t col[4] = {};
        getPalette(col, palette);

        for (int i = 0; i < 18; ++i) {
            for (int j = 0; j < 20; ++j) {
                uint8_t tileNumber = cpu.memory.read(tileSelect + (i*32) + j);
                int screenY = i * 8;
                int screenX = j * 8;

                if (screenX < 160 && screenY < 144) {
                    for (int ii = 0; ii < 8; ++ii) { // Y
                        uint8_t byte1 = 0;
                        uint8_t byte2 = 0;
                        if (tileData == 0x8000) {
                            byte1 = cpu.memory.read(tileData + (tileNumber*16) + (ii*2));
                            byte2 = cpu.memory.read(tileData + (tileNumber*16) + (ii*2)+1);
                        } else {
                            byte1 = cpu.memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2));
                            byte2 = cpu.memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2)+1);  
                        }

                        for (int jj = 0; jj < 8; ++jj) { // X
                            uint8_t bit1 = (byte1 >> (7-jj)) & 0x1;
                            uint8_t bit2 = (byte2 >> (7-jj)) & 0x1;
                            uint8_t color = (bit2 << 1) | bit1;
                            uint32_t pixel = col[color];

                            int Y = screenY+ii+windowY;
                            int X = screenX+jj+windowX-7;
                            if (Y >= 0 && X >= 0 && Y < 144 && X < 160) {
                                framebuffer[Y*160 + X] = pixel;
                            }
                        }
                    }
                }
            }
        }
    }
}

void PPU::drawSprites() {
    uint8_t LCDC = cpu.memory.read(0xFF40);
    int SIZE = ((LCDC & 0x4) >> 2) ? 16 : 8;
    uint16_t OAM = 0xFE00;
    if (LCDC & 0x2 || true) {
        for (uint8_t byte = 0; byte <= 0x9F; byte += 4) {
            int Y = cpu.memory.read(OAM+byte)-16;
            int X = cpu.memory.read(OAM+byte+1)-8;
            uint8_t tileNumber = cpu.memory.read(OAM+byte+2);
            if (SIZE == 16) {
                tileNumber &= ~1;
            }
            uint8_t attributes = cpu.memory.read(OAM+byte+3);
            uint16_t paletteNumber = (attributes & 0x10) ? 0xFF49 : 0xFF48;
            uint8_t xFlip = attributes & 0x20;
            uint8_t yFlip = attributes & 0x40;

            uint8_t palette = cpu.memory.read(paletteNumber);
            uint32_t col[4] = {};
            getPalette(col, palette);

            for (int i = 0; i < SIZE; ++i) {
                uint8_t byte1 = cpu.memory.read(0x8000 + (tileNumber*16) + i*2);
                uint8_t byte2 = cpu.memory.read(0x8000 + (tileNumber*16) + i*2+1);
                if (yFlip) {
                    byte1 = cpu.memory.read(0x8000 + (tileNumber*16) + (SIZE-1-i)*2);
                    byte2 = cpu.memory.read(0x8000 + (tileNumber*16) + (SIZE-1-i)*2+1);
                }
                for (int j = 0; j < 8; ++j) {
                    if ((Y+i) < 144 && (X+j) < 160 && (Y+i) >= 0 && (X+j) >= 0) {
                        uint8_t bit1 = (byte1 >> (7-j)) & 0x1;
                        uint8_t bit2 = (byte2 >> (7-j)) & 0x1;
                        if (xFlip) {
                            bit1 = (byte1 >> j) & 0x1;
                            bit2 = (byte2 >> j) & 0x1;
                        }
                        uint8_t color = (bit2 << 1) | bit1;
                        uint32_t pixel = col[color];

                        if (color != 0) {
                            framebuffer[(Y+i)*160 + (X+j)] = pixel;
                        }
                    }
                }
            }
        }
    }
}

void PPU::drawBackground() {
    uint8_t LCDC = cpu.memory.read(0xFF40);
    uint16_t tileSelect = (((LCDC & 0x8) >> 3)) ? 0x9C00 : 0x9800;
    uint16_t tileData = ((((LCDC & 0x10) >> 4)) ? 0x8000 : 0x9000);

    uint8_t scrollY = cpu.memory.read(0xFF42)/8;
    uint8_t scrollX = cpu.memory.read(0xFF43)/8;

    uint8_t palette = cpu.memory.read(0xFF47);
    uint32_t col[4] = {};
    getPalette(col, palette);

    for (int i = 0; i < 19; ++i) {
        for (int j = 0; j < 21; ++j) {
            uint8_t tileNumber = cpu.memory.read(tileSelect + (((i+scrollY) % 32)*32) + ((j+scrollX) % 32));

            int screenY = i * 8;
            int screenX = j * 8;

            if (screenX < 168 && screenY < 152) {
                for (int ii = 0; ii < 8; ++ii) { // Y
                    uint8_t byte1 = 0;
                    uint8_t byte2 = 0;
                    if (tileData == 0x8000) {
                        byte1 = cpu.memory.read(tileData + (tileNumber*16) + (ii*2));
                        byte2 = cpu.memory.read(tileData + (tileNumber*16) + (ii*2)+1);
                    } else {
                        byte1 = cpu.memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2));
                        byte2 = cpu.memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2)+1);  
                    }

                    for (int jj = 0; jj < 8; ++jj) { // X
                        uint8_t bit1 = (byte1 >> (7-jj)) & 0x1;
                        uint8_t bit2 = (byte2 >> (7-jj)) & 0x1;
                        uint8_t color = (bit2 << 1) | bit1;
                        uint32_t pixel = col[color];

                        int Y = screenY+ii-(cpu.memory.read(0xFF42) % 8);
                        int X = screenX+jj-(cpu.memory.read(0xFF43) % 8);
                        if (Y >= 0 && X >= 0 && Y < 144 && X < 160) {
                            framebuffer[Y*160 + X] = pixel;
                        }
                    }
                }
            }
        }
    }
}
