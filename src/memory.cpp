#include <string>
#include <cstdint>
#include <fstream>

#include "cartridge.hpp"
#include "memory.hpp"

void Memory::interrupt(uint8_t IRQ) {
    write(0xFF0F, read(0xFF0F) | IRQ);
}

uint8_t Memory::read(uint16_t address) {
    // Bootrom
    if (address < 0x100 && bootEnabled) {
        return boot[address];
    
    // Cartridge ROM
    } else if (address <= 0x7FFF && cartridge.loaded == true) {
        return cartridge.read(address);

    // VRAM
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // if (!((read(0xFF41) & 0x03) == 0x03)) {
            return vram[address - 0x8000];
        // }
        return 0xFF;

    // Cartridge RAM
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        return cartridge.read(address);

    // WRAM
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        return wram[address - 0xC000];

    // OAM
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // if (!((read(0xFF41) & 0x03) == 0x03)) {
            return oam[address - 0xFE00];
        // }
        return 0xFF;

    // IO
    } else if ((address >= 0xFF00 && address <= 0xFF7F) || address == 0xFFFF) {
        if (address == 0xFF00) {
            return joypad.read();
        }
        return IO[address - 0xFF00];

    // HRAM
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        return hram[address - 0xFF80];
    } else {
        return memory[address];
        // printf("ERROR: Reading from invalid address: 0x%04X\n", address);
        return 0xFF;
    }
}

void Memory::write(uint16_t address, uint8_t n) {
    if (address <= 0x7FFF) {
        cartridge.write(address, n);
    } else if (address >= 0x8000 && address <= 0x9FFF) {
        // if (!((read(0xFF41) & 0x03) == 0x03)) {
            vram[address - 0x8000] = n;
        // }
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        cartridge.write(address, n); 
    } else if (address >= 0xC000 && address <= 0xDFFF) {
        wram[address - 0xC000] = n;
    } else if (address >= 0xFE00 && address <= 0xFE9F) {
        // if (!((read(0xFF41) & 0x03) == 0x03)) {
            oam[address - 0xFE00] = n;
        // }
    } else if ((address >= 0xFF00 && address <= 0xFF7F) || address == 0xFFFF) {
        if (address == 0xFF46) { // DMA Transfers
            // TODO
            IO[address - 0xFF00] = n;
        } else if (address == 0xFF50 && n == 0x01) {
            IO[address - 0xFF00] = n;
            bootEnabled = false;
        } else if (address == 0xFF04) { // DIV
            IO[address - 0xFF00] = 0;
        } else if (address == 0xFF00) { // Joypad
            joypad.write(n);
        } else {
            IO[address - 0xFF00] = n;
        }
    } else if (address >= 0xFF80 && address <= 0xFFFE) {
        hram[address - 0xFF80] = n;
    } else {
        memory[address] = n;
        // printf("ERROR: Writing to invalid address: 0x%04X\n", address);
    }
}
