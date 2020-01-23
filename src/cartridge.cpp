#include <string>
#include <cstdint>
#include <fstream>
#include <cstring>

#include "cartridge.hpp"

void Cartridge::load(std::string path) {
    std::ifstream cartridgeFile;
    cartridgeFile.open(path, std::ifstream::binary);

    if (cartridgeFile) {
        cartridgeFile.seekg(0, cartridgeFile.end);
        cartridgeSize = (unsigned long) cartridgeFile.tellg();
        cartridgeFile.seekg(0, cartridgeFile.beg);

        cartridgeData = new char [cartridgeSize];

        cartridgeFile.read(cartridgeData, cartridgeSize);
        loaded = true;
        cartridgeFile.close();
    
        // Title
        for (int i = 0; i < 16; ++i) {
            printf("%c", cartridgeData[0x0134+i]);
        }
        printf("\n");
        cartridgeType = cartridgeData[0x0147];

        romSize = cartridgeSize;

        switch (cartridgeData[0x0149]) {
            case 0:
                ramSize = 1;
                break;
            
            case 1:
                ramSize = 2048;
                break;
            
            case 2:
                ramSize = 8192;
                break;

            case 3:
                ramSize = 4 * 8192;
                break;

            case 4:
                ramSize = 16 * 8192;
                break;

            case 5:
                ramSize = 8 * 8192;
                break;
        }
    
        printf("0x%X 0x%X 0x%X\n", cartridgeType, cartridgeData[0x0148],  cartridgeData[0x0149]);

        cartridgeRAM = new char [ramSize];
        memset(cartridgeRAM, 0, ramSize);

        switch (cartridgeType) {
            // ROM
            case 0x00:
            case 0x08:
            case 0x09:
                mbc = new MBC0(&cartridgeData, &cartridgeRAM, romSize, ramSize);
                break;

            // MBC1
            case 0x01:
            case 0x02:
            case 0x03:
                mbc = new MBC1(&cartridgeData, &cartridgeRAM, romSize, ramSize);
                break;

            // // MBC2
            // case 0x05:
            // case 0x06:
                // break;
    
            // // MMM01
            // case 0x0B:
            // case 0x0C:
            // case 0x0D:
                // break;

            /// MBC3
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                mbc = new MBC3(&cartridgeData, &cartridgeRAM, romSize, ramSize);
                break;

            // MBC5
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
                mbc = new MBC5(&cartridgeData, &cartridgeRAM, romSize, ramSize);
                break;

            // // MBC6
            // case 0x20:
            //     break;

            // // MBC7
            // case 0x22:
            //     break;

            default:
                mbc = new MBC1(&cartridgeData, &cartridgeRAM, romSize, ramSize);
                break;
        }
    }
}

uint8_t Cartridge::read(uint16_t address) {
    return mbc->read(address);
}

void Cartridge::write(uint16_t address, uint8_t n) {
    mbc->write(address, n);
}
