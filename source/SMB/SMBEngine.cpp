#include <cstring>
#include "SMBEngine.hpp"

SMBEngine::SMBEngine(uint8_t* romImage) :
    apu(),
    controller1(),
    controller2(),
    ppu(romImage + 0x10 + 0x8000), // CHR-ROM starts after 16-byte header + 32KB PRG-ROM
    dataPointers(),
    audioBuffer(),
    a(0),
    x(0),
    y(0),
    s(0),
    p(0),
    ram()
{
    // Initialize pointers in SMBDataPointers
    writeDataPointers();

    // Map memory callbacks for NES address space
    memoryAccess.resize(0x10000);

    // 0x0000 - 0x07FF: Internal 2KB RAM (mirrored up to 0x1FFF)
    for (int addr = 0x0000; addr < 0x2000; addr++)
    {
        memoryAccess[addr] = MemoryAccess(&ram[addr & 0x07FF]);
    }

    // 0x2000 - 0x2007: PPU Registers (mirrored up to 0x3FFF)
    for (int addr = 0x2000; addr < 0x4000; addr += 8)
    {
        memoryAccess[addr + 0] = MemoryAccess(nullptr, [this](uint8_t val) { ppu.writePPUCTRL(val); });
        memoryAccess[addr + 1] = MemoryAccess(nullptr, [this](uint8_t val) { ppu.writePPUMASK(val); });
        memoryAccess[addr + 2] = MemoryAccess([this]() { return ppu.readPPUSTATUS(); });
        memoryAccess[addr + 3] = MemoryAccess(nullptr, [this](uint8_t val) { ppu.writeOAMADDR(val); });
        memoryAccess[addr + 4] = MemoryAccess([this]() { return ppu.readOAMDATA(); }, [this](uint8_t val) { ppu.writeOAMDATA(val); });
        memoryAccess[addr + 5] = MemoryAccess(nullptr, [this](uint8_t val) { ppu.writePPUSCROLL(val); });
        memoryAccess[addr + 6] = MemoryAccess(nullptr, [this](uint8_t val) { ppu.writePPUADDR(val); });
        memoryAccess[addr + 7] = MemoryAccess([this]() { return ppu.readPPUDATA(); }, [this](uint8_t val) { ppu.writePPUDATA(val); });
    }

    // 0x4000 - 0x4013: APU Registers
    for (int addr = 0x4000; addr <= 0x4013; addr++)
    {
        memoryAccess[addr] = MemoryAccess(nullptr, [this, addr](uint8_t val) { apu.writeRegister(addr, val); });
    }

    // 0x4014: OAM DMA
    memoryAccess[0x4014] = MemoryAccess(nullptr, [this](uint8_t val) {
        uint16_t base = static_cast<uint16_t>(val) << 8;
        for (int i = 0; i < 256; i++)
        {
            ppu.writeOAMDATA(readMemory(base + i));
        }
    });

    // 0x4015: APU Status
    memoryAccess[0x4015] = MemoryAccess(
        [this]() { return apu.readStatus(); },
        [this](uint8_t val) { apu.writeStatus(val); }
    );

    // 0x4016: Controller 1
    memoryAccess[0x4016] = MemoryAccess(
        [this]() { return controller1.read(); },
        [this](uint8_t val) {
            controller1.write(val);
            controller2.write(val);
        }
    );

    // 0x4017: Controller 2 / APU Frame Counter
    memoryAccess[0x4017] = MemoryAccess(
        [this]() { return controller2.read(); },
        [this](uint8_t val) { apu.writeFrameCounter(val); }
    );
}

SMBEngine::~SMBEngine()
{
}

void SMBEngine::reset()
{
    std::memset(ram, 0, sizeof(ram));
    a = 0;
    x = 0;
    y = 0;
    s = 0xFD;
    p = 0x34;

    apu.reset();
    ppu.reset();

    // Run original SMB reset/init routine
    code_RESET();
}

void SMBEngine::update()
{
    // Step APU emulation
    apu.step();

    // Trigger NMI (vblank interrupt routine in SMB)
    code_NonMaskableInterrupt();

    // Render frame to PPU output
    ppu.renderFrame();
}

void SMBEngine::render(uint32_t* buffer)
{
    std::memcpy(buffer, ppu.getFrameBuffer(), sizeof(uint32_t) * 256 * 240);
}

void SMBEngine::audioCallback(uint8_t* stream, int len)
{
    apu.readSamples(stream, len);
}

uint8_t SMBEngine::readMemory(uint16_t address)
{
    return memoryAccess[address].read();
}

void SMBEngine::writeMemory(uint16_t address, uint8_t value)
{
    memoryAccess[address].write(value);
}
