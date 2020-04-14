#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include "mbc.hpp"

class Cartridge {
    public:
        std::vector<char> title;
        bool loaded;
        void load(std::string path);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        Cartridge();

    private:
        std::unique_ptr<MBC> mbc;
        std::vector<uint8_t> cartridgeROM;
        std::vector<uint8_t> cartridgeRAM;
        uint8_t cartridgeType;
        size_t romSize;
        int ramSize;
};
