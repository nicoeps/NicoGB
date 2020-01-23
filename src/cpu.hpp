union RegisterPair {
    struct {
        uint8_t low;
        uint8_t high;
    };
    struct {
        uint16_t value;
    };
};

class CPU {
    public:
        Memory& memory;

        RegisterPair af{}, bc{}, de{}, hl{};

        uint16_t& AF = af.value; uint8_t& A = af.high; uint8_t& F = af.low;
        uint16_t& BC = bc.value; uint8_t& B = bc.high; uint8_t& C = bc.low;
        uint16_t& DE = de.value; uint8_t& D = de.high; uint8_t& E = de.low;
        uint16_t& HL = hl.value; uint8_t& H = hl.high; uint8_t& L = hl.low;

        uint16_t SP = 0;
        uint16_t PC = 0;
        uint8_t opcode = 0;

        uint8_t V_BLANK = 0x01;
        uint8_t TIMER = 0x04;

        long long totalCycles = 0;
        uint8_t cycles = 0;

        uint8_t timer = 0;
        uint8_t prevTimer = 0;

        uint8_t divider = 0;
        uint8_t prevDivider = 0;

        bool halted = 0;

        uint8_t enableIRQ = 0;
        uint8_t disableIRQ = 0;
        bool IME = 0;

        bool run = true;

        uint8_t opCycles[0x100] = {
            1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
            0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
            2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
            2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            2,2,2,2,2,2,0,2,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
            2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
            2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
            3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
            3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4
        };
        
        uint8_t cbCycles[0x100] = {
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
            2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
            2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
            2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
            2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2
        };

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

        void cycle();

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

        CPU(Memory& memory) : memory(memory) {}

        void init() {
            AF = 0; // AAAAAAAAZNHC0000
            BC = 0; // BBBBBBBBCCCCCCCC
            DE = 0; // DDDDDDDDEEEEEEEE
            HL = 0; // HHHHHHHHLLLLLLLL
            SP = 0;
            PC = 0;
            opcode = 0;

            V_BLANK = 0x01;
            TIMER = 0x04;

            totalCycles = 0;
            cycles = 0;

            timer = 0;
            prevTimer = 0;

            divider = 0;
            prevDivider = 0;

            halted = 0;

            enableIRQ = 0;
            disableIRQ = 0;
            IME = 0;

            run = true;
        }
};
