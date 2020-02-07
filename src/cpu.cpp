#include <bitset>

#include "cartridge.hpp"
#include "memory.hpp"
#include "cpu.hpp"
#include "ppu.hpp"

const uint8_t ZERO = 0x80;
const uint8_t NEG = 0x40;
const uint8_t HALF = 0x20;
const uint8_t CARRY = 0x10;

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
    uint8_t low = readByte();
    uint8_t high = readByte();
    return (high << 8) | low;
}

// Stack
void CPU::push(uint16_t r) {
    memory.write(--SP, r >> 8);
    memory.write(--SP, r & 0x00FF);
}

uint16_t CPU::pop() {
    uint8_t low = memory.read(SP++);
    uint8_t high = memory.read(SP++);
    return (high << 8) | low;
}

// ADD
void CPU::ADD(uint8_t n) {
    uint8_t a = A;
    A += n;
    F = ZERO_S(A) | HALF_S(a, n) | CARRY_S(a, n);
}

// ADC
void CPU::ADC(uint8_t n) {
    uint8_t a = A;
    A += n+(CARRY_F() >> 4);
    F = ZERO_S(A) | HALF_Sc(a, n) | CARRY_Sc(a, n);
}

// SUB
void CPU::SUB(uint8_t n) {
    uint8_t a = A;
    A -= n;
    F = ZERO_S(A) | NEG | HALF_Sb(a, n) | CARRY_Sb(a, n);
}

// SBC
void CPU::SBC(uint8_t n) {
    uint8_t a = A;
    A = A-n-(CARRY_F() >> 4);
    F = ZERO_S(A) | NEG | HALF_Sbc(a, n) | CARRY_Sbc(a, n);
}

// AND
void CPU::AND(uint8_t n) {
    A &= n;
    F = ZERO_S(A) | HALF;
}

// OR
void CPU::OR(uint8_t n) {
    A |= n;
    F = ZERO_S(A);
}

// XOR
void CPU::XOR(uint8_t n) {
    A ^= n;
    F = ZERO_S(A);
}

// CP
void CPU::CP(uint8_t n) {
    F = ZERO_S(A-n) | NEG | (((A & 0x0F) < (n & 0x0F)) ? HALF : 0) | ((A < n) ? CARRY : 0);
}

// INC
void CPU::INC(uint8_t n) {
    F = ZERO_S(n) | (((n & 0x0F) == 0) ? HALF : 0) | CARRY_F();
}

// DEC
void CPU::DEC(uint8_t n) {
    F = ZERO_S(n-1) | NEG | (((n >> 4) > ((n-1) >> 4)) ? HALF : 0) | CARRY_F();
}

// ADD nn
void CPU::ADD_nn(uint16_t nn) {
    F = ZERO_F() | HALF_Se(HL, nn) | CARRY_Se(HL, nn);
    HL += nn;
}

// Rotate Shift Instructions
// Rotate A left
void CPU::RLCA() {
    uint8_t a = A;
    A = (a << 1) | (a >> 7);
    F = (a >> 7) << 4;
}

void CPU::RLA() {
    uint8_t a = A;
    A = (a << 1) | (CARRY_F() >> 4);
    F = (a >> 7) << 4;
}

// Rotate A right
void CPU::RRCA() {
    uint8_t a = A;
    A = (a >> 1) | (a << 7);
    F = (a & 0x1) << 4;
}

void CPU::RRA() {
    uint8_t a = A;
    A = (a >> 1) | (CARRY_F() << 3);
    F = (a & 0x1) << 4;
}

// Rotate left
uint8_t CPU::RLC(uint8_t n) {
    n = (n << 1) | (n >> 7);
    F = ZERO_S(n) | ((n & 0x1) << 4);
    return n;
}

uint8_t CPU::RL(uint8_t n) {
    uint8_t C = (n >> 7) << 4;
    n = (n << 1) | (CARRY_F() >> 4);
    F = ZERO_S(n) | C;
    return n;
}

// Rotate right
uint8_t CPU::RRC(uint8_t n) {
    n = (n >> 1) | (n << 7);
    F = ZERO_S(n) | ((n & 0x80) >> 3);
    return n;
}

