#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>
#include "chip8.h"

// indexed by CHIP-8 key number
constexpr std::array<char, 16> keyMap{
    'X', '1', '2', '3',
    'Q', 'W', 'E', 'A',
    'S', 'D', 'Z', 'C',
    '4', 'R', 'F', 'V'
};

void checkForInputs(Chip8& chip8)
{
    for (int i{ 0 }; i < 16; i++)
    {
        chip8.setKeyHeld(i, GetAsyncKeyState(keyMap[i]) & 0x8000);
    }
}

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

    // clear terminal
    std::cout << "\033[2J";

    std::cout << "\033[?25l";

    // begin infinite loop to render ~60 frames per second
    while (true)
    {
        checkForInputs(chip8);

        for (int i{ 0 }; i < 10; i++)
        {
            chip8.cycle();
        }

        chip8.tickTimers();

        chip8.renderDisplay();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }
}