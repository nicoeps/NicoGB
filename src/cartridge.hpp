#include "mbc.hpp"

class Cartridge {
    private:
    public:
        long cartridgeSize;
        char* cartridgeData;
        char* cartridgeRAM;
        bool loaded = false;

        uint8_t title[16];
        uint8_t cartridgeType = 0;
        int romSize = 0;
        uint8_t romBank = 0x1;
        int ramSize = 0;
        uint8_t ramBank = 0x0;
        bool ramEnable = false;
        uint8_t modeSelect = 0;
        
        void setRomBank(uint8_t n);
        void load(std::string path);
        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);

        MBC *mbc = new MBC0(&cartridgeData, &cartridgeRAM, romSize, ramSize);
};
