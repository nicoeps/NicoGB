#include "joypad.hpp"

class Memory {
    private:
    public:
        Cartridge& cartridge;
        Joypad& joypad;

        uint8_t memory[0x10000] = {};
        uint8_t vram[0x2000] = {};
        uint8_t wram[0x2000] = {};
        uint8_t oam[0xA0] = {};
        uint8_t IO[0x100] = {};
        uint8_t hram[0x7F] = {};
        uint8_t boot[0x100] = {
            0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E, 
            0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 
            0x47, 0x11, 0xA8, 0x00, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B, 
            0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9, 
            0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 
            0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04, 
            0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2, 
            0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 
            0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20, 
            0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17, 
            0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0C, 0x00, 0x0F, 
            0x00, 0x01, 0x00, 0x0E, 0x36, 0x66, 0xC6, 0x60, 0xFC, 0xCF, 0x8D, 0xD9, 0xCE, 0xFF, 0x6F, 0xFF, 
            0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDC, 0x98, 0x9F, 0xB3, 0xB1, 0x33, 0x3E, 0x66, 0x63, 0xEE, 0x6E, 
            0xCC, 0xCF, 0xCC, 0xC8, 0xF7, 0x31, 0xEC, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
            0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0x01, 0x23, 0x7D, 0xFE, 0x34, 0x20, 
            0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0x01, 0x3E, 0x01, 0xE0, 0x50
        };

        bool bootEnabled = true;
        void load(std::string path);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        void interrupt(uint8_t IRQ);

        Memory(Cartridge& cartridge, Joypad& joypad) : cartridge(cartridge), joypad(joypad) {}
};