uint8_t CPU::RR(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = (n >> 1) | (CARRY_F() << 3);
    F = ZERO_S(n) | C;
    return n;
}

// SLA
uint8_t CPU::SLA(uint8_t n) {
    uint8_t C = (n >> 7) << 4;
    n = n << 1;
    F = ZERO_S(n) | C;
    return n;
}

// SRA
uint8_t CPU::SRA(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = (n >> 1) | (n & 0x80);
    F = ZERO_S(n) | C;
    return n;
}

// SRL
uint8_t CPU::SRL(uint8_t n) {
    uint8_t C = (n & 0x1) << 4;
    n = n >> 1;
    F = ZERO_S(n) | C;
    return n;
}

// SWAP
uint8_t CPU::SWAP(uint8_t n) {
    F = ZERO_S(n);
    return (n << 4) | (n >> 4);
}

// BIT
void CPU::BIT(uint8_t n, uint8_t b) {
    F = ZERO_S(n & (1 << b)) | HALF | CARRY_F();
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
    }
    prevDivider = divider;

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
        case 0x78: A = B; break;
        case 0x79: A = C; break;
        case 0x7A: A = D; break;
        case 0x7B: A = E; break;
        case 0x7C: A = H; break;
        case 0x7D: A = L; break;
        case 0x7F: break;

        // LD B,r
        case 0x40: break;
        case 0x41: B = C; break;
        case 0x42: B = D; break;
        case 0x43: B = E; break;
        case 0x44: B = H; break;
        case 0x45: B = L; break;
        case 0x47: B = A; break;

        // LD C,r
        case 0x48: C = B; break;
        case 0x49: break;
        case 0x4A: C = D; break;
        case 0x4B: C = E; break;
        case 0x4C: C = H; break;
        case 0x4D: C = L; break;
        case 0x4F: C = A; break;

        // LD D,r
        case 0x50: D = B; break;
        case 0x51: D = C; break;
        case 0x52: break;
        case 0x53: D = E; break;
        case 0x54: D = H; break;
        case 0x55: D = L; break;
        case 0x57: D = A; break;

        // LD E,r
        case 0x58: E = B; break;
        case 0x59: E = C; break;
        case 0x5A: E = D; break;
        case 0x5B: break;
        case 0x5C: E = H; break;
        case 0x5D: E = L; break;
        case 0x5F: E = A; break;

        // LD H,r
        case 0x60: H = B; break;
        case 0x61: H = C; break;
        case 0x62: H = D; break;
        case 0x63: H = E; break;
        case 0x64: break;
        case 0x65: H = L; break;
        case 0x67: H = A; break;
        
        // LD L,r
        case 0x68: L = B; break;
        case 0x69: L = C; break;
        case 0x6A: L = D; break;
        case 0x6B: L = E; break;
        case 0x6C: L = H; break;
        case 0x6D: break;
        case 0x6F: L = A; break;

        // LD r,n
        case 0x06: B = readByte(); break;
        case 0x0E: C = readByte(); break;
        case 0x16: D = readByte(); break;
        case 0x1E: E = readByte(); break;
        case 0x26: H = readByte(); break;
        case 0x2E: L = readByte(); break;
        case 0x3E: A = readByte(); break;

        // LD r,(HL)
        case 0x46: B = memory.read(HL); break;
        case 0x4E: C = memory.read(HL); break;
        case 0x56: D = memory.read(HL); break;
        case 0x5E: E = memory.read(HL); break;
        case 0x66: H = memory.read(HL); break;
        case 0x6E: L = memory.read(HL); break;
        case 0x7E: A = memory.read(HL); break;

        // LD (HL),r
        case 0x70: memory.write(HL, B); break;
        case 0x71: memory.write(HL, C); break;
        case 0x72: memory.write(HL, D); break;
        case 0x73: memory.write(HL, E); break;
        case 0x74: memory.write(HL, H); break;
        case 0x75: memory.write(HL, L); break;
        case 0x77: memory.write(HL, A); break;

        // LD (HL),n
        case 0x36: memory.write(HL, readByte()); break;

        // LD A,(BC)
        case 0x0A: A = memory.read(BC); break;

        // LD A,(DE)
        case 0x1A: A = memory.read(DE); break;
            
        // LD A,(C)
        case 0xF2: A = memory.read(0xFF00 | C); break;

        // LD (C),A
        case 0xE2: memory.write(0xFF00 | C, A); break;

        // LDH A,(n)
        case 0xF0: A = memory.read(0xFF00 | readByte()); break;

        // LDH (n),A
        case 0xE0: memory.write(0xFF00 | readByte(), A); break;

        // LD A,(nn)
        case 0xFA: A = memory.read(readb16()); break;

        // LD (nn),A
        case 0xEA: memory.write(readb16(), A); break;

        // LD A,(HLI)
        case 0x2A: A = memory.read(HL++); break;

        // LD A,(HLD)
        case 0x3A: A = memory.read(HL--); break;

        // LD (BC),A
        case 0x02: memory.write(BC, A); break;

        // LD (DE),A
        case 0x12: memory.write(DE, A); break;

        // LD (HLI),A
        case 0x22: memory.write(HL++, A); break;

        // LD (HLD),A
        case 0x32: memory.write(HL--, A); break;

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
            F = HALF_S(SP, e) | CARRY_S(SP, e);
            break;

        // LD (nn),SP
        case 0x08:
            nn = readb16();
            memory.write(nn, SP & 0x00FF);
            memory.write(nn+1, SP >> 8);
            break;

        // 8-Bit Arithmetic and Logical Operation Instructions
        // ADD A,r
        case 0x80: ADD(B); break;
        case 0x81: ADD(C); break;
        case 0x82: ADD(D); break;
        case 0x83: ADD(E); break;
        case 0x84: ADD(H); break;
        case 0x85: ADD(L); break;
        case 0x87: ADD(A); break;

        // ADD A,n
        case 0xC6: ADD(readByte()); break;

        // ADD A,(HL)
        case 0x86: ADD(memory.read(HL)); break;

        // ADC A,r
        case 0x88: ADC(B); break;
        case 0x89: ADC(C); break;
        case 0x8A: ADC(D); break;
        case 0x8B: ADC(E); break;
        case 0x8C: ADC(H); break;
        case 0x8D: ADC(L); break;
        case 0x8F: ADC(A); break;

        // ADC A,n
        case 0xCE: ADC(readByte()); break;

        // ADC A,(HL)
        case 0x8E: ADC(memory.read(HL)); break;

        // SUB r
        case 0x90: SUB(B); break;
        case 0x91: SUB(C); break;
        case 0x92: SUB(D); break;
        case 0x93: SUB(E); break;
        case 0x94: SUB(H); break;
        case 0x95: SUB(L); break;
        case 0x97: SUB(A); break;

        // SUB n
        case 0xD6: SUB(readByte()); break;

        // SUB (HL)
        case 0x96: SUB(memory.read(HL)); break;

        // SBC r
        case 0x98: SBC(B); break;
        case 0x99: SBC(C); break;
        case 0x9A: SBC(D); break;
        case 0x9B: SBC(E); break;
        case 0x9C: SBC(H); break;
        case 0x9D: SBC(L); break;
        case 0x9F: SBC(A); break;

        // SBC n
        case 0xDE: SBC(readByte()); break;

        // SBC (HL)
        case 0x9E: SBC(memory.read(HL)); break;

        // AND r
        case 0xA0: AND(B); break;
        case 0xA1: AND(C); break;
        case 0xA2: AND(D); break;
        case 0xA3: AND(E); break;
        case 0xA4: AND(H); break;
        case 0xA5: AND(L); break;
        case 0xA7: AND(A); break;

        // AND n
        case 0xE6: AND(readByte()); break;

        // AND (HL)
        case 0xA6: AND(memory.read(HL)); break;

        // OR r
        case 0xB0: OR(B); break;
        case 0xB1: OR(C); break;
        case 0xB2: OR(D); break;
        case 0xB3: OR(E); break;
        case 0xB4: OR(H); break;
        case 0xB5: OR(L); break;
        case 0xB7: OR(A); break;

        // OR n
        case 0xF6: OR(readByte()); break;

        // OR (HL)
        case 0xB6: OR(memory.read(HL)); break;

        // XOR r
        case 0xA8: XOR(B); break;
        case 0xA9: XOR(C); break;
        case 0xAA: XOR(D); break;
        case 0xAB: XOR(E); break;
        case 0xAC: XOR(H); break;
        case 0xAD: XOR(L); break;
        case 0xAF: XOR(A); break;

        // XOR n
        case 0xEE: XOR(readByte()); break;

        // XOR (HL)
        case 0xAE: XOR(memory.read(HL)); break;

        // CP r
        case 0xB8: CP(B); break;
        case 0xB9: CP(C); break;
        case 0xBA: CP(D); break;
        case 0xBB: CP(E); break;
        case 0xBC: CP(H); break;
        case 0xBD: CP(L); break;
        case 0xBF: CP(A); break;

        // CP n
        case 0xFE: CP(readByte()); break;

        // CP (HL)
        case 0xBE: CP(memory.read(HL)); break;

        // INC,r
        case 0x04: B++; INC(B); break;
        case 0x0C: C++; INC(C); break;
        case 0x14: D++; INC(D); break;
        case 0x1C: E++; INC(E); break;
        case 0x24: H++; INC(H); break;
        case 0x2C: L++; INC(L); break;
        case 0x3C: A++; INC(A); break;

        // INC,(HL)
        case 0x34: memory.write(HL, memory.read(HL)+1); INC(memory.read(HL)); break;

        // DEC,r
        case 0x05: DEC(B); B--; break;
        case 0x0D: DEC(C); C--; break;
        case 0x15: DEC(D); D--; break;
        case 0x1D: DEC(E); E--; break;
        case 0x25: DEC(H); H--; break;
        case 0x2D: DEC(L); L--; break;
        case 0x3D: DEC(A); A--; break;

        // DEC,(HL)
        case 0x35: DEC(memory.read(HL)); memory.write(HL, memory.read(HL)-1); break;

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
            F = HALF_S(SP, e) | CARRY_S(SP, e);
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
                case 0x00: B = RLC(B); break;
                case 0x01: C = RLC(C); break;
                case 0x02: D = RLC(D); break;
                case 0x03: E = RLC(E); break;
                case 0x04: H = RLC(H); break;
                case 0x05: L = RLC(L); break;
                case 0x06: memory.write(HL, RLC(memory.read(HL))); break;
                case 0x07: A = RLC(A); break;

                // RL
                case 0x10: B = RL(B); break;
                case 0x11: C = RL(C); break;
                case 0x12: D = RL(D); break;
                case 0x13: E = RL(E); break;
                case 0x14: H = RL(H); break;
                case 0x15: L = RL(L); break;
                case 0x16: memory.write(HL, RL(memory.read(HL))); break;
                case 0x17: A = RL(A); break;

                // RRC
                case 0x08: B = RRC(B); break;
                case 0x09: C = RRC(C); break;
                case 0x0A: D = RRC(D); break;
                case 0x0B: E = RRC(E); break;
                case 0x0C: H = RRC(H); break;
                case 0x0D: L = RRC(L); break;
                case 0x0E: memory.write(HL, RRC(memory.read(HL))); break;
                case 0x0F: A = RRC(A); break;

                // RR
                case 0x18: B = RR(B); break;
                case 0x19: C = RR(C); break;
                case 0x1A: D = RR(D); break;
                case 0x1B: E = RR(E); break;
                case 0x1C: H = RR(H); break;
                case 0x1D: L = RR(L); break;
                case 0x1E: memory.write(HL, RR(memory.read(HL))); break;
                case 0x1F: A = RR(A); break;

                // SLA
                case 0x20: B = SLA(B); break;
                case 0x21: C = SLA(C); break;
                case 0x22: D = SLA(D); break;
                case 0x23: E = SLA(E); break;
                case 0x24: H = SLA(H); break;
                case 0x25: L = SLA(L); break;
                case 0x26: memory.write(HL, SLA(memory.read(HL))); break;
                case 0x27: A = SLA(A); break;

                // SRA
                case 0x28: B = SRA(B); break;
                case 0x29: C = SRA(C); break;
                case 0x2A: D = SRA(D); break;
                case 0x2B: E = SRA(E); break;
                case 0x2C: H = SRA(H); break;
                case 0x2D: L = SRA(L); break;
                case 0x2E: memory.write(HL, SRA(memory.read(HL))); break;
                case 0x2F: A = SRA(A); break;

                // SRL
                case 0x38: B = SRL(B); break;
                case 0x39: C = SRL(C); break;
                case 0x3A: D = SRL(D); break;
                case 0x3B: E = SRL(E); break;
                case 0x3C: H = SRL(H); break;
                case 0x3D: L = SRL(L); break;
                case 0x3E: memory.write(HL, SRL(memory.read(HL))); break;
                case 0x3F: A = SRL(A); break;

                // SWAP
                case 0x30: B = SWAP(B); break;
                case 0x31: C = SWAP(C); break;
                case 0x32: D = SWAP(D); break;
                case 0x33: E = SWAP(E); break;
                case 0x34: H = SWAP(H); break;
                case 0x35: L = SWAP(L); break;
                case 0x36: memory.write(HL, SWAP(memory.read(HL))); break;
                case 0x37: A = SWAP(A); break;

                // Bit Operations
                // BIT 0,r
                case 0x40: BIT(B, 0); break;
                case 0x41: BIT(C, 0); break;
                case 0x42: BIT(D, 0); break;
                case 0x43: BIT(E, 0); break;
                case 0x44: BIT(H, 0); break;
                case 0x45: BIT(L, 0); break;
                case 0x46: BIT(memory.read(HL), 0); break;
                case 0x47: BIT(A, 0); break;

                // BIT 1,r
                case 0x48: BIT(B, 1); break;
                case 0x49: BIT(C, 1); break;
                case 0x4A: BIT(D, 1); break;
                case 0x4B: BIT(E, 1); break;
                case 0x4C: BIT(H, 1); break;
                case 0x4D: BIT(L, 1); break;
                case 0x4E: BIT(memory.read(HL), 1); break;
                case 0x4F: BIT(A, 1); break;

                // BIT 2,r
                case 0x50: BIT(B, 2); break;
                case 0x51: BIT(C, 2); break;
                case 0x52: BIT(D, 2); break;
                case 0x53: BIT(E, 2); break;
                case 0x54: BIT(H, 2); break;
                case 0x55: BIT(L, 2); break;
                case 0x56: BIT(memory.read(HL), 2); break;
                case 0x57: BIT(A, 2); break;

                // BIT 3,r
                case 0x58: BIT(B, 3); break;
                case 0x59: BIT(C, 3); break;
                case 0x5A: BIT(D, 3); break;
                case 0x5B: BIT(E, 3); break;
                case 0x5C: BIT(H, 3); break;
                case 0x5D: BIT(L, 3); break;
                case 0x5E: BIT(memory.read(HL), 3); break;
                case 0x5F: BIT(A, 3); break;

                // BIT 4,r
                case 0x60: BIT(B, 4); break;
                case 0x61: BIT(C, 4); break;
                case 0x62: BIT(D, 4); break;
                case 0x63: BIT(E, 4); break;
                case 0x64: BIT(H, 4); break;
                case 0x65: BIT(L, 4); break;
                case 0x66: BIT(memory.read(HL), 4); break;
                case 0x67: BIT(A, 4); break;

                // BIT 5,r
                case 0x68: BIT(B, 5); break;
                case 0x69: BIT(C, 5); break;
                case 0x6A: BIT(D, 5); break;
                case 0x6B: BIT(E, 5); break;
                case 0x6C: BIT(H, 5); break;
                case 0x6D: BIT(L, 5); break;
                case 0x6E: BIT(memory.read(HL), 5); break;
                case 0x6F: BIT(A, 5); break;

                // BIT 6,r
                case 0x70: BIT(B, 6); break;
                case 0x71: BIT(C, 6); break;
                case 0x72: BIT(D, 6); break;
                case 0x73: BIT(E, 6); break;
                case 0x74: BIT(H, 6); break;
                case 0x75: BIT(L, 6); break;
                case 0x76: BIT(memory.read(HL), 6); break;
                case 0x77: BIT(A, 6); break;

                // BIT 7,r
                case 0x78: BIT(B, 7); break;
                case 0x79: BIT(C, 7); break;
                case 0x7A: BIT(D, 7); break;
                case 0x7B: BIT(E, 7); break;
                case 0x7C: BIT(H, 7); break;
                case 0x7D: BIT(L, 7); break;
                case 0x7E: BIT(memory.read(HL), 7); break;
                case 0x7F: BIT(A, 7); break;

                // RES 0,r
                case 0x80: B = RES(B, 0); break;
                case 0x81: C = RES(C, 0); break;
                case 0x82: D = RES(D, 0); break;
                case 0x83: E = RES(E, 0); break;
                case 0x84: H = RES(H, 0); break;
                case 0x85: L = RES(L, 0); break;
                case 0x86: memory.write(HL, RES(memory.read(HL), 0)); break;
                case 0x87: A = RES(A, 0); break;

                // RES 1,r
                case 0x88: B = RES(B, 1); break;
                case 0x89: C = RES(C, 1); break;
                case 0x8A: D = RES(D, 1); break;
                case 0x8B: E = RES(E, 1); break;
                case 0x8C: H = RES(H, 1); break;
                case 0x8D: L = RES(L, 1); break;
                case 0x8E: memory.write(HL, RES(memory.read(HL), 1)); break;
                case 0x8F: A = RES(A, 1); break;

                // RES 2,r
                case 0x90: B = RES(B, 2); break;
                case 0x91: C = RES(C, 2); break;
                case 0x92: D = RES(D, 2); break;
                case 0x93: E = RES(E, 2); break;
                case 0x94: H = RES(H, 2); break;
                case 0x95: L = RES(L, 2); break;
                case 0x96: memory.write(HL, RES(memory.read(HL), 2)); break;
                case 0x97: A = RES(A, 2); break;

                // RES 3,r
                case 0x98: B = RES(B, 3); break;
                case 0x99: C = RES(C, 3); break;
                case 0x9A: D = RES(D, 3); break;
                case 0x9B: E = RES(E, 3); break;
                case 0x9C: H = RES(H, 3); break;
                case 0x9D: L = RES(L, 3); break;
                case 0x9E: memory.write(HL, RES(memory.read(HL), 3)); break;
                case 0x9F: A = RES(A, 3); break;

                // RES 4,r
                case 0xA0: B = RES(B, 4); break;
                case 0xA1: C = RES(C, 4); break;
                case 0xA2: D = RES(D, 4); break;
                case 0xA3: E = RES(E, 4); break;
                case 0xA4: H = RES(H, 4); break;
                case 0xA5: L = RES(L, 4); break;
                case 0xA6: memory.write(HL, RES(memory.read(HL), 4)); break;
                case 0xA7: A = RES(A, 4); break;

                // RES 5,r
                case 0xA8: B = RES(B, 5); break;
                case 0xA9: C = RES(C, 5); break;
                case 0xAA: D = RES(D, 5); break;
                case 0xAB: E = RES(E, 5); break;
                case 0xAC: H = RES(H, 5); break;
                case 0xAD: L = RES(L, 5); break;
                case 0xAE: memory.write(HL, RES(memory.read(HL), 5)); break;
                case 0xAF: A = RES(A, 5); break;

                // RES 6,r
                case 0xB0: B = RES(B, 6); break;
                case 0xB1: C = RES(C, 6); break;
                case 0xB2: D = RES(D, 6); break;
                case 0xB3: E = RES(E, 6); break;
                case 0xB4: H = RES(H, 6); break;
                case 0xB5: L = RES(L, 6); break;
                case 0xB6: memory.write(HL, RES(memory.read(HL), 6)); break;
                case 0xB7: A = RES(A, 6); break;

                // RES 7,r
                case 0xB8: B = RES(B, 7); break;
                case 0xB9: C = RES(C, 7); break;
                case 0xBA: D = RES(D, 7); break;
                case 0xBB: E = RES(E, 7); break;
                case 0xBC: H = RES(H, 7); break;
                case 0xBD: L = RES(L, 7); break;
                case 0xBE: memory.write(HL, RES(memory.read(HL), 7)); break;
                case 0xBF: A = RES(A, 7); break;

                // SET 0,r
                case 0xC0: B = SET(B, 0); break;
                case 0xC1: C = SET(C, 0); break;
                case 0xC2: D = SET(D, 0); break;
                case 0xC3: E = SET(E, 0); break;
                case 0xC4: H = SET(H, 0); break;
                case 0xC5: L = SET(L, 0); break;
                case 0xC6: memory.write(HL, SET(memory.read(HL), 0)); break;
                case 0xC7: A = SET(A, 0); break;

                // SET 1,r
                case 0xC8: B = SET(B, 1); break;
                case 0xC9: C = SET(C, 1); break;
                case 0xCA: D = SET(D, 1); break;
                case 0xCB: E = SET(E, 1); break;
                case 0xCC: H = SET(H, 1); break;
                case 0xCD: L = SET(L, 1); break;
                case 0xCE: memory.write(HL, SET(memory.read(HL), 1)); break;
                case 0xCF: A = SET(A, 1); break;

                // SET 2,r
                case 0xD0: B = SET(B, 2); break;
                case 0xD1: C = SET(C, 2); break;
                case 0xD2: D = SET(D, 2); break;
                case 0xD3: E = SET(E, 2); break;
                case 0xD4: H = SET(H, 2); break;
                case 0xD5: L = SET(L, 2); break;
                case 0xD6: memory.write(HL, SET(memory.read(HL), 2)); break;
                case 0xD7: A = SET(A, 2); break;

                // SET 3,r
                case 0xD8: B = SET(B, 3); break;
                case 0xD9: C = SET(C, 3); break;
                case 0xDA: D = SET(D, 3); break;
                case 0xDB: E = SET(E, 3); break;
                case 0xDC: H = SET(H, 3); break;
                case 0xDD: L = SET(L, 3); break;
                case 0xDE: memory.write(HL, SET(memory.read(HL), 3)); break;
                case 0xDF: A = SET(A, 3); break;

                // SET 4,r
                case 0xE0: B = SET(B, 4); break;
                case 0xE1: C = SET(C, 4); break;
                case 0xE2: D = SET(D, 4); break;
                case 0xE3: E = SET(E, 4); break;
                case 0xE4: H = SET(H, 4); break;
                case 0xE5: L = SET(L, 4); break;
                case 0xE6: memory.write(HL, SET(memory.read(HL), 4)); break;
                case 0xE7: A = SET(A, 4); break;

                // SET 5,r
                case 0xE8: B = SET(B, 5); break;
                case 0xE9: C = SET(C, 5); break;
                case 0xEA: D = SET(D, 5); break;
                case 0xEB: E = SET(E, 5); break;
                case 0xEC: H = SET(H, 5); break;
                case 0xED: L = SET(L, 5); break;
                case 0xEE: memory.write(HL, SET(memory.read(HL), 5)); break;
                case 0xEF: A = SET(A, 5); break;

                // SET 6,r
                case 0xF0: B = SET(B, 6); break;
                case 0xF1: C = SET(C, 6); break;
                case 0xF2: D = SET(D, 6); break;
                case 0xF3: E = SET(E, 6); break;
                case 0xF4: H = SET(H, 6); break;
                case 0xF5: L = SET(L, 6); break;
                case 0xF6: memory.write(HL, SET(memory.read(HL), 6)); break;
                case 0xF7: A = SET(A, 6); break;

                // SET 7,r
                case 0xF8: B = SET(B, 7); break;
                case 0xF9: C = SET(C, 7); break;
                case 0xFA: D = SET(D, 7); break;
                case 0xFB: E = SET(E, 7); break;
                case 0xFC: H = SET(H, 7); break;
                case 0xFD: L = SET(L, 7); break;
                case 0xFE: memory.write(HL, SET(memory.read(HL), 7)); break;
                case 0xFF: A = SET(A, 7); break;

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
            if (HALF_F() || (!SUB_F() && ((A & 0xF) > 0x09))) {
                n |= 0x06;
            }
            if (CARRY_F() || (!SUB_F() && (A > 0x99))) {
                n |= 0x60;
            }
            A += (SUB_F() ? -n : n);
            F = ZERO_S(A) | SUB_F() | (n > 0x06 ? CARRY : 0);
            break;

        // CPL
        case 0x2F: A = ~A; F = ZERO_F() | NEG | HALF | CARRY_F(); break;

        // NOP
        case 0x00: break;

        // CCF
        case 0x3F: F = ZERO_F() | (CARRY_F() ^ (1 << 4)); break;

        // SCFs
        case 0x37: F = ZERO_F() | CARRY; break;

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
