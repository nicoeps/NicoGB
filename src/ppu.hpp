class PPU {
    private:
        void updateScanLine();
        void drawBackground();
        void drawSprites();
        void drawWindow();

    public:
        CPU& cpu;

        uint32_t framebuffer[160*144] = {};

        uint16_t lyAddr = 0xFF44;
        uint16_t statAddr = 0xFF41;
        uint16_t lcdcAddr = 0xFF40;
        uint8_t ly = 0;
        uint8_t stat = 0;
        uint8_t lcdc = 0;

        void update();
        int totalCycles = 0;

        PPU(CPU& cpu) : cpu(cpu) {}
};
