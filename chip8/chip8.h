#ifndef CHIP8HFHFBANAHFG
#define CHIP8HFHFBANAHFG

#include <cstdint>
#include <array>
#include <string>
#include <random>

class Chip8
{

    // random number generator
    std::mt19937 rng{ std::random_device{}() };

    // stack
    std::array<uint16_t, 16> stack{};

    // program counter, address register
    uint16_t pc{ ROM_START };
    uint16_t I{};

    // array of memory and array of registers
    std::array<uint8_t, 4096> mem{};
    std::array<uint8_t, 16> V{};

    // sound timer and delay timer
    uint8_t st{};
    uint8_t dt{};

    // stack pointer
    uint8_t sp{};

    std::array<bool, 2048> display{};

    static constexpr std::array<uint8_t, 80> font{
    0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
    0x20, 0x60, 0x20, 0x20, 0x70,   // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
    0xF0, 0x80, 0xF0, 0x80, 0x80    // F
    };

    static constexpr uint16_t FONT_START{ 0x50 };
    static constexpr uint16_t ROM_START{ 0x200 };

public:
    Chip8();
    bool loadROM(const std::string& path);
    void cycle();
    void debugDump() const;
    void dumpMemory(uint16_t pos, int count) const;
    void renderDisplay() const;
    void printUnknownOpcode(uint16_t opcode) const;
    void tickTimers();
};

#endif