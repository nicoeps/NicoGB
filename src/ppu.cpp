#include <tuple>
#include <algorithm>

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
    writebuffer = std::vector<uint32_t>(160*144);
    line = std::vector<uint8_t>(160);
    init();
}

void PPU::init() {
    std::fill_n(framebuffer.begin(), 160*144, 0);
    std::fill_n(writebuffer.begin(), 160*144, 0);
    std::fill_n(line.begin(), 160, 0);
    mode = 4;
    totalCycles = 0;
    interrupt = false;
    windowCounter = 0;
    clear = true;
}

void PPU::update() {
    if ((lcdc & 0x80) == 0) {
        totalCycles = 0;
        ly = 0;
        windowCounter = 0;
        stat = (stat & 0xFC) | 0x00;
        stat = (lyc == ly) ? (stat | 0x4) : (stat & ~0x4);
        if (clear) {
            clear = false;
            std::fill_n(writebuffer.begin(), 160*144, getPalette(0)[0]);
            std::fill_n(framebuffer.begin(), 160*144, getPalette(0)[0]);
        }
        return;
    }
    clear = true;

    totalCycles++;

    switch (mode) {
        case 2: // OAM Search
            if (totalCycles >= 20) {
                totalCycles -= 20;
                mode = 3;
                stat = (stat & 0xFC) | 0x03;
            }
            break;

        case 3: // Pixel transfer
            if (totalCycles >= 43) {
                totalCycles -= 43;
                mode = 0;
                stat = (stat & 0xFC) | 0x00;
                std::fill_n(line.begin(), 160, 0);
                drawBackground();
                drawWindow();
                drawSprites();
            }
            break;

        case 0: // H-Blank
            if (totalCycles >= 51) {
                totalCycles -= 51;
                ++ly;
                if (ly < 144) {
                    mode = 2;
                    stat = (stat & 0xFC) | 0x02;
                } else {
                    mode = 1;
                    stat = (stat & 0xFC) | 0x01;
                    framebuffer.swap(writebuffer);
                    memory.interrupt(0x1);
                }
            }
            break;

        case 1: // V-Blank
            if (totalCycles >= 114) {
                totalCycles -= 114;
                ++ly;
                if (ly > 153) {
                    ly = 0;
                    windowCounter = 0;
                    mode = 2;
                    stat = (stat & 0xFC) | 0x02;
                }
            }
            break;

        default: // Glitched OAM
            if (totalCycles >= 19) {
                totalCycles -= 19;
                mode = 3;
                stat = (stat & 0xFC) | 0x03;
            }
            break;
    }

    stat = (lyc == ly) ? (stat | 0x4) : (stat & ~0x4);
    if (((stat & 0x4) && (stat & 0x40))
    || (((stat >> (mode + 3)) & 1) && mode != 0x3)) {
        if (interrupt == false) {
            memory.interrupt(0x2);
            interrupt = true;
        }
    } else {
        interrupt = false;
    }
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

void PPU::drawBackground() {
    if (!(lcdc & 0x1)) {
        if (ly < 144) {
            std::fill_n(writebuffer.begin() + ly * 160, 160, getPalette(bgp)[0]);
        }
        return;
    }
    uint16_t tileSelect = ((lcdc & 0x8) >> 3) ? 0x9C00 : 0x9800;
    uint16_t tileData = ((lcdc & 0x10) >> 4) ? 0x8000 : 0x9000;
    std::vector<uint32_t> col = getPalette(bgp);

    uint8_t tileY = ((ly + scy) / 8) % 32;
    for (int i = 0; i < 21; ++i) {
        uint8_t tileX = (scx / 8 + i) % 32;

        uint8_t tileNumber = memory.read(tileSelect + (tileY * 32) + tileX);
        uint8_t pixelY = (ly + scy) % 8;

        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        if (tileData == 0x8000) {
            byte1 = memory.read(tileData + tileNumber * 16 + (pixelY * 2));
            byte2 = memory.read(tileData + tileNumber * 16 + (pixelY * 2) + 1);
        } else {
            byte1 = memory.read(tileData + ((int8_t) (tileNumber) * 16) + (pixelY * 2));
            byte2 = memory.read(tileData + ((int8_t) (tileNumber) * 16) + (pixelY * 2) + 1);
        }

        for (uint8_t pixelX = 0; pixelX < 8; ++pixelX) {
            uint8_t bit1 = (byte1 >> (7 - pixelX)) & 0x1;
            uint8_t bit2 = (byte2 >> (7 - pixelX)) & 0x1;
            uint8_t color = (bit2 << 1) | bit1;
            uint32_t pixel = col[color];

            int X = i * 8 + pixelX - (scx % 8);
            if (X >= 0 && ly < 144 && X < 160) {
                writebuffer[ly * 160 + X] = pixel;
                line[X] = color;
            }
        }
    }
}

void PPU::drawWindow() {
    if (!((lcdc & 0x20) && (lcdc & 0x1) && wx <= 166 && wy <= 143 && wy <= ly)) {
        return;
    }
    uint16_t tileSelect = (((lcdc & 0x40) >> 6)) ? 0x9C00 : 0x9800;
    uint16_t tileData = ((((lcdc & 0x10) >> 4)) ? 0x8000 : 0x9000);
    std::vector<uint32_t> col = getPalette(bgp);

    uint8_t tileY = windowCounter / 8;
    uint8_t pixelY = windowCounter % 8;
    windowCounter++;
    for (int i = 0; i < 21; ++i) {
        uint8_t tileNumber = memory.read(tileSelect + (tileY * 32) + i);

        uint8_t byte1 = 0;
        uint8_t byte2 = 0;
        if (tileData == 0x8000) {
            byte1 = memory.read(tileData + (tileNumber*16) + (pixelY * 2));
            byte2 = memory.read(tileData + (tileNumber*16) + (pixelY * 2) + 1);
        } else {
            byte1 = memory.read(tileData + ((int8_t) (tileNumber) * 16) + (pixelY * 2));
            byte2 = memory.read(tileData + ((int8_t) (tileNumber) * 16) + (pixelY * 2) + 1);
        }

        for (int pixelX = 0; pixelX < 8; ++pixelX) { // X
            uint8_t bit1 = (byte1 >> (7 - pixelX)) & 0x1;
            uint8_t bit2 = (byte2 >> (7 - pixelX)) & 0x1;
            uint8_t color = (bit2 << 1) | bit1;
            uint32_t pixel = col[color];

            int X = i * 8 + pixelX + wx - 7;
            if (X >= 0 && ly < 144 && X < 160) {
                writebuffer[ly * 160 + X] = pixel;
                line[X] = color;
            }
        }
    }
}

void PPU::drawSprites() {
    if (!(lcdc & 0x2)) {
        return;
    }
    int size = ((lcdc & 0x4) >> 2) ? 16 : 8;
    const uint16_t OAM = 0xFE00;

    typedef std::tuple<int, int, int> SpriteTuple;
    std::vector<SpriteTuple> sprites = std::vector<SpriteTuple>();
    sprites.reserve(10);
    for (int byte = 0; byte < 0x9F; byte += 4) {
        int Y = memory.read(OAM+byte) - 16;
        int X = memory.read(OAM+byte + 1) - 8;

        if (Y <= ly && Y > ly - size) {
            sprites.push_back(std::make_tuple(byte, Y, X));
            if (sprites.size() >= 10) {
                break;
            }
        }
    }

    std::sort(sprites.begin(), sprites.end(),
        [](const SpriteTuple& a, const SpriteTuple& b) {
            return (std::get<2>(a) >= std::get<2>(b));
        }
    );

    for (const auto &sprite : sprites) {
        int byte = std::get<0>(sprite);
        int Y = std::get<1>(sprite);
        int X = std::get<2>(sprite);

        if (X == -8 || X >= 160) {
            continue;
        }

        uint8_t tileNumber = memory.read(OAM+byte+2);
        if (size == 16) {
            tileNumber &= ~1;
        }

        uint8_t attributes = memory.read(OAM+byte+3);
        std::vector<uint32_t> col = getPalette((attributes & 0x10) ? obp1 : obp0);
        bool xFlip = attributes & 0x20;
        bool yFlip = attributes & 0x40;
        bool priority = attributes & 0x80;

        int i = ly - Y;
        uint8_t byte1 = memory.read(0x8000 + (tileNumber*16) + i * 2);
        uint8_t byte2 = memory.read(0x8000 + (tileNumber*16) + i * 2 + 1);
        if (yFlip) {
            byte1 = memory.read(0x8000 + (tileNumber*16) + (size - 1 - i) * 2);
            byte2 = memory.read(0x8000 + (tileNumber*16) + (size - 1 - i) * 2 + 1);
        }

        for (int j = 0; j < 8; ++j) {
            int XX = X + j;
            if (ly < 144 && XX < 160 && XX >= 0) {
                if (priority && line[XX] > 0) {
                    continue;
                }
                uint8_t bit1 = (byte1 >> (7 - j)) & 0x1;
                uint8_t bit2 = (byte2 >> (7 - j)) & 0x1;
                if (xFlip) {
                    bit1 = (byte1 >> j) & 0x1;
                    bit2 = (byte2 >> j) & 0x1;
                }
                uint8_t color = (bit2 << 1) | bit1;
                uint32_t pixel = col[color];

                if (color != 0) {
                    writebuffer[ly * 160 + XX] = pixel;
                }
            }
        }
    }
}
