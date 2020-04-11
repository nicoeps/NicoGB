#pragma once

#include <cstdint>

union RegisterPair {
    struct {
        uint8_t low;
        uint8_t high;
    };
    struct {
        uint16_t value;
    };
};

class Memory;
class PPU;

class CPU {
    public:
        Memory& memory;
        PPU& ppu;
        long long totalCycles;
        bool run;
        void init();
        void cycle();
        CPU(Memory& memory, PPU& ppu);

    private:
        RegisterPair af{}, bc{}, de{}, hl{};

        uint16_t& AF = af.value; uint8_t& A = af.high; uint8_t& F = af.low;
        uint16_t& BC = bc.value; uint8_t& B = bc.high; uint8_t& C = bc.low;
        uint16_t& DE = de.value; uint8_t& D = de.high; uint8_t& E = de.low;
        uint16_t& HL = hl.value; uint8_t& H = hl.high; uint8_t& L = hl.low;

        uint16_t SP;
        uint16_t PC;
        uint8_t opcode;

        bool halted;
        bool haltBug;
        uint8_t IRQ;
        bool IME;

        void tick();

        uint8_t ZERO_F();
        uint8_t SUB_F();
        uint8_t HALF_F();
        uint8_t CARRY_F();

        uint8_t ZERO_S(uint16_t n);
        uint8_t SUB_S(uint8_t a, uint8_t b);
        uint8_t HALF_S(uint8_t a, uint8_t b);
        uint8_t CARRY_S(uint8_t a, uint8_t b);

        uint8_t SUB_Se(uint16_t a, uint16_t b);
        uint8_t HALF_Se(uint16_t a, uint16_t b);
        uint8_t CARRY_Se(uint16_t a, uint16_t b);

        uint8_t SUB_Sc(uint8_t a, uint8_t b);
        uint8_t HALF_Sc(uint8_t a, uint8_t b);
        uint8_t CARRY_Sc(uint8_t a, uint8_t b);

        uint8_t HALF_Sb(uint8_t a, uint8_t b);
        uint8_t CARRY_Sb(uint8_t a, uint8_t b);

        uint8_t HALF_Sbc(uint8_t a, uint8_t b);
        uint8_t CARRY_Sbc(uint8_t a, uint8_t b);

        uint8_t read(uint16_t address);
        void write(uint16_t address, uint8_t n);

        uint8_t readByte();
        uint16_t readb16();

        void push(uint16_t r);
        uint16_t pop();

        void ADD(uint8_t n);
        void ADC(uint8_t n);
        void SUB(uint8_t n);
        void SBC(uint8_t n);
        void AND(uint8_t n);
        void OR(uint8_t n);
        void XOR(uint8_t n);
        void CP(uint8_t n);
        void INC(uint8_t n);
        void DEC(uint8_t n);
        void ADD_nn(uint16_t nn);

        void RLCA();
        void RLA();
        void RRCA();
        void RRA();

        uint8_t RL(uint8_t n);
        uint8_t RLC(uint8_t n);
        uint8_t RR(uint8_t n);
        uint8_t RRC(uint8_t n);

        uint8_t SLA(uint8_t n);
        uint8_t SRA(uint8_t n);
        uint8_t SRL(uint8_t n);
        uint8_t SWAP(uint8_t n);

        void BIT(uint8_t n, uint8_t b);
        uint8_t SET(uint8_t n, uint8_t b);
        uint8_t RES(uint8_t n, uint8_t b);

        void JP(bool flag);
        void JR(bool flag);

        void CALL(bool flag);
        void RET (bool flag);
};
