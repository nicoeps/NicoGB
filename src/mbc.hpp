#pragma once

#include <cstdint>
#include <vector>

class MBC {
    public:
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t n) = 0;
        virtual ~MBC() {};
};

class MBC0 : public MBC {
    private:
        std::vector<uint8_t>& rom;
        std::vector<uint8_t>& ram;

    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);

        MBC0(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram)
            : rom(rom), ram(ram) {}
};

class MBC1 : public MBC {
    private:
        std::vector<uint8_t>& rom;
        std::vector<uint8_t>& ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t bank1 = 1;
        uint8_t bank2 = 0;
        uint8_t romBank = 0;
        uint8_t ramBank = 0;
        bool mode = 0;

    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC1(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, int romSize, int ramSize)
            : rom(rom), ram(ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC2 : public MBC {
    private:
        std::vector<uint8_t>& rom;
        std::vector<uint8_t>& ram;
        bool ramg = 0;
        int romSize;
        int ramSize;

    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC2(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, int romSize, int ramSize)
            : rom(rom), ram(ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC3 : public MBC {
    private:
        std::vector<uint8_t>& rom;
        std::vector<uint8_t>& ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t romBank = 1;
        uint8_t ramBank = 0;
        uint8_t RTC[5] = {};

    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC3(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, int romSize, int ramSize)
            : rom(rom), ram(ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC5 : public MBC {
    private:
        std::vector<uint8_t>& rom;
        std::vector<uint8_t>& ram;
        int romSize;
        int ramSize;
        bool ramg = 0;
        uint8_t bank1 = 1;
        uint8_t bank2 = 0;
        uint16_t romBank = 0;
        uint8_t ramBank = 0;

    public:
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC5(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram, int romSize, int ramSize)
            : rom(rom), ram(ram), romSize(romSize), ramSize(ramSize) {}
};
