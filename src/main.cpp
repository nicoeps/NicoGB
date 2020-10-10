#include <chrono>
#include <filesystem>

#include "SDL2/SDL.h"

#include "nicogb.hpp"

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

Key getGameController(SDL_GameControllerButton button) {
    switch (button) {
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return RIGHT;  break;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:  return LEFT;   break;
        case SDL_CONTROLLER_BUTTON_DPAD_UP:    return UP;     break;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:  return DOWN;   break;
        case SDL_CONTROLLER_BUTTON_B:
        case SDL_CONTROLLER_BUTTON_X:          return A;      break;
        case SDL_CONTROLLER_BUTTON_A:
        case SDL_CONTROLLER_BUTTON_Y:          return B;      break;
        case SDL_CONTROLLER_BUTTON_BACK:       return SELECT; break;
        case SDL_CONTROLLER_BUTTON_START:      return START;  break;
        default:                               return NONE;   break;
    }
}

void run(NicoGB& nicogb) {
    const int SCALE = 4;
    const int WIDTH = 160;
    const int HEIGHT = 144;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    SDL_Window *window = SDL_CreateWindow("NicoGB",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH*SCALE, HEIGHT*SCALE, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture *texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 160, 144);
    SDL_UpdateTexture(texture, NULL, nicogb.framebuffer.data(), 160 * sizeof(uint32_t));

    SDL_Event event;
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    std::string path;
    Key key;

    SDL_GameController *controller = NULL;

    auto last = millis();
    bool run = true;
    while (run) {
        nicogb.tick();

        if (millis() - last >= 1000/60) {
            last = millis();

            SDL_UpdateTexture(texture, NULL, nicogb.framebuffer.data(), 160 * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT:
                        run = false;
                        break;

                    case SDL_DROPFILE:
                        path = event.drop.file;
                        nicogb.load(path);
                        SDL_SetWindowTitle(window, (std::string("NicoGB - ") + nicogb.title).c_str());
                        break;

                    case SDL_KEYDOWN:
                        switch (event.key.keysym.sym) {
                            case SDLK_q:
                                run = false;
                                break;
                            case SDLK_r:
                                nicogb.init();
                                break;
                            case SDLK_SPACE:
                                nicogb.speed = !nicogb.speed;
                                break;
                            default:
                                key = getKey(event.key.keysym.sym);
                                nicogb.keyDown(key);
                                break;
                        }
                        break;

                    case SDL_KEYUP:
                        key = getKey(event.key.keysym.sym);
                        nicogb.keyUp(key);
                        break;

                    case SDL_CONTROLLERDEVICEADDED:
                        if (SDL_IsGameController(event.cdevice.which)) {
                            controller = SDL_GameControllerOpen(event.cdevice.which);
                        }
                        break;

                    case SDL_CONTROLLERDEVICEREMOVED:
                        SDL_GameControllerClose(controller);
                        break;

                    case SDL_CONTROLLERBUTTONDOWN:
                        switch (event.cbutton.button) {
                            case SDL_CONTROLLER_BUTTON_GUIDE:
                                nicogb.speed = !nicogb.speed;
                                break;
                            default:
                                key = getGameController((SDL_GameControllerButton) event.cbutton.button);
                                nicogb.keyDown(key);
                                break;
                        }
                        break;

                    case SDL_CONTROLLERBUTTONUP:
                        key = getGameController((SDL_GameControllerButton) event.cbutton.button);
                        nicogb.keyUp(key);
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

std::string assert(NicoGB& nicogb, testcase test) {
    std::string output;
    std::string passed;
    std::string failed;
    std::string rom;

    std::tie(passed, failed, rom) = test;

    nicogb.load(rom);
    if (!nicogb.loaded) return rom + ": file not found";

    auto time = millis();
    bool run = true;
    nicogb.speed = 1;
    while (run) {
        nicogb.tick();
        if (nicogb.serialTransferRead() == 1) {
            nicogb.serialTransferWrite(0);
            output += nicogb.serialDataRead();
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
    NicoGB nicogb;

#ifndef TEST

    run(nicogb);

#else

    std::vector <testcase> tests {
        testcase("Passed", "Failed", "tests/blargg/"),
        testcase({ 3, 5, 8, 13, 21, 34 }, { 66 }, "tests/mooneye/"),
    };

    for (auto& test: tests) {
        std::string path = std::get<2>(test);
        if (path.find(".gb") != std::string::npos) {
            printf("%s\n", assert(nicogb, test).c_str());
            continue;
        }
        for(auto& p: std::filesystem::recursive_directory_iterator(path)) {
            if(p.path().extension() == ".gb") {
                printf("%s\n", assert(nicogb,
                    testcase(std::get<0>(test), std::get<1>(test), p.path())).c_str());
            }
        }
    }

#endif

    return 0;
}
