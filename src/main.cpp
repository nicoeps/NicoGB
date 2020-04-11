#include <thread>
#include <chrono>

#include "SDL2/SDL.h"

#include "timer.hpp"
#include "joypad.hpp"
#include "cartridge.hpp"
#include "memory.hpp"
#include "ppu.hpp"
#include "cpu.hpp"

auto epoch = std::chrono::high_resolution_clock::from_time_t(0);

auto now() {
    return std::chrono::high_resolution_clock::now();
}

auto millis() {
    return std::chrono::duration_cast
    <std::chrono::milliseconds>(now() - epoch).count();
}

Key getKey(SDL_Keycode sym) {
    switch (sym) {
        case SDLK_RIGHT:  return RIGHT;  break;
        case SDLK_LEFT:   return LEFT;   break;
        case SDLK_UP:     return UP;     break;
        case SDLK_DOWN:   return DOWN;   break;
        case SDLK_z:      return A;      break;
        case SDLK_x:      return B;      break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT: return SELECT; break;
        case SDLK_RETURN: return START;  break;
        default:          return NONE;   break;
    }
}

void init(CPU& cpu) {
    cpu.init();
    cpu.memory.timer.init();
    cpu.memory.init();
    cpu.ppu.init();
}

void run(CPU& cpu) {
    const int SCALE = 4;
    const int WIDTH = 160;
    const int HEIGHT = 144;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("NicoGB",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH*SCALE, HEIGHT*SCALE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture *texture = SDL_CreateTexture(renderer,
    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 160, 144);
    SDL_UpdateTexture(texture, NULL, cpu.ppu.framebuffer.data(), 160 * sizeof(uint32_t));

    SDL_Event event;
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::string path;
    Key key;

    auto last = millis();
    while (cpu.run) {
        if (cpu.memory.cartridge.loaded && cpu.totalCycles < 69905) {
            cpu.cycle();
        }

        if (millis() - last >= 1000/60) {
            last = millis();
            cpu.totalCycles = 0;

            SDL_UpdateTexture(texture, NULL, cpu.ppu.framebuffer.data(), 160 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        cpu.run = false;
                        break;

                    case SDL_DROPFILE:
                        init(cpu);
                        path = event.drop.file;
                        cpu.memory.cartridge.load(path);
                        SDL_SetWindowTitle(window, (std::string("NicoGB - ") + cpu.memory.cartridge.title.data()).c_str());
                        break;

                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_q: cpu.run = false; break;
                            case SDLK_r: init(cpu); break;
                            default:
                                key = getKey(event.key.keysym.sym);
                                cpu.memory.joypad.keyDown(key);
                                break;
                        }
                        break;

                    case SDL_KEYUP:
                        key = getKey(event.key.keysym.sym);
                        cpu.memory.joypad.keyUp(key);
                        break;

                    default: break;
                }
            }
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

#ifdef TEST

typedef std::tuple<std::string, std::string, std::string> testcase;

std::string assert(CPU& cpu, testcase test) {
    std::string output;
    std::string passed;
    std::string failed;
    std::string rom;

    std::tie(passed, failed, rom) = test;

    init(cpu);
    cpu.memory.cartridge.load(rom);
    if (!cpu.memory.cartridge.loaded) return rom + ": file not found";

    auto time = millis();
    while (cpu.run) {
        cpu.cycle();
        if (cpu.memory.read(0xFF02) == 0x81) {
            cpu.memory.write(0xFF02, 0);
            output += cpu.memory.read(0xFF01);
        }
        if (output.find(passed) != std::string::npos) {
            return rom + ": pass";
        } else if (output.find(failed) != std::string::npos) {
            return rom + ": fail";
        }
        if (millis() - time >= 10000) break;
    }
    return rom + ": timeout";
}

#endif

int main() {
    Timer timer;
    Cartridge cartridge;
    Joypad joypad;
    Memory memory(cartridge, joypad, timer);
    PPU ppu(memory);
    CPU cpu(memory, ppu);

#ifndef TEST

    run(cpu);

#else

    std::vector <testcase> tests {
        testcase("Passed", "Failed", "tests/blargg/cpu_instrs/cpu_instrs.gb"),
        testcase("Passed", "Failed", "tests/blargg/instr_timing/instr_timing.gb"),
        testcase("Passed", "Failed", "tests/blargg/mem_timing/mem_timing.gb"),
    };

    for (auto test: tests) {
        printf("%s\n", assert(cpu, test).c_str());
    }

#endif

    return 0;
}
