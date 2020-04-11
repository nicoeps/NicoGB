#include "mbc.hpp"

// ROM
uint8_t MBC0::read(uint16_t address) {
    if (address <= 0x7FFF) {
        return rom[address];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        return ram[address - 0xA000];
    } else {
        return 0xFF;
    }
}

void MBC0::write(uint16_t address, uint8_t n) {
    if (address >= 0xA000 && address <= 0xBFFF) {
        ram[address - 0xA000] = n;
    }
}


// MBC1
uint8_t MBC1::read(uint16_t address) {
    if (address <= 0x3FFF) {
        romBank = (mode ? (bank2 << 5) : 0);
        return rom[((romBank << 14) | (address & 0x3FFF)) % romSize];
    } else if (address <= 0x7FFF) {
        romBank = ((bank2 << 5) | bank1);
        return rom[((romBank << 14) | (address & 0x3FFF)) % romSize];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            ramBank = (mode ? bank2 : 0);
            return ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize];
        } else {
            return 0xFF;
        }
    } else {
        return 0xFF;
    }
}

void MBC1::write(uint16_t address, uint8_t n) {
    if (address <= 0x1FFF) {
        ramg = ((n & 0x0F) == 0x0A);
    } else if (address <= 0x3FFF) {
        n &= 0x1F;
        if (n == 0) {
            n = 0x1;
        }
        bank1 = n;
    } else if (address <= 0x5FFF) {
        bank2 = n & 0x3;
    } else if (address <= 0x7FFF) {
        mode = n & 0x1;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            ramBank = (mode ? bank2 : 0);
            ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize] = n;
        }
    }
}


// MBC3
uint8_t MBC3::read(uint16_t address) {
    if (address <= 0x3FFF) {
        return rom[address];
    } else if (address <= 0x7FFF) {
        return rom[((romBank << 14) | (address & 0x3FFF)) % romSize];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            if (ramBank <= 0x3) {
                return ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize];
            } else if (ramBank >= 0x08 && ramBank <= 0x0C) {
                return RTC[ramBank - 0x08];
            } else {
                return 0xFF;
            }
        } else {
            return 0xFF;
        }
    } else {
        return 0xFF;
    }
}

void MBC3::write(uint16_t address, uint8_t n) {
    if (address <= 0x1FFF) {
        ramg = ((n & 0x0F) == 0x0A);
    } else if (address <= 0x3FFF) {
        n &= 0x7F;
        if (n == 0) {
            n = 0x1;
        }
        romBank = n;
    } else if (address <= 0x5FFF) {
        ramBank = n;
    } else if (address <= 0x7FFF) {
        ;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            if (ramBank <= 0x3) {
                ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize] = n;
            } else if (ramBank >= 0x08 && ramBank <= 0x0C) {
                RTC[ramBank - 0x08] = n;
            }
        }
    }
}


// MBC5
uint8_t MBC5::read(uint16_t address) {
    if (address <= 0x3FFF) {
        return rom[address];
    } else if (address <= 0x7FFF) {
        romBank = ((bank2 << 8) | bank1);
        return rom[((romBank << 14) | (address & 0x3FFF)) % romSize];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            return ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize];
        } else {
            return 0xFF;
        }
    } else {
        return 0xFF;
    }
}

void MBC5::write(uint16_t address, uint8_t n) {
    if (address <= 0x1FFF) {
        ramg = (n == 0x0A);
    } else if (address <= 0x2FFF) {
        bank1 = n;
    } else if (address <= 0x3FFF) {
        bank2 = n & 0x1;
    } else if (address <= 0x5FFF) {
        ramBank = n & 0xF;
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (ramg) {
            ram[((ramBank << 13) | (address & 0x1FFF)) % ramSize] = n;
        }
    }
}
