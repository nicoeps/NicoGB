#include <bitset>

#include "cartridge.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

const uint8_t ZERO = 0x80;
const uint8_t NEG = 0x40;
const uint8_t HALF = 0x20;
const uint8_t CARRY = 0x10;

const uint16_t DIV = 0xFF04;
const uint16_t TIMA = 0xFF05;
const uint16_t TMA = 0xFF06;
const uint16_t TAC = 0xFF07;

const uint16_t IE = 0xFFFF;
const uint16_t IF = 0xFF0F;

// Current flag
uint8_t CPU::ZERO_F() { return AF & ZERO; }
uint8_t CPU::SUB_F() { return AF & NEG; }
uint8_t CPU::HALF_F() { return AF & HALF; }
uint8_t CPU::CARRY_F() { return AF & CARRY; }


// Set flag
uint8_t CPU::ZERO_S(uint16_t n) {
    return (n == 0) ? ZERO : 0;
}

uint8_t CPU::HALF_S(uint8_t a, uint8_t b) {
    return ((((a & 0x0F) + (b & 0x0F)) & 0x10) == 0x10) ? HALF : 0;
}

uint8_t CPU::CARRY_S(uint8_t a, uint8_t b) {
    return (a+b > 0xFF) ? CARRY : 0;
}

// Set flag 16-bit
uint8_t CPU::HALF_Se(uint16_t a, uint16_t b) {
    return ((((a & 0x0FFF) + (b & 0x0FFF)) & 0x1000) == 0x1000) ? HALF : 0;
}

uint8_t CPU::CARRY_Se(uint16_t a, uint16_t b) {
    return (a+b > 0xFFFF) ? CARRY : 0;
}

// Set flag carry
uint8_t CPU::HALF_Sc(uint8_t a, uint8_t b) {
    return ((((a & 0x0F) + (b & 0x0F) + (CARRY_F() >> 4)) & 0x10) == 0x10) ? HALF : 0;
}

uint8_t CPU::CARRY_Sc(uint8_t a, uint8_t b) {
    return ((a+b + (CARRY_F() >> 4)) > 0xFF) ? CARRY : 0;
}

// Set flag borrow
uint8_t CPU::HALF_Sb(uint8_t a, uint8_t b) {
    return (((a & 0x0F) - (b & 0x0F)) < 0) ? HALF : 0;
}

uint8_t CPU::CARRY_Sb(uint8_t a, uint8_t b) {
    return (a-b < 0) ? CARRY : 0;
}

// Set flag borrow carry
uint8_t CPU::HALF_Sbc(uint8_t a, uint8_t b) {
    return (((a & 0x0F) - (b & 0x0F) - (CARRY_F() >> 4)) < 0) ? HALF : 0;
}

uint8_t CPU::CARRY_Sbc(uint8_t a, uint8_t b) {
    return ((a-b - (CARRY_F() >> 4)) < 0) ? CARRY : 0;
}

// Read memory
uint8_t CPU::readByte() {
    return memory.read(PC++);
}

uint16_t CPU::readb16() {
    uint8_t lower = readByte();
    uint8_t upper = readByte();
    return (upper << 8) | lower;
}

// Stack
void CPU::push(uint16_t r) {
    memory.write(--SP, r >> 8);
    memory.write(--SP, r & 0x00FF);
}

uint16_t CPU::pop() {
    uint8_t lower = memory.read(SP++);
    uint8_t upper = memory.read(SP++);
    return (upper << 8) | lower;
}

// 8-Bit Loads
// LD nn,n
uint16_t CPU::LD_Ab_n(uint16_t r, uint8_t n) {
    return ((r & 0x00FF) | (n << 8));
}
uint16_t CPU::LD_aB_n(uint16_t r, uint8_t n) {
    return ((r & 0xFF00) | n);
}

// LD r1,r2
uint16_t CPU::LD_Ab_Ab(uint16_t r1, uint16_t r2) {
    return ((r1 & 0x00FF) | (r2 & 0xFF00));
}
uint16_t CPU::LD_Ab_aB(uint16_t r1, uint16_t r2) {
    return ((r1 & 0x00FF) | ((r2 & 0x00FF) << 8));
}
uint16_t CPU::LD_aB_Ab(uint16_t r1, uint16_t r2) {
    return ((r1 & 0xFF00) | (r2 >> 8));
}
uint16_t CPU::LD_aB_aB(uint16_t r1, uint16_t r2) {
    return ((r1 & 0xFF00) | (r2 & 0x00FF));
}

// ADD
void CPU::ADD(uint8_t n) {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, A+n);
    AF = LD_aB_n(AF, ZERO_S(AF >> 8) | HALF_S(A, n) | CARRY_S(A, n));
}

// ADC
void CPU::ADC(uint8_t n) {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, A+n+(CARRY_F() >> 4));
    AF = LD_aB_n(AF, ZERO_S(AF >> 8) | HALF_Sc(A, n) | CARRY_Sc(A, n));
}

// SUB
void CPU::SUB(uint8_t n) {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, A-n);
    AF = LD_aB_n(AF, ZERO_S(AF >> 8) | NEG | HALF_Sb(A, n) | CARRY_Sb(A, n));
}

// SBC
void CPU::SBC(uint8_t n) {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, A-n-(CARRY_F() >> 4));
    AF = LD_aB_n(AF, ZERO_S(AF >> 8) | NEG | HALF_Sbc(A, n) | CARRY_Sbc(A, n));
}

// AND
void CPU::AND(uint8_t n) {
    AF = LD_Ab_n(AF, (AF >> 8) & n);
    AF = LD_aB_n(AF, ZERO_S(AF >> 8) | HALF);
}

// OR
void CPU::OR(uint8_t n) {
    AF = LD_Ab_n(AF, (AF >> 8) | n);
    AF = LD_aB_n(AF, ZERO_S(AF >> 8));
}

// XOR
void CPU::XOR(uint8_t n) {
    AF = LD_Ab_n(AF, (AF >> 8) ^ n);
    AF = LD_aB_n(AF, ZERO_S(AF >> 8));
}

// CP
void CPU::CP(uint8_t n) {
    uint8_t A = AF >> 8;
    AF = LD_aB_n(AF, ZERO_S(A-n) | NEG | (((A & 0x0F) < (n & 0x0F)) ? HALF : 0) | ((A < n) ? CARRY : 0));
}

// INC
uint8_t CPU::INC(uint8_t n) {
    n++;
    AF = LD_aB_n(AF, ZERO_S(n) | (((n & 0x0F) == 0) ? HALF : 0) | CARRY_F());
    return n;
}

// DEC
uint8_t CPU::DEC(uint8_t n) {
    AF = LD_aB_n(AF, ZERO_S(n-1) | NEG | (((n >> 4) > ((n-1) >> 4)) ? HALF : 0) | CARRY_F());
    n--;
    return n;
}

// ADD nn
void CPU::ADD_nn(uint16_t nn) {
    AF = LD_aB_n(AF, ZERO_F() | HALF_Se(HL, nn) | CARRY_Se(HL, nn));
    HL += nn;
}

