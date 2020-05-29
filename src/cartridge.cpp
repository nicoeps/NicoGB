#include <fstream>

#include "cartridge.hpp"

Cartridge::Cartridge() {
    cartridgeROM = std::vector<uint8_t>();
    cartridgeRAM = std::vector<uint8_t>();
    title = "";
    loaded = false;
    cartridgeType = 0;
    romSize = 0;
    ramSize = 0;
}

void Cartridge::load(std::string path) {
    std::ifstream cartridgeFile(path, std::ifstream::binary);

    if (!cartridgeFile.is_open()) {
        loaded = false;
        return;
    } else {
        cartridgeFile.seekg(0, cartridgeFile.end);
        romSize = cartridgeFile.tellg();
        cartridgeFile.seekg(0, cartridgeFile.beg);
        cartridgeFile.clear();
        if (romSize % 32768 != 0) {
            loaded = false;
            cartridgeFile.close();
            return;
        }
        cartridgeROM.resize(romSize);
        cartridgeFile.read((char*) cartridgeROM.data(), romSize);
        cartridgeFile.close();
        loaded = true;

        cartridgeType = cartridgeROM[0x0147];
        switch (cartridgeROM[0x0149]) {
            case 0:  ramSize = 1;         break;
            case 1:  ramSize = 2048;      break;
            case 2:  ramSize = 8192;      break;
            case 3:  ramSize = 4 * 8192;  break;
            case 4:  ramSize = 16 * 8192; break;
            case 5:  ramSize = 8 * 8192;  break;
            default: ramSize = 1;         break;
        }

        if (cartridgeType == 0x05 || cartridgeType == 0x06) { // MBC2
            ramSize = 512;
        }

        cartridgeRAM.resize(ramSize);
        std::fill_n(cartridgeRAM.begin(), ramSize, 0);

        int titleSize = cartridgeROM[0x0143] & 0x80 ? 15 : 16; // CGB Flag
        title.assign(&cartridgeROM[0x0134], &cartridgeROM[0x0134] + titleSize);

        switch (cartridgeType) {
            // ROM
            case 0x00:
            case 0x08:
            case 0x09:
                mbc.reset(new MBC0(cartridgeROM, cartridgeRAM, ramSize));
                break;

            // MBC1
            case 0x01:
            case 0x02:
            case 0x03:
                mbc.reset(new MBC1(cartridgeROM, cartridgeRAM, romSize, ramSize));
                break;

            // MBC2
            case 0x05:
            case 0x06:
                mbc.reset(new MBC2(cartridgeROM, cartridgeRAM, romSize, ramSize));
                break;

            // MMM01
            // case 0x0B:
            // case 0x0C:
            // case 0x0D:
                // break;

            // MBC3
            case 0x0F:
            case 0x10:
            case 0x11:
            case 0x12:
            case 0x13:
                mbc.reset(new MBC3(cartridgeROM, cartridgeRAM, romSize, ramSize));
                break;

            // MBC5
            case 0x19:
            case 0x1A:
            case 0x1B:
            case 0x1C:
            case 0x1D:
            case 0x1E:
                mbc.reset(new MBC5(cartridgeROM, cartridgeRAM, romSize, ramSize));
                break;

            // MBC6
            // case 0x20:
            //     break;

            // MBC7
            // case 0x22:
            //     break;

            default:
                mbc.reset(new MBC0(cartridgeROM, cartridgeRAM, ramSize));
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
