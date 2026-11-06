#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include "chip8.h"

int main()
{
    Chip8 chip8{};
    std::string input{};
    std::string message{ "Please type a valid filepath in this directory to load a CHIP8 ROM from: " };

    std::cout << message;
    std::getline(std::cin, input);

    while (!chip8.loadROM(input))
    {
        std::cout << message;
        std::getline(std::cin, input);
    }

    chip8.dumpMemory(0x200, 16);

    // chip8::cycle test loop below
    // TODO: remove

    for (int i{ 0 }; i < 200; i++)
    {
        chip8.cycle();
    }

    chip8.renderDisplay();
    chip8.debugDump();

    std::cin.get();
}