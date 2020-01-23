class MBC {
    public:
        virtual uint8_t read(uint16_t address) = 0;
        virtual void write(uint16_t address, uint8_t n) = 0;
};

class MBC0 : public MBC {
    public:
        char* rom;
        char* ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC0(char** rom, char** ram, int romSize, int ramSize)
            : rom(*rom), ram(*ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC1 : public MBC {
    public:
        char* rom;
        char* ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t bank1 = 1;
        uint8_t bank2 = 0;
        uint8_t romBank = 0;
        uint8_t ramBank = 0;
        bool mode = 0;
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC1(char** rom, char** ram, int romSize, int ramSize)
            : rom(*rom), ram(*ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC2 : public MBC {
    public:
        char* rom;
        char* ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC2(char** rom, char** ram, int romSize, int ramSize)
            : rom(*rom), ram(*ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC3 : public MBC {
    public:
        char* rom;
        char* ram;
        bool ramg = 0;
        int romSize;
        int ramSize;
        uint8_t romBank = 1;
        uint8_t ramBank = 0;
        uint8_t RTC[5] = {};
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC3(char** rom, char** ram, int romSize, int ramSize)
            : rom(*rom), ram(*ram), romSize(romSize), ramSize(ramSize) {}
};

class MBC5 : public MBC {
    public:
        char* rom;
        char* ram;
        int romSize;
        int ramSize;
        bool ramg = 0;
        uint8_t bank1 = 1;
        uint8_t bank2 = 0;
        uint16_t romBank = 0;
        uint8_t ramBank = 0;
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);
        MBC5(char** rom, char** ram, int romSize, int ramSize)
            : rom(*rom), ram(*ram), romSize(romSize), ramSize(ramSize) {}
};
