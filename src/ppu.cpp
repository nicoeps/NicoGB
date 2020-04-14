#include "memory.hpp"
#include "ppu.hpp"

PPU::PPU(Memory& memory) :
    memory(memory),
    lcdc(memory.lcd.lcdc),
    stat(memory.lcd.stat),
    scy(memory.lcd.scy),
    scx(memory.lcd.scx),
    ly(memory.lcd.ly),
    lyc(memory.lcd.lyc),
    bgp(memory.lcd.bgp),
    obp0(memory.lcd.obp0),
    obp1(memory.lcd.obp1),
    wy(memory.lcd.wy),
    wx(memory.lcd.wx) {
    framebuffer = std::vector<uint32_t>(160*144);
    init();
}

void PPU::init() {
    std::fill_n(framebuffer.begin(), 160*144, 0);
    totalCycles = 0;
    interrupt = false;
}

void PPU::update() {
    if ((lcdc & 0x80) == 0) {
        totalCycles = 0;
        ly = 0;
        stat = (stat & 0xFC) | 0x00;
        stat = (lyc == ly) ? (stat | 0x4) : (stat & ~0x4);
        return;
    }

    totalCycles++;

    if (ly >= 144 && totalCycles >= 114) { // V-Blank
        stat = (stat & 0xFC) | 0x01;
        if (ly == 144) {
            drawBackground();
            drawWindow();
            drawSprites();
            memory.interrupt(0x1);
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

    stat = (lyc == ly) ? (stat | 0x4) : (stat & ~0x4);
    uint8_t mode = stat & 0x3;
    if (((stat & 0x4) && (stat & 0x40))
    || (((stat >> (mode+3)) & 1) && mode != 0x3)) {
        if (interrupt == false) {
            memory.interrupt(0x2);
            interrupt = true;
        }
    } else {
        interrupt = false;
    }
}

void PPU::updateScanLine() {
    ++ly;
    if (ly > 153) {
        ly = 0;
    }

    totalCycles = 0;
}

std::vector<uint32_t> PPU::getPalette(uint8_t palette) {
    const std::vector<uint32_t> PALETTE = {0xfffff6d3, 0xfff9a875, 0xffeb6b6f, 0xff7c3f58};
    std::vector<uint32_t> col = {0, 0, 0, 0};
    for (int i = 0; i <= 6; i += 2) {
        switch ((palette >> i) & 0x3) {
            case 0:
                col[i/2] = PALETTE[0];
                break;
            case 1:
                col[i/2] = PALETTE[1];
                break;
            case 2:
                col[i/2] = PALETTE[2];
                break;
            case 3:
                col[i/2] = PALETTE[3];
                break;
        }
    }
    return col;
}

void PPU::drawWindow() {
    if ((lcdc & 0x20) && wx <= 166 && wy <= 143) { // Window Enabled
        uint16_t tileSelect = (((lcdc & 0x40) >> 6)) ? 0x9C00 : 0x9800;
        uint16_t tileData = ((((lcdc & 0x10) >> 4)) ? 0x8000 : 0x9000);
        std::vector<uint32_t> col = getPalette(bgp);

        for (int i = 0; i < 18; ++i) {
            for (int j = 0; j < 20; ++j) {
                uint8_t tileNumber = memory.read(tileSelect + (i*32) + j);
                int screenY = i * 8;
                int screenX = j * 8;

                if (screenX < 160 && screenY < 144) {
                    for (int ii = 0; ii < 8; ++ii) { // Y
                        uint8_t byte1 = 0;
                        uint8_t byte2 = 0;
                        if (tileData == 0x8000) {
                            byte1 = memory.read(tileData + (tileNumber*16) + (ii*2));
                            byte2 = memory.read(tileData + (tileNumber*16) + (ii*2)+1);
                        } else {
                            byte1 = memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2));
                            byte2 = memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2)+1);
                        }

                        for (int jj = 0; jj < 8; ++jj) { // X
                            uint8_t bit1 = (byte1 >> (7-jj)) & 0x1;
                            uint8_t bit2 = (byte2 >> (7-jj)) & 0x1;
                            uint8_t color = (bit2 << 1) | bit1;
                            uint32_t pixel = col[color];

                            int Y = screenY+ii+wy;
                            int X = screenX+jj+wx-7;
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
    int SIZE = ((lcdc & 0x4) >> 2) ? 16 : 8;
    const uint16_t OAM = 0xFE00;
    if (lcdc & 0x2 || true) {
        for (uint8_t byte = 0; byte <= 0x9F; byte += 4) {
            int Y = memory.read(OAM+byte)-16;
            int X = memory.read(OAM+byte+1)-8;
            uint8_t tileNumber = memory.read(OAM+byte+2);
            if (SIZE == 16) {
                tileNumber &= ~1;
            }
            uint8_t attributes = memory.read(OAM+byte+3);
            uint8_t xFlip = attributes & 0x20;
            uint8_t yFlip = attributes & 0x40;
            std::vector<uint32_t> col = getPalette((attributes & 0x10) ? obp1 : obp0);

            for (int i = 0; i < SIZE; ++i) {
                uint8_t byte1 = memory.read(0x8000 + (tileNumber*16) + i*2);
                uint8_t byte2 = memory.read(0x8000 + (tileNumber*16) + i*2+1);
                if (yFlip) {
                    byte1 = memory.read(0x8000 + (tileNumber*16) + (SIZE-1-i)*2);
                    byte2 = memory.read(0x8000 + (tileNumber*16) + (SIZE-1-i)*2+1);
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
    uint16_t tileSelect = ((lcdc & 0x8) >> 3) ? 0x9C00 : 0x9800;
    uint16_t tileData = ((lcdc & 0x10) >> 4) ? 0x8000 : 0x9000;

    uint8_t scrollY = scy / 8;
    uint8_t scrollX = scx / 8;
    std::vector<uint32_t> col = getPalette(bgp);

    for (int i = 0; i < 19; ++i) {
        for (int j = 0; j < 21; ++j) {
            uint8_t tileNumber = memory.read(tileSelect + (((i+scrollY) % 32)*32) + ((j+scrollX) % 32));

            int screenY = i * 8;
            int screenX = j * 8;

            if (screenX < 168 && screenY < 152) {
                for (int ii = 0; ii < 8; ++ii) { // Y
                    uint8_t byte1 = 0;
                    uint8_t byte2 = 0;
                    if (tileData == 0x8000) {
                        byte1 = memory.read(tileData + (tileNumber*16) + (ii*2));
                        byte2 = memory.read(tileData + (tileNumber*16) + (ii*2)+1);
                    } else {
                        byte1 = memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2));
                        byte2 = memory.read(tileData + ((int8_t) (tileNumber)*16) + (ii*2)+1);
                    }

                    for (int jj = 0; jj < 8; ++jj) { // X
                        uint8_t bit1 = (byte1 >> (7-jj)) & 0x1;
                        uint8_t bit2 = (byte2 >> (7-jj)) & 0x1;
                        uint8_t color = (bit2 << 1) | bit1;
                        uint32_t pixel = col[color];

                        int Y = screenY+ii-(scy % 8);
                        int X = screenX+jj-(scx % 8);
                        if (Y >= 0 && X >= 0 && Y < 144 && X < 160) {
                            framebuffer[Y*160 + X] = pixel;
                        }
                    }
                }
            }
        }
    }
}