// Rotate Shift Instructions
// Rotate A left
void CPU::RLCA() {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, (A << 1) | (A >> 7));
    AF = LD_aB_n(AF, (A >> 7) << 4);   
}

void CPU::RLA() {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, (A << 1) | (CARRY_F() >> 4));
    AF = LD_aB_n(AF, (A >> 7) << 4);
}


// Rotate A right
void CPU::RRCA() {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, (A >> 1) | (A << 7));
    AF = LD_aB_n(AF, (A & 0x1) << 4);
}

void CPU::RRA() {
    uint8_t A = AF >> 8;
    AF = LD_Ab_n(AF, (A >> 1) | (CARRY_F() << 3));
    AF = LD_aB_n(AF, (A & 0x1) << 4);
}

// Rotate left
uint8_t CPU::RLC(uint8_t n) {
    n = (n << 1) | (n >> 7);
    AF = LD_aB_n(AF, ZERO_S(n) | ((n & 0x1) << 4));
    return n;
}

uint8_t CPU::RL(uint8_t n) {
    uint8_t C = (n >> 7) << 4;
    n = (n << 1) | (CARRY_F() >> 4);
    AF = LD_aB_n(AF, ZERO_S(n) | C);
    return n;
}

// Rotate right
uint8_t CPU::RRC(uint8_t n) {
    n = (n >> 1) | (n << 7);
    AF = LD_aB_n(AF, ZERO_S(n) | ((n & 0x80) >> 3));
    return n;
}

uint8_t CPU::RR(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = (n >> 1) | (CARRY_F() << 3);
    AF = LD_aB_n(AF, ZERO_S(n) | C);
    return n;
}

// SLA
uint8_t CPU::SLA(uint8_t n) {
    uint8_t C = (n >> 7) << 4;
    n = n << 1;
    AF = LD_aB_n(AF, ZERO_S(n) | C);
    return n;
}

// SRA
uint8_t CPU::SRA(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = (n >> 1) | (n & 0x80);
    AF = LD_aB_n(AF, ZERO_S(n) | C);
    return n;
}

// SRL
uint8_t CPU::SRL(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = n >> 1;
    AF = LD_aB_n(AF, ZERO_S(n) | C);
    return n;
}

// SWAP
uint8_t CPU::SWAP(uint8_t n) {
    AF = LD_aB_n(AF, ZERO_S(n));
    return (n << 4) | (n >> 4);
}

// BIT
void CPU::BIT(uint8_t n, uint8_t b) {
    AF = LD_aB_n(AF, ZERO_S(n & (1 << b)) | HALF | CARRY_F());
}

// SET
uint8_t CPU::SET(uint8_t n, uint8_t b) {
    return n | (1 << b);
}

// RES
uint8_t CPU::RES(uint8_t n, uint8_t b) {
    return n & (~(1 << b));
}

// Jump Instructions
// JP
void CPU::JP(bool flag) {
    uint16_t nn = readb16();
    if (flag) {
        PC = nn;
        cycles++;
    }
}

// JR
void CPU::JR(bool flag) {
    int8_t e = readByte();
    if (flag) {
        PC += e;
        cycles++;
    }
}

void CPU::CALL(bool flag) {
    uint16_t nn = readb16();
    if (flag) {
        push(PC);
        PC = nn;
        cycles += 3;
    }
}

void CPU::RET(bool flag) {
    if (flag) {
        PC = pop();
        cycles += 3;
    }
}

