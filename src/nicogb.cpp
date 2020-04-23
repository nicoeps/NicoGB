#include <chrono>

#include "nicogb.hpp"

NicoGB::NicoGB() :
    memory(cartridge, joypad, timer),
    ppu(memory),
    cpu(memory, ppu),
    loaded(cartridge.loaded),
    title(cartridge.title),
    framebuffer(ppu.framebuffer) {
        timer = Timer();
        joypad = Joypad();
        cartridge = Cartridge();
        speed = 0;
        epoch = std::chrono::high_resolution_clock::from_time_t(0);
        last = millis();
}

void NicoGB::init() {
    cpu.init();
    timer.init();
    joypad.reset();
    memory.init();
    ppu.init();
}

std::chrono::_V2::system_clock::time_point NicoGB::now() {
    return std::chrono::high_resolution_clock::now();
}

int64_t NicoGB::millis() {
    return std::chrono::duration_cast
    <std::chrono::milliseconds>(now() - epoch).count();
}

void NicoGB::load(std::string path) {
    init();
    cartridge.load(path);
}

void NicoGB::tick() {
    if (speed) {
        cpu.totalCycles = 0;
    }

    if (cartridge.loaded && cpu.totalCycles < 69905) {
        cpu.cycle();
    }

    if (millis() - last >= 1000/60) {
        last = millis();
        if (cartridge.loaded && !speed) {
            cpu.totalCycles -= 69905;
        }
    }
}

void NicoGB::keyDown(Key key) {
    joypad.keyDown(key);
}

void NicoGB::keyUp(Key key) {
    joypad.keyUp(key);
}

uint8_t NicoGB::serialDataRead() {
    return memory.read(0xFF01);
}

void NicoGB::serialDataWrite(uint8_t value) {
    memory.write(0xFF01, value);
}

bool NicoGB::serialTransferRead() {
    return memory.read(0xFF02) & 0x80;
}

void NicoGB::serialTransferWrite(bool value) {
    uint8_t serialControl = memory.read(0xFF02) & 0x1;
    memory.write(0xFF02, (value << 7) | serialControl);
}
