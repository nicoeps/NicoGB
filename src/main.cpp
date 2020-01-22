#include <cstdint>
#include <thread>
#include <chrono>

#include "SDL2/SDL.h"

#include "cartridge.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

auto epoch = std::chrono::high_resolution_clock::from_time_t(0);

auto now() {
    return std::chrono::high_resolution_clock::now();
}

auto millis() {
    return std::chrono::duration_cast
    <std::chrono::milliseconds>(now() - epoch).count();
}

void startup(PPU &ppu) {
    memset(ppu.cpu.memory.memory, 0, sizeof(ppu.cpu.memory.memory));
    memset(ppu.cpu.memory.vram, 0, sizeof(ppu.cpu.memory.vram));
    memset(ppu.cpu.memory.wram, 0, sizeof(ppu.cpu.memory.wram));
    memset(ppu.cpu.memory.oam, 0, sizeof(ppu.cpu.memory.oam));
    memset(ppu.cpu.memory.IO, 0, sizeof(ppu.cpu.memory.IO));
    memset(ppu.cpu.memory.hram, 0, sizeof(ppu.cpu.memory.hram));
    ppu.cpu.memory.bootEnabled = true;
    ppu.ly = 0;
    ppu.stat = 0;
    ppu.totalCycles = 0;
    ppu.cpu.memory.write(0xFF00, 0xCF);  // Joypad 1100 1111 (No Buttons pressed)
    ppu.cpu.memory.write(0xFF05, 0x0);   // TIMA
    ppu.cpu.memory.write(0xFF06, 0x0);   // TMA
    ppu.cpu.memory.write(0xFF07, 0x0);   // TAC
    ppu.cpu.memory.write(0xFF10, 0x80);  // NR10
    ppu.cpu.memory.write(0xFF11, 0xBF);  // NR11
    ppu.cpu.memory.write(0xFF12, 0xF3);  // NR12
    ppu.cpu.memory.write(0xFF14, 0xBF);  // NR14
    ppu.cpu.memory.write(0xFF16, 0x3F);  // NR21
    ppu.cpu.memory.write(0xFF17, 0x00);  // NR22
    ppu.cpu.memory.write(0xFF19, 0xBF);  // NR24
    ppu.cpu.memory.write(0xFF1A, 0x7F);  // NR30
    ppu.cpu.memory.write(0xFF1B, 0xFF);  // NR31
    ppu.cpu.memory.write(0xFF1C, 0x9F);  // NR32
    ppu.cpu.memory.write(0xFF1E, 0xBF);  // NR33
    ppu.cpu.memory.write(0xFF20, 0xFF);  // NR41
    ppu.cpu.memory.write(0xFF21, 0x00);  // NR42
    ppu.cpu.memory.write(0xFF22, 0x00);  // NR43
    ppu.cpu.memory.write(0xFF23, 0xBF);  // NR30
    ppu.cpu.memory.write(0xFF24, 0x77);  // NR50
    ppu.cpu.memory.write(0xFF25, 0xF3);  // NR51
    ppu.cpu.memory.write(0xFF26, 0xF1);  // NR52
    ppu.cpu.memory.write(0xFF40, 0x91);  // LCDC
    ppu.cpu.memory.write(0xFF42, 0x00);  // SCY
    ppu.cpu.memory.write(0xFF43, 0x00);  // SCX
    ppu.cpu.memory.write(0xFF44, 0x00);  // LY
    ppu.cpu.memory.write(0xFF45, 0x00);  // LYC
    ppu.cpu.memory.write(0xFF47, 0xFC);  // BGP
    ppu.cpu.memory.write(0xFF48, 0xFF);  // OBP0
    ppu.cpu.memory.write(0xFF49, 0xFF);  // OBP1
    ppu.cpu.memory.write(0xFF4A, 0x00);  // WY
    ppu.cpu.memory.write(0xFF4B, 0x00);  // WX
    ppu.cpu.memory.write(0xFF0F, 0x00);  // IF
    ppu.cpu.memory.write(0xFFFF, 0x00);  // IE
}

int main() {
    Cartridge cartridge;
    Joypad joypad;
    Memory memory(cartridge, joypad);
    CPU cpu(memory);
    PPU ppu(cpu);

    memory.bootEnabled = true;

    const int SCALE = 4;
    const int WIDTH = 160;
    const int HEIGHT = 144;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Event event;

    SDL_Window *window = SDL_CreateWindow("NicoGB",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH*SCALE, HEIGHT*SCALE, 0);

    // SDL_SetWindowSize(window, WIDTH, HEIGHT);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_Texture *texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 160, 144);


    SDL_UpdateTexture(texture, NULL, ppu.framebuffer, 160 * sizeof(uint32_t));

    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    std::string path;

    SDL_bool done = SDL_FALSE;
    while (!done) {
        while (!done && SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: {
                    cpu.run = false;
                    done = SDL_TRUE;
                    break;
                }

                case SDL_DROPFILE: {
                    path = event.drop.file;
                    cartridge.load(path);
                    startup(ppu);
                    done = SDL_TRUE;
                    break;
               }
            }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        }
    }

    auto last = millis();
    while (cpu.run) {
        cpu.cycle();
        ppu.update();
        if (millis() - last >= 1000/60) {
            last = millis();
            SDL_UpdateTexture(texture, NULL, ppu.framebuffer, 160 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        cpu.run = false;
                        break;

                    case SDL_DROPFILE:
                        path = event.drop.file;
                        cpu.init();
                        cartridge.load(path);
                        startup(ppu);
                        break;

                    case SDL_KEYDOWN:
                        joypad.keyDown(event.key.keysym);
                        break;

                    case SDL_KEYUP:
                        joypad.keyUp(event.key.keysym);
                        break;
                }
            }
            SDL_Delay(7);
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