void CPU::cycle() {

    uint8_t n;
    uint16_t nn;
    int8_t e;
    uint8_t interrupt;

    totalCycles += cycles;

    divider = totalCycles % 64;
    if (divider == 0 || divider < prevDivider) {
        memory.IO[0x4]++;
        prevDivider = totalCycles % 64;
    }

    if (memory.read(TAC) & 0x04) { // Timer Enable
        switch (memory.read(TAC) & 0x03) { // Input Clock Select
            case 0:
                timer = totalCycles % 256;
                break;

            case 1:
                timer = totalCycles % 4;
                break;

            case 2:
                timer = totalCycles % 16;
                break;

            case 3:
                timer = totalCycles % 64;
                break;
        }

        if (timer == 0 || timer < prevTimer) {
            memory.write(TIMA, memory.read(TIMA) + 1);

            if (memory.read(TIMA) == 0) { // If overflow
                memory.write(TIMA, memory.read(TMA)); // Reset TIMA
                memory.interrupt(TIMER);
            }
        }
        prevTimer = timer;
    }

    if (enableIRQ != 0 && --enableIRQ == 0) {
        IME = 1;
    }
    if (disableIRQ != 0 && --disableIRQ == 0) {
        IME = 0;
    }

    if (memory.joypad.interrupt) {
        memory.joypad.interrupt = false;
        memory.interrupt(0x10);
    }

    if (IME || halted) {
        interrupt = memory.read(IF) & memory.read(IE);
        // if (interrupt) {
        //     halted = 0;
        // }
        if (interrupt & V_BLANK) {
            // IME = 0;
            // if (halted) {
            //     halted = 0;
            // } else {
            if (IME == 1) {
                memory.write(IF, memory.read(IF) & ~V_BLANK);
                push(PC);
                PC = 0x40;
            }
            IME = 0;
            halted = 0;
            // }
        } else if (interrupt & 0x02) { // LCD STAT
            // IME = 0;
            if (IME == 1) {
                memory.write(IF, memory.read(IF) & ~0x02);
                push(PC);
                PC = 0x48;
            }
            IME = 0;
            halted = 0;
        } else if (interrupt & TIMER) {
            // IME = 0;
            // if (halted) {
            //     halted = 0;
            // } else {
            if (IME == 1) {
                memory.write(IF, memory.read(IF) & ~TIMER);
                push(PC);
                PC = 0x50;
            }
            IME = 0;
            halted = 0;
            // }
        } else if (interrupt & 0x10) { // Joypad
            // IME = 0;
            if (IME == 1) {
                memory.write(IF, memory.read(IF) & ~0x10);
                push(PC);
                PC = 0x60;
            }
            IME = 0;
            halted = 0;
        }
    }

    if (halted) {
        totalCycles++;
        cycles = 1;
        return;
    }

    opcode = readByte();

    if (opcode == 0xCB) {
        cycles = cbCycles[opcode];
    } else {
        cycles = opCycles[opcode];
    }

    switch (opcode) {
        // 8-Bit Transfer and Input/Output Instructions
        // LD A,r
        case 0x78: AF = LD_Ab_Ab(AF, BC); break;
        case 0x79: AF = LD_Ab_aB(AF, BC); break;
        case 0x7A: AF = LD_Ab_Ab(AF, DE); break;
        case 0x7B: AF = LD_Ab_aB(AF, DE); break;
        case 0x7C: AF = LD_Ab_Ab(AF, HL); break;
        case 0x7D: AF = LD_Ab_aB(AF, HL); break;
        case 0x7F: AF = LD_Ab_Ab(AF, AF); break;

        // LD B,r
        case 0x40: BC = LD_Ab_Ab(BC, BC); break;
        case 0x41: BC = LD_Ab_aB(BC, BC); break;
        case 0x42: BC = LD_Ab_Ab(BC, DE); break;
        case 0x43: BC = LD_Ab_aB(BC, DE); break;
        case 0x44: BC = LD_Ab_Ab(BC, HL); break;
        case 0x45: BC = LD_Ab_aB(BC, HL); break;
        case 0x47: BC = LD_Ab_Ab(BC, AF); break;

        // LD C,r
        case 0x48: BC = LD_aB_Ab(BC, BC); break;
        case 0x49: BC = LD_aB_aB(BC, BC); break;
        case 0x4A: BC = LD_aB_Ab(BC, DE); break;
        case 0x4B: BC = LD_aB_aB(BC, DE); break;
        case 0x4C: BC = LD_aB_Ab(BC, HL); break;
        case 0x4D: BC = LD_aB_aB(BC, HL); break;
        case 0x4F: BC = LD_aB_Ab(BC, AF); break;

        // LD D,r
        case 0x50: DE = LD_Ab_Ab(DE, BC); break;
        case 0x51: DE = LD_Ab_aB(DE, BC); break;
        case 0x52: DE = LD_Ab_Ab(DE, DE); break;
        case 0x53: DE = LD_Ab_aB(DE, DE); break;
        case 0x54: DE = LD_Ab_Ab(DE, HL); break;
        case 0x55: DE = LD_Ab_aB(DE, HL); break;
        case 0x57: DE = LD_Ab_Ab(DE, AF); break;

        // LD E,r
        case 0x58: DE = LD_aB_Ab(DE, BC); break;
        case 0x59: DE = LD_aB_aB(DE, BC); break;
        case 0x5A: DE = LD_aB_Ab(DE, DE); break;
        case 0x5B: DE = LD_aB_aB(DE, DE); break;
        case 0x5C: DE = LD_aB_Ab(DE, HL); break;
        case 0x5D: DE = LD_aB_aB(DE, HL); break;
        case 0x5F: DE = LD_aB_Ab(DE, AF); break;

        // LD H,r
        case 0x60: HL = LD_Ab_Ab(HL, BC); break;
        case 0x61: HL = LD_Ab_aB(HL, BC); break;
        case 0x62: HL = LD_Ab_Ab(HL, DE); break;
        case 0x63: HL = LD_Ab_aB(HL, DE); break;
        case 0x64: HL = LD_Ab_Ab(HL, HL); break;
        case 0x65: HL = LD_Ab_aB(HL, HL); break;
        case 0x67: HL = LD_Ab_Ab(HL, AF); break;
        
        // LD L,r
        case 0x68: HL = LD_aB_Ab(HL, BC); break;
        case 0x69: HL = LD_aB_aB(HL, BC); break;
        case 0x6A: HL = LD_aB_Ab(HL, DE); break;
        case 0x6B: HL = LD_aB_aB(HL, DE); break;
        case 0x6C: HL = LD_aB_Ab(HL, HL); break;
        case 0x6D: HL = LD_aB_aB(HL, HL); break;
        case 0x6F: HL = LD_aB_Ab(HL, AF); break;

        // LD r,n
        case 0x06: BC = LD_Ab_n(BC, readByte()); break;
        case 0x0E: BC = LD_aB_n(BC, readByte()); break;
        case 0x16: DE = LD_Ab_n(DE, readByte()); break;
        case 0x1E: DE = LD_aB_n(DE, readByte()); break;
        case 0x26: HL = LD_Ab_n(HL, readByte()); break;
        case 0x2E: HL = LD_aB_n(HL, readByte()); break;
        case 0x3E: AF = LD_Ab_n(AF, readByte()); break;

        // LD r,(HL)
        case 0x46: BC = LD_Ab_n(BC, memory.read(HL)); break;
        case 0x4E: BC = LD_aB_n(BC, memory.read(HL)); break;
        case 0x56: DE = LD_Ab_n(DE, memory.read(HL)); break;
        case 0x5E: DE = LD_aB_n(DE, memory.read(HL)); break;
        case 0x66: HL = LD_Ab_n(HL, memory.read(HL)); break;
        case 0x6E: HL = LD_aB_n(HL, memory.read(HL)); break;
        case 0x7E: AF = LD_Ab_n(AF, memory.read(HL)); break;

        // LD (HL),r
        case 0x70: memory.write(HL, BC >> 8); break;
        case 0x71: memory.write(HL, BC & 0x00FF); break;
        case 0x72: memory.write(HL, DE >> 8); break;
        case 0x73: memory.write(HL, DE & 0x00FF); break;
        case 0x74: memory.write(HL, HL >> 8); break;
        case 0x75: memory.write(HL, HL & 0x00FF); break;
        case 0x77: memory.write(HL, AF >> 8); break;

        // LD (HL),n
        case 0x36: memory.write(HL, readByte()); break;

        // LD A,(BC)
        case 0x0A: AF = LD_Ab_n(AF, memory.read(BC)); break;

        // LD A,(DE)
        case 0x1A: AF = LD_Ab_n(AF, memory.read(DE)); break;
            
        // LD A,(C)
        case 0xF2: AF = LD_Ab_n(AF, memory.read(0xFF00 | BC)); break;

        // LD (C),A
        case 0xE2: memory.write(0xFF00 | BC, AF >> 8); break;

        // LDH A,(n)
        case 0xF0: AF = LD_Ab_n(AF, memory.read(0xFF00 | readByte())); break;

        // LDH (n),A
        case 0xE0: memory.write(0xFF00 | readByte(), AF >> 8); break;

        // LD A,(nn)
        case 0xFA: AF = LD_Ab_n(AF, memory.read(readb16())); break;

        // LD (nn),A
        case 0xEA: memory.write(readb16(), AF >> 8); break;

        // LD A,(HLI)
        case 0x2A: AF = LD_Ab_n(AF, memory.read(HL++)); break;

        // LD A,(HLD)
        case 0x3A: AF = LD_Ab_n(AF, memory.read(HL--)); break;

        // LD (BC),A
        case 0x02: memory.write(BC, AF >> 8); break;

        // LD (DE),A
        case 0x12: memory.write(DE, AF >> 8); break;

        // LD (HLI),A
        case 0x22: memory.write(HL++, AF >> 8); break;

        // LD (HLD),A
        case 0x32: memory.write(HL--, AF >> 8); break;

        // 16-Bit Transfer Instructions
        // LD dd,nn
        case 0x01: BC = readb16(); break;
        case 0x11: DE = readb16(); break;
        case 0x21: HL = readb16(); break;
        case 0x31: SP = readb16(); break;

        // LD SP,HL
        case 0xF9: SP = HL; break;

        // PUSH nn
        case 0xC5: push(BC); break;
        case 0xD5: push(DE); break;
        case 0xE5: push(HL); break;
        case 0xF5: push(AF); break;

        // POP nn
        case 0xC1: BC = pop(); break;
        case 0xD1: DE = pop(); break;
        case 0xE1: HL = pop(); break;
        case 0xF1: AF = pop() & 0xFFF0; break;

        // LDHL SP,e
        case 0xF8:
            e = readByte();
            HL = SP + e;
            AF = LD_aB_n(AF, HALF_S(SP, e) | CARRY_S(SP, e));
            break;

        // LD (nn),SP
        case 0x08:
            nn = readb16();
            memory.write(nn, SP & 0x00FF);
            memory.write(nn+1, SP >> 8);
            break;

        // 8-Bit Arithmetic and Logical Operation Instructions
        // ADD A,r
        case 0x80: ADD(BC >> 8); break;
        case 0x81: ADD(BC & 0x00FF); break;
        case 0x82: ADD(DE >> 8); break;
        case 0x83: ADD(DE & 0x00FF); break;
        case 0x84: ADD(HL >> 8); break;
        case 0x85: ADD(HL & 0x00FF); break;
        case 0x87: ADD(AF >> 8); break;

        // ADD A,n
        case 0xC6: ADD(readByte()); break;

        // ADD A,(HL)
        case 0x86: ADD(memory.read(HL)); break;

        // ADC A,r
        case 0x88: ADC(BC >> 8); break;
        case 0x89: ADC(BC & 0x00FF); break;
        case 0x8A: ADC(DE >> 8); break;
        case 0x8B: ADC(DE & 0x00FF); break;
        case 0x8C: ADC(HL >> 8); break;
        case 0x8D: ADC(HL & 0x00FF); break;
        case 0x8F: ADC(AF >> 8); break;

        // ADC A,n
        case 0xCE: ADC(readByte()); break;

        // ADC A,(HL)
        case 0x8E: ADC(memory.read(HL)); break;

        // SUB r
        case 0x90: SUB(BC >> 8); break;
        case 0x91: SUB(BC & 0x00FF); break;
        case 0x92: SUB(DE >> 8); break;
        case 0x93: SUB(DE & 0x00FF); break;
        case 0x94: SUB(HL >> 8); break;
        case 0x95: SUB(HL & 0x00FF); break;
        case 0x97: SUB(AF >> 8); break;

        // SUB n
        case 0xD6: SUB(readByte()); break;

        // SUB (HL)
        case 0x96: SUB(memory.read(HL)); break;

        // SBC r
        case 0x98: SBC(BC >> 8); break;
        case 0x99: SBC(BC & 0x00FF); break;
        case 0x9A: SBC(DE >> 8); break;
        case 0x9B: SBC(DE & 0x00FF); break;
        case 0x9C: SBC(HL >> 8); break;
        case 0x9D: SBC(HL & 0x00FF); break;
        case 0x9F: SBC(AF >> 8); break;

        // SBC n
        case 0xDE: SBC(readByte()); break;

        // SBC (HL)
        case 0x9E: SBC(memory.read(HL)); break;

        // AND r
        case 0xA0: AND(BC >> 8); break;
        case 0xA1: AND(BC & 0x00FF); break;
        case 0xA2: AND(DE >> 8); break;
        case 0xA3: AND(DE & 0x00FF); break;
        case 0xA4: AND(HL >> 8); break;
        case 0xA5: AND(HL & 0x00FF); break;
        case 0xA7: AND(AF >> 8); break;

        // AND n
        case 0xE6: AND(readByte()); break;

        // AND (HL)
        case 0xA6: AND(memory.read(HL)); break;

        // OR r
        case 0xB0: OR(BC >> 8); break;
        case 0xB1: OR(BC & 0x00FF); break;
        case 0xB2: OR(DE >> 8); break;
        case 0xB3: OR(DE & 0x00FF); break;
        case 0xB4: OR(HL >> 8); break;
        case 0xB5: OR(HL & 0x00FF); break;
        case 0xB7: OR(AF >> 8); break;

        // OR n
        case 0xF6: OR(readByte()); break;

        // OR (HL)
        case 0xB6: OR(memory.read(HL)); break;

        // XOR r
        case 0xA8: XOR(BC >> 8); break;
        case 0xA9: XOR(BC & 0x00FF); break;
        case 0xAA: XOR(DE >> 8); break;
        case 0xAB: XOR(DE & 0x00FF); break;
        case 0xAC: XOR(HL >> 8); break;
        case 0xAD: XOR(HL & 0x00FF); break;
        case 0xAF: XOR(AF >> 8); break;

        // XOR n
        case 0xEE: XOR(readByte()); break;

        // XOR (HL)
        case 0xAE: XOR(memory.read(HL)); break;

        // CP r
        case 0xB8: CP(BC >> 8); break;
        case 0xB9: CP(BC & 0x00FF); break;
        case 0xBA: CP(DE >> 8); break;
        case 0xBB: CP(DE & 0x00FF); break;
        case 0xBC: CP(HL >> 8); break;
        case 0xBD: CP(HL & 0x00FF); break;
        case 0xBF: CP(AF >> 8); break;

        // CP n
        case 0xFE: CP(readByte()); break;

        // CP (HL)
        case 0xBE: CP(memory.read(HL)); break;

        // INC,r
        case 0x04:; BC = LD_Ab_n(BC, INC(BC >> 8)); break;
        case 0x0C: BC = LD_aB_n(BC, INC(BC & 0x00FF)); break;
        case 0x14: DE = LD_Ab_n(DE, INC(DE >> 8)); break;
        case 0x1C: DE = LD_aB_n(DE, INC(DE & 0x00FF)); break;
        case 0x24: HL = LD_Ab_n(HL, INC(HL >> 8)); break;
        case 0x2C: HL = LD_aB_n(HL, INC(HL & 0x00FF)); break;
        case 0x3C: AF = LD_Ab_n(AF, INC(AF >> 8)); break;

        // INC,(HL)
        case 0x34: memory.write(HL, INC(memory.read(HL))); break;

        // DEC,r
        case 0x05: BC = LD_Ab_n(BC, DEC(BC >> 8)); break;
        case 0x0D: BC = LD_aB_n(BC, DEC(BC & 0x00FF)); break;
        case 0x15: DE = LD_Ab_n(DE, DEC(DE >> 8)); break;
        case 0x1D: DE = LD_aB_n(DE, DEC(DE & 0x00FF)); break;
        case 0x25: HL = LD_Ab_n(HL, DEC(HL >> 8)); break;
        case 0x2D: HL = LD_aB_n(HL, DEC(HL & 0x00FF)); break;
        case 0x3D: AF = LD_Ab_n(AF, DEC(AF >> 8)); break;

        // DEC,(HL)
        case 0x35: memory.write(HL, DEC(memory.read(HL))); break;

        // 16-Bit Arithmetic Operation Instructions
        // ADD HL,BC
        case 0x09: ADD_nn(BC); break;

        // ADD HL,DE
        case 0x19: ADD_nn(DE); break;

        // ADD HL,HL
        case 0x29: ADD_nn(HL); break;

        // ADD HL,SP
        case 0x39: ADD_nn(SP); break;

        // ADD SP,e
        case 0xE8:
            e = readByte();
            AF = LD_aB_n(AF, HALF_S(SP, e) | CARRY_S(SP, e));
            SP += e;
            break;

        // INC BC
        case 0x03: BC++; break;

        // INC DE
        case 0x13: DE++; break;
        
        // INC HL
        case 0x23: HL++; break;

        // INC SP
        case 0x33: SP++; break;

        // DEC BC
        case 0x0B: BC--; break;

        // DEC DE
        case 0x1B: DE--; break;

        // DEC HL
        case 0x2B: HL--; break;

        // DEC SP
        case 0x3B: SP--; break;

        // Rotate Shift Instructions
        // RLCA
        case 0x07: RLCA(); break;

        // RLA
        case 0x17: RLA(); break;

        // RRCA
        case 0x0F: RRCA(); break;

        // RRA
        case 0x1F: RRA(); break;

        // CB Prefix
        case 0xCB:
            switch (readByte()) {
                // Rotate Shift Instructions
                // RLC
                case 0x00: BC = LD_Ab_n(BC, RLC(BC >> 8)); break;
                case 0x01: BC = LD_aB_n(BC, RLC(BC & 0x00FF)); break;
                case 0x02: DE = LD_Ab_n(DE, RLC(DE >> 8)); break;
                case 0x03: DE = LD_aB_n(DE, RLC(DE & 0x00FF)); break;
                case 0x04: HL = LD_Ab_n(HL, RLC(HL >> 8)); break;
                case 0x05: HL = LD_aB_n(HL, RLC(HL & 0x00FF)); break;
                case 0x06: memory.write(HL, RLC(memory.read(HL))); break;
                case 0x07: AF = LD_Ab_n(AF, RLC(AF >> 8)); break;

                // RL
                case 0x10: BC = LD_Ab_n(BC, RL(BC >> 8)); break;
                case 0x11: BC = LD_aB_n(BC, RL(BC & 0x00FF)); break;
                case 0x12: DE = LD_Ab_n(DE, RL(DE >> 8)); break;
                case 0x13: DE = LD_aB_n(DE, RL(DE & 0x00FF)); break;
                case 0x14: HL = LD_Ab_n(HL, RL(HL >> 8)); break;
                case 0x15: HL = LD_aB_n(HL, RL(HL & 0x00FF)); break;
                case 0x16: memory.write(HL, RL(memory.read(HL))); break;
                case 0x17: AF = LD_Ab_n(AF, RL(AF >> 8)); break;

                // RRC
                case 0x08: BC = LD_Ab_n(BC, RRC(BC >> 8)); break;
                case 0x09: BC = LD_aB_n(BC, RRC(BC & 0x00FF)); break;
                case 0x0A: DE = LD_Ab_n(DE, RRC(DE >> 8)); break;
                case 0x0B: DE = LD_aB_n(DE, RRC(DE & 0x00FF)); break;
                case 0x0C: HL = LD_Ab_n(HL, RRC(HL >> 8)); break;
                case 0x0D: HL = LD_aB_n(HL, RRC(HL & 0x00FF)); break;
                case 0x0E: memory.write(HL, RRC(memory.read(HL))); break;
                case 0x0F: AF = LD_Ab_n(AF, RRC(AF >> 8)); break;

                // RR
                case 0x18: BC = LD_Ab_n(BC, RR(BC >> 8)); break;
                case 0x19: BC = LD_aB_n(BC, RR(BC & 0x00FF)); break;
                case 0x1A: DE = LD_Ab_n(DE, RR(DE >> 8)); break;
                case 0x1B: DE = LD_aB_n(DE, RR(DE & 0x00FF)); break;
                case 0x1C: HL = LD_Ab_n(HL, RR(HL >> 8)); break;
                case 0x1D: HL = LD_aB_n(HL, RR(HL & 0x00FF)); break;
                case 0x1E: memory.write(HL, RR(memory.read(HL))); break;
                case 0x1F: AF = LD_Ab_n(AF, RR(AF >> 8)); break;

                // SLA
                case 0x20: BC = LD_Ab_n(BC, SLA(BC >> 8)); break;
                case 0x21: BC = LD_aB_n(BC, SLA(BC & 0x00FF)); break;
                case 0x22: DE = LD_Ab_n(DE, SLA(DE >> 8)); break;
                case 0x23: DE = LD_aB_n(DE, SLA(DE & 0x00FF)); break;
                case 0x24: HL = LD_Ab_n(HL, SLA(HL >> 8)); break;
                case 0x25: HL = LD_aB_n(HL, SLA(HL & 0x00FF)); break;
                case 0x26: memory.write(HL, SLA(memory.read(HL))); break;
                case 0x27: AF = LD_Ab_n(AF, SLA(AF >> 8)); break;

                // SRA
                case 0x28: BC = LD_Ab_n(BC, SRA(BC >> 8)); break;
                case 0x29: BC = LD_aB_n(BC, SRA(BC & 0x00FF)); break;
                case 0x2A: DE = LD_Ab_n(DE, SRA(DE >> 8)); break;
                case 0x2B: DE = LD_aB_n(DE, SRA(DE & 0x00FF)); break;
                case 0x2C: HL = LD_Ab_n(HL, SRA(HL >> 8)); break;
                case 0x2D: HL = LD_aB_n(HL, SRA(HL & 0x00FF)); break;
                case 0x2E: memory.write(HL, SRA(memory.read(HL))); break;
                case 0x2F: AF = LD_Ab_n(AF, SRA(AF >> 8)); break;

                // SRL
                case 0x38: BC = LD_Ab_n(BC, SRL(BC >> 8)); break;
                case 0x39: BC = LD_aB_n(BC, SRL(BC & 0x00FF)); break;
                case 0x3A: DE = LD_Ab_n(DE, SRL(DE >> 8)); break;
                case 0x3B: DE = LD_aB_n(DE, SRL(DE & 0x00FF)); break;
                case 0x3C: HL = LD_Ab_n(HL, SRL(HL >> 8)); break;
                case 0x3D: HL = LD_aB_n(HL, SRL(HL & 0x00FF)); break;
                case 0x3E: memory.write(HL, SRL(memory.read(HL))); break;
                case 0x3F: AF = LD_Ab_n(AF, SRL(AF >> 8)); break;

                // SWAP
                case 0x30: BC = LD_Ab_n(BC, SWAP(BC >> 8)); break;
                case 0x31: BC = LD_aB_n(BC, SWAP(BC & 0x00FF)); break;
                case 0x32: DE = LD_Ab_n(DE, SWAP(DE >> 8)); break;
                case 0x33: DE = LD_aB_n(DE, SWAP(DE & 0x00FF)); break;
                case 0x34: HL = LD_Ab_n(HL, SWAP(HL >> 8)); break;
                case 0x35: HL = LD_aB_n(HL, SWAP(HL & 0x00FF)); break;
                case 0x36: memory.write(HL, SWAP(memory.read(HL))); break;
                case 0x37: AF = LD_Ab_n(AF, SWAP(AF >> 8)); break;

                // Bit Operations
                // BIT 0,r
                case 0x40: BIT(BC >> 8, 0); break;
                case 0x41: BIT(BC & 0x00FF, 0); break;
                case 0x42: BIT(DE >> 8, 0); break;
                case 0x43: BIT(DE & 0x00FF, 0); break;
                case 0x44: BIT(HL >> 8, 0); break;
                case 0x45: BIT(HL & 0x00FF, 0); break;
                case 0x46: BIT(memory.read(HL), 0); break;
                case 0x47: BIT(AF >> 8, 0); break;

                // BIT 1,r
                case 0x48: BIT(BC >> 8, 1); break;
                case 0x49: BIT(BC & 0x00FF, 1); break;
                case 0x4A: BIT(DE >> 8, 1); break;
                case 0x4B: BIT(DE & 0x00FF, 1); break;
                case 0x4C: BIT(HL >> 8, 1); break;
                case 0x4D: BIT(HL & 0x00FF, 1); break;
                case 0x4E: BIT(memory.read(HL), 1); break;
                case 0x4F: BIT(AF >> 8, 1); break;

                // BIT 2,r
                case 0x50: BIT(BC >> 8, 2); break;
                case 0x51: BIT(BC & 0x00FF, 2); break;
                case 0x52: BIT(DE >> 8, 2); break;
                case 0x53: BIT(DE & 0x00FF, 2); break;
                case 0x54: BIT(HL >> 8, 2); break;
                case 0x55: BIT(HL & 0x00FF, 2); break;
                case 0x56: BIT(memory.read(HL), 2); break;
                case 0x57: BIT(AF >> 8, 2); break;

                // BIT 3,r
                case 0x58: BIT(BC >> 8, 3); break;
                case 0x59: BIT(BC & 0x00FF, 3); break;
                case 0x5A: BIT(DE >> 8, 3); break;
                case 0x5B: BIT(DE & 0x00FF, 3); break;
                case 0x5C: BIT(HL >> 8, 3); break;
                case 0x5D: BIT(HL & 0x00FF, 3); break;
                case 0x5E: BIT(memory.read(HL), 3); break;
                case 0x5F: BIT(AF >> 8, 3); break;

                // BIT 4,r
                case 0x60: BIT(BC >> 8, 4); break;
                case 0x61: BIT(BC & 0x00FF, 4); break;
                case 0x62: BIT(DE >> 8, 4); break;
                case 0x63: BIT(DE & 0x00FF, 4); break;
                case 0x64: BIT(HL >> 8, 4); break;
                case 0x65: BIT(HL & 0x00FF, 4); break;
                case 0x66: BIT(memory.read(HL), 4); break;
                case 0x67: BIT(AF >> 8, 4); break;

                // BIT 5,r
                case 0x68: BIT(BC >> 8, 5); break;
                case 0x69: BIT(BC & 0x00FF, 5); break;
                case 0x6A: BIT(DE >> 8, 5); break;
                case 0x6B: BIT(DE & 0x00FF, 5); break;
                case 0x6C: BIT(HL >> 8, 5); break;
                case 0x6D: BIT(HL & 0x00FF, 5); break;
                case 0x6E: BIT(memory.read(HL), 5); break;
                case 0x6F: BIT(AF >> 8, 5); break;

                // BIT 6,r
                case 0x70: BIT(BC >> 8, 6); break;
                case 0x71: BIT(BC & 0x00FF, 6); break;
                case 0x72: BIT(DE >> 8, 6); break;
                case 0x73: BIT(DE & 0x00FF, 6); break;
                case 0x74: BIT(HL >> 8, 6); break;
                case 0x75: BIT(HL & 0x00FF, 6); break;
                case 0x76: BIT(memory.read(HL), 6); break;
                case 0x77: BIT(AF >> 8, 6); break;

                // BIT 7,r
                case 0x78: BIT(BC >> 8, 7); break;
                case 0x79: BIT(BC & 0x00FF, 7); break;
                case 0x7A: BIT(DE >> 8, 7); break;
                case 0x7B: BIT(DE & 0x00FF, 7); break;
                case 0x7C: BIT(HL >> 8, 7); break;
                case 0x7D: BIT(HL & 0x00FF, 7); break;
                case 0x7E: BIT(memory.read(HL), 7); break;
                case 0x7F: BIT(AF >> 8, 7); break;

                // RES 0,r
                case 0x80: BC = LD_Ab_n(BC, RES(BC >> 8, 0)); break;
                case 0x81: BC = LD_aB_n(BC, RES(BC & 0x00FF, 0)); break;
                case 0x82: DE = LD_Ab_n(DE, RES(DE >> 8, 0)); break;
                case 0x83: DE = LD_aB_n(DE, RES(DE & 0x00FF, 0)); break;
                case 0x84: HL = LD_Ab_n(HL, RES(HL >> 8, 0)); break;
                case 0x85: HL = LD_aB_n(HL, RES(HL & 0x00FF, 0)); break;
                case 0x86: memory.write(HL, RES(memory.read(HL), 0)); break;
                case 0x87: AF = LD_Ab_n(AF, RES(AF >> 8, 0)); break;

                // RES 1,r
                case 0x88: BC = LD_Ab_n(BC, RES(BC >> 8, 1)); break;
                case 0x89: BC = LD_aB_n(BC, RES(BC & 0x00FF, 1)); break;
                case 0x8A: DE = LD_Ab_n(DE, RES(DE >> 8, 1)); break;
                case 0x8B: DE = LD_aB_n(DE, RES(DE & 0x00FF, 1)); break;
                case 0x8C: HL = LD_Ab_n(HL, RES(HL >> 8, 1)); break;
                case 0x8D: HL = LD_aB_n(HL, RES(HL & 0x00FF, 1)); break;
                case 0x8E: memory.write(HL, RES(memory.read(HL), 1)); break;
                case 0x8F: AF = LD_Ab_n(AF, RES(AF >> 8, 1)); break;

                // RES 2,r
                case 0x90: BC = LD_Ab_n(BC, RES(BC >> 8, 2)); break;
                case 0x91: BC = LD_aB_n(BC, RES(BC & 0x00FF, 2)); break;
                case 0x92: DE = LD_Ab_n(DE, RES(DE >> 8, 2)); break;
                case 0x93: DE = LD_aB_n(DE, RES(DE & 0x00FF, 2)); break;
                case 0x94: HL = LD_Ab_n(HL, RES(HL >> 8, 2)); break;
                case 0x95: HL = LD_aB_n(HL, RES(HL & 0x00FF, 2)); break;
                case 0x96: memory.write(HL, RES(memory.read(HL), 2)); break;
                case 0x97: AF = LD_Ab_n(AF, RES(AF >> 8, 2)); break;

                // RES 3,r
                case 0x98: BC = LD_Ab_n(BC, RES(BC >> 8, 3)); break;
                case 0x99: BC = LD_aB_n(BC, RES(BC & 0x00FF, 3)); break;
                case 0x9A: DE = LD_Ab_n(DE, RES(DE >> 8, 3)); break;
                case 0x9B: DE = LD_aB_n(DE, RES(DE & 0x00FF, 3)); break;
                case 0x9C: HL = LD_Ab_n(HL, RES(HL >> 8, 3)); break;
                case 0x9D: HL = LD_aB_n(HL, RES(HL & 0x00FF, 3)); break;
                case 0x9E: memory.write(HL, RES(memory.read(HL), 3)); break;
                case 0x9F: AF = LD_Ab_n(AF, RES(AF >> 8, 3)); break;

                // RES 4,r
                case 0xA0: BC = LD_Ab_n(BC, RES(BC >> 8, 4)); break;
                case 0xA1: BC = LD_aB_n(BC, RES(BC & 0x00FF, 4)); break;
                case 0xA2: DE = LD_Ab_n(DE, RES(DE >> 8, 4)); break;
                case 0xA3: DE = LD_aB_n(DE, RES(DE & 0x00FF, 4)); break;
                case 0xA4: HL = LD_Ab_n(HL, RES(HL >> 8, 4)); break;
                case 0xA5: HL = LD_aB_n(HL, RES(HL & 0x00FF, 4)); break;
                case 0xA6: memory.write(HL, RES(memory.read(HL), 4)); break;
                case 0xA7: AF = LD_Ab_n(AF, RES(AF >> 8, 4)); break;

                // RES 5,r
                case 0xA8: BC = LD_Ab_n(BC, RES(BC >> 8, 5)); break;
                case 0xA9: BC = LD_aB_n(BC, RES(BC & 0x00FF, 5)); break;
                case 0xAA: DE = LD_Ab_n(DE, RES(DE >> 8, 5)); break;
                case 0xAB: DE = LD_aB_n(DE, RES(DE & 0x00FF, 5)); break;
                case 0xAC: HL = LD_Ab_n(HL, RES(HL >> 8, 5)); break;
                case 0xAD: HL = LD_aB_n(HL, RES(HL & 0x00FF, 5)); break;
                case 0xAE: memory.write(HL, RES(memory.read(HL), 5)); break;
                case 0xAF: AF = LD_Ab_n(AF, RES(AF >> 8, 5)); break;

                // RES 6,r
                case 0xB0: BC = LD_Ab_n(BC, RES(BC >> 8, 6)); break;
                case 0xB1: BC = LD_aB_n(BC, RES(BC & 0x00FF, 6)); break;
                case 0xB2: DE = LD_Ab_n(DE, RES(DE >> 8, 6)); break;
                case 0xB3: DE = LD_aB_n(DE, RES(DE & 0x00FF, 6)); break;
                case 0xB4: HL = LD_Ab_n(HL, RES(HL >> 8, 6)); break;
                case 0xB5: HL = LD_aB_n(HL, RES(HL & 0x00FF, 6)); break;
                case 0xB6: memory.write(HL, RES(memory.read(HL), 6)); break;
                case 0xB7: AF = LD_Ab_n(AF, RES(AF >> 8, 6)); break;

                // RES 7,r
                case 0xB8: BC = LD_Ab_n(BC, RES(BC >> 8, 7)); break;
                case 0xB9: BC = LD_aB_n(BC, RES(BC & 0x00FF, 7)); break;
                case 0xBA: DE = LD_Ab_n(DE, RES(DE >> 8, 7)); break;
                case 0xBB: DE = LD_aB_n(DE, RES(DE & 0x00FF, 7)); break;
                case 0xBC: HL = LD_Ab_n(HL, RES(HL >> 8, 7)); break;
                case 0xBD: HL = LD_aB_n(HL, RES(HL & 0x00FF, 7)); break;
                case 0xBE: memory.write(HL, RES(memory.read(HL), 7)); break;
                case 0xBF: AF = LD_Ab_n(AF, RES(AF >> 8, 7)); break;

                // SET 0,r
                case 0xC0: BC = LD_Ab_n(BC, SET(BC >> 8, 0)); break;
                case 0xC1: BC = LD_aB_n(BC, SET(BC & 0x00FF, 0)); break;
                case 0xC2: DE = LD_Ab_n(DE, SET(DE >> 8, 0)); break;
                case 0xC3: DE = LD_aB_n(DE, SET(DE & 0x00FF, 0)); break;
                case 0xC4: HL = LD_Ab_n(HL, SET(HL >> 8, 0)); break;
                case 0xC5: HL = LD_aB_n(HL, SET(HL & 0x00FF, 0)); break;
                case 0xC6: memory.write(HL, SET(memory.read(HL), 0)); break;
                case 0xC7: AF = LD_Ab_n(AF, SET(AF >> 8, 0)); break;

                // SET 1,r
                case 0xC8: BC = LD_Ab_n(BC, SET(BC >> 8, 1)); break;
                case 0xC9: BC = LD_aB_n(BC, SET(BC & 0x00FF, 1)); break;
                case 0xCA: DE = LD_Ab_n(DE, SET(DE >> 8, 1)); break;
                case 0xCB: DE = LD_aB_n(DE, SET(DE & 0x00FF, 1)); break;
                case 0xCC: HL = LD_Ab_n(HL, SET(HL >> 8, 1)); break;
                case 0xCD: HL = LD_aB_n(HL, SET(HL & 0x00FF, 1)); break;
                case 0xCE: memory.write(HL, SET(memory.read(HL), 1)); break;
                case 0xCF: AF = LD_Ab_n(AF, SET(AF >> 8, 1)); break;

                // SET 2,r
                case 0xD0: BC = LD_Ab_n(BC, SET(BC >> 8, 2)); break;
                case 0xD1: BC = LD_aB_n(BC, SET(BC & 0x00FF, 2)); break;
                case 0xD2: DE = LD_Ab_n(DE, SET(DE >> 8, 2)); break;
                case 0xD3: DE = LD_aB_n(DE, SET(DE & 0x00FF, 2)); break;
                case 0xD4: HL = LD_Ab_n(HL, SET(HL >> 8, 2)); break;
                case 0xD5: HL = LD_aB_n(HL, SET(HL & 0x00FF, 2)); break;
                case 0xD6: memory.write(HL, SET(memory.read(HL), 2)); break;
                case 0xD7: AF = LD_Ab_n(AF, SET(AF >> 8, 2)); break;

                // SET 3,r
                case 0xD8: BC = LD_Ab_n(BC, SET(BC >> 8, 3)); break;
                case 0xD9: BC = LD_aB_n(BC, SET(BC & 0x00FF, 3)); break;
                case 0xDA: DE = LD_Ab_n(DE, SET(DE >> 8, 3)); break;
                case 0xDB: DE = LD_aB_n(DE, SET(DE & 0x00FF, 3)); break;
                case 0xDC: HL = LD_Ab_n(HL, SET(HL >> 8, 3)); break;
                case 0xDD: HL = LD_aB_n(HL, SET(HL & 0x00FF, 3)); break;
                case 0xDE: memory.write(HL, SET(memory.read(HL), 3)); break;
                case 0xDF: AF = LD_Ab_n(AF, SET(AF >> 8, 3)); break;

                // SET 4,r
                case 0xE0: BC = LD_Ab_n(BC, SET(BC >> 8, 4)); break;
                case 0xE1: BC = LD_aB_n(BC, SET(BC & 0x00FF, 4)); break;
                case 0xE2: DE = LD_Ab_n(DE, SET(DE >> 8, 4)); break;
                case 0xE3: DE = LD_aB_n(DE, SET(DE & 0x00FF, 4)); break;
                case 0xE4: HL = LD_Ab_n(HL, SET(HL >> 8, 4)); break;
                case 0xE5: HL = LD_aB_n(HL, SET(HL & 0x00FF, 4)); break;
                case 0xE6: memory.write(HL, SET(memory.read(HL), 4)); break;
                case 0xE7: AF = LD_Ab_n(AF, SET(AF >> 8, 4)); break;

                // SET 5,r
                case 0xE8: BC = LD_Ab_n(BC, SET(BC >> 8, 5)); break;
                case 0xE9: BC = LD_aB_n(BC, SET(BC & 0x00FF, 5)); break;
                case 0xEA: DE = LD_Ab_n(DE, SET(DE >> 8, 5)); break;
                case 0xEB: DE = LD_aB_n(DE, SET(DE & 0x00FF, 5)); break;
                case 0xEC: HL = LD_Ab_n(HL, SET(HL >> 8, 5)); break;
                case 0xED: HL = LD_aB_n(HL, SET(HL & 0x00FF, 5)); break;
                case 0xEE: memory.write(HL, SET(memory.read(HL), 5)); break;
                case 0xEF: AF = LD_Ab_n(AF, SET(AF >> 8, 5)); break;

                // SET 6,r
                case 0xF0: BC = LD_Ab_n(BC, SET(BC >> 8, 6)); break;
                case 0xF1: BC = LD_aB_n(BC, SET(BC & 0x00FF, 6)); break;
                case 0xF2: DE = LD_Ab_n(DE, SET(DE >> 8, 6)); break;
                case 0xF3: DE = LD_aB_n(DE, SET(DE & 0x00FF, 6)); break;
                case 0xF4: HL = LD_Ab_n(HL, SET(HL >> 8, 6)); break;
                case 0xF5: HL = LD_aB_n(HL, SET(HL & 0x00FF, 6)); break;
                case 0xF6: memory.write(HL, SET(memory.read(HL), 6)); break;
                case 0xF7: AF = LD_Ab_n(AF, SET(AF >> 8, 6)); break;

                // SET 7,r
                case 0xF8: BC = LD_Ab_n(BC, SET(BC >> 8, 7)); break;
                case 0xF9: BC = LD_aB_n(BC, SET(BC & 0x00FF, 7)); break;
                case 0xFA: DE = LD_Ab_n(DE, SET(DE >> 8, 7)); break;
                case 0xFB: DE = LD_aB_n(DE, SET(DE & 0x00FF, 7)); break;
                case 0xFC: HL = LD_Ab_n(HL, SET(HL >> 8, 7)); break;
                case 0xFD: HL = LD_aB_n(HL, SET(HL & 0x00FF, 7)); break;
                case 0xFE: memory.write(HL, SET(memory.read(HL), 7)); break;
                case 0xFF: AF = LD_Ab_n(AF, SET(AF >> 8, 7)); break;

                default: break;
            }
            break;

        // Jump Instructions
        // JP nn
        case 0xC3: PC = readb16(); break;

        // JP NZ,nn
        case 0xC2: JP(!ZERO_F()); break;

        // JP Z,nn
        case 0xCA: JP(ZERO_F()); break;

        // JP NC,nn
        case 0xD2: JP(!CARRY_F()); break;

        // JP C,nn
        case 0xDA: JP(CARRY_F()); break;

        // JR e
        case 0x18:
            e = readByte();
            PC += e;
            break;

        // JR cc,e
        // JR NZ,e
        case 0x20: JR(!ZERO_F()); break;

        // JR Z,e
        case 0x28: JR(ZERO_F()); break;

        // JR NC,e
        case 0x30: JR(!CARRY_F()); break;

        // JR C,e
        case 0x38: JR(CARRY_F()); break;

        // JP HL
        case 0xE9: PC = HL; break;

        // Call and Return Instructions
        // CALL nn
        case 0xCD: nn = readb16(); push(PC); PC = nn; break;

        // CALL NZ,nn
        case 0xC4: CALL(!ZERO_F()); break;

        // CALL Z,nn
        case 0xCC: CALL(ZERO_F()); break;

        // CALL NC,nn
        case 0xD4: CALL(!CARRY_F()); break;

        // CALL C,nn
        case 0xDC: CALL(CARRY_F()); break;

        // RET
        case 0xC9: PC = pop(); break;

        // RETI
        case 0xD9: PC = pop(); IME = 1; break;

        // RET NZ
        case 0xC0: RET(!ZERO_F()); break;

        // RET Z
        case 0xC8: RET(ZERO_F()); break;

        // RET NC
        case 0xD0: RET(!CARRY_F()); break;

        // RET C
        case 0xD8: RET(CARRY_F()); break;

        // RST
        case 0xC7: push(PC); PC = 0x00; break;
        case 0xCF: push(PC); PC = 0x08; break;
        case 0xD7: push(PC); PC = 0x10; break;
        case 0xDF: push(PC); PC = 0x18; break;
        case 0xE7: push(PC); PC = 0x20; break;
        case 0xEF: push(PC); PC = 0x28; break;
        case 0xF7: push(PC); PC = 0x30; break;
        case 0xFF: push(PC); PC = 0x38; break;

        // General-Purpose Arithmetic Operations and CPU Control Instructions
        // DAA
        case 0x27:
            n = 0;
            if (HALF_F() || (!SUB_F() && (((AF >> 8) & 0xF) > 0x09))) {
                n |= 0x06;
            }
            if (CARRY_F() || (!SUB_F() && ((AF >> 8) > 0x99))) {
                n |= 0x60;
            }
            AF = LD_Ab_n(AF, (AF >> 8) + (SUB_F() ? -n : n));
            AF = LD_aB_n(AF, ZERO_S(AF >> 8) | SUB_F() | (n > 0x06 ? CARRY : 0));
            break;

        // CPL
        case 0x2F:
            AF = LD_Ab_Ab(AF, ~AF);
            AF = LD_aB_n(AF, ZERO_F() | NEG | HALF | CARRY_F());
            break;

        // NOP
        case 0x00: break;

        // CCF
        case 0x3F: AF = LD_aB_n(AF, ZERO_F() | (CARRY_F() ^ (1 << 4))); break;

        // SCF
        case 0x37: AF = LD_aB_n(AF, ZERO_F() | CARRY); break;

        // DI
        case 0xF3: disableIRQ = 2; break;

        // EI
        case 0xFB: enableIRQ = 2; break;

        // HALT
        case 0x76: halted = 1; break;

        // STOP
        case 0x10: break;

        default: break;
    }
}
