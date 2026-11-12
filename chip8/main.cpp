#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
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

    // begin infinite loop to render ~60 frames per second
    while (true)
    {
        for (int i{ 0 }; i < 10; i++)
        {
            chip8.cycle();
        }

        chip8.tickTimers();

        chip8.renderDisplay();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }
}