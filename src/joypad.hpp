#include "SDL2/SDL.h"

class Joypad {
    private:
    public:
        uint8_t keys[2] = {0x0F, 0x0F};
        uint8_t mode = 0;

        void reset();
        bool interrupt = false;
        uint8_t read();
        void write(uint8_t n);
        void keyDown(SDL_Keysym key);
        void keyUp(SDL_Keysym key);
};
