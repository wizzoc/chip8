#include <array>
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#define NOMINMAX
#include <Windows.h>
#include <limits>
#include <cstdlib>
#include "chip8.h"

// int input validation
int getValidatedInt(const std::string& prompt, const int lowLimit, const int highLimit)
{
    int input{};

    while (true)
    {
        std::cout << prompt;
        std::cin >> input;

        if (std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        if (input < lowLimit || input > highLimit)
        {
            std::cout << "That input is not within the valid range. Please try again.\n";
            continue;
        }

        return input;

    }
}


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

    int profile{ getValidatedInt("Please input 0 to use a modern profile for this ROM or enter 1 to use a classic profile: ", 0, 1) };
    chip8.setUsesClassicProfile(profile);

    // clear terminal
    std::cout << "\033[2J";

    std::cout << "\033[?25l";

    // begin infinite loop to render ~60 frames per second
    bool soundWasOn{ false };
    while (true)
    {
        checkForInputs(chip8);

        for (int i{ 0 }; i < 10; i++)
        {
            bool endFrame{ chip8.cycle() };

            if (endFrame)
                break;
        }

        bool soundIsOn{chip8.getSoundTimer() > 0};
        chip8.tickTimers();

        if (!soundWasOn && soundIsOn)
            std::cout << "\a";

        soundWasOn = soundIsOn;

        chip8.renderDisplay();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
   }
}