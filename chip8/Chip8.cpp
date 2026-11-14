#include <iostream>
#include <fstream>
#include <iomanip>
#include "chip8.h"

Chip8::Chip8()
{
	for (std::size_t i{ 0 }; i < font.size(); i++)
	{
		mem[FONT_START + i] = font[i];
	}
}

bool Chip8::loadROM(const std::string& filepath)
{
	std::ifstream file{ filepath, std::ios::binary };

	if (!file)
	{
		std::cout << "File not found or unable to open file.\n";
		return false;
	}

	file.seekg(0, std::ios::end);
	const std::streamsize size{ file.tellg() };
	file.seekg(0, std::ios::beg);

	if (size > static_cast<std::streamsize>(mem.size() - ROM_START))
	{
		std::cout << "File size too big.\n";
		return false;
	}

	file.read(reinterpret_cast<char*>(&mem[ROM_START]), size);

	if (file.gcount() < size)
	{
		std::cout << "File read failed.\n";
		return false;
	}

	return true;
}

void Chip8::dumpMemory(uint16_t pos, int count) const
{
	for (int i{ 0 }; i < count; i++)
	{
		std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(mem[pos + i]) << ' ';
	}
	std::cout << '\n';
}

void Chip8::debugDump() const
{
	std::cout << "pc:" << std::hex << pc << std::dec << '\n';
	std::cout << "address register: " << std::hex << I << std::dec << '\n';
	std::cout << "stack pointer: " << static_cast<int>(sp) << '\n';

	for (std::size_t i{ 0 }; i < V.size(); i++)
	{
		std::cout << "V[" << i << "]: " << static_cast<int>(V[i]) << '\n';
	}

	std::cout << "st: " << static_cast<int>(st) << '\n';
	std::cout << "dt:" << static_cast<int>(dt) << '\n';
}

void Chip8::renderDisplay() const
{
	std::string frame{};
	
	// reserving space so it doesn't need to change size mid-function
	// 32 rows, 128 chars per row (+1 for newline), 3 for escape code
	frame.reserve(32 * 129 + 3);
	frame += "\033[H";

	for (int row{ 0 }; row < 32; row++)
	{
		for (int col{ 0 }; col < 64; col++)
		{
			if (display[(row * 64) + col])
				frame += "##";
			else
				frame += "  ";
		}

		frame += '\n';
	}

	std::cout << frame << std::flush;
}

void Chip8::printUnknownOpcode(uint16_t opcode) const
{
	std::cout << "Unknown opcode " << std::hex << opcode << std::dec << " at " << pc - 2 << '\n';
}

void Chip8::tickTimers()
{
	if (st > 0)
		st--;

	if (dt > 0)
		dt--;
}

void Chip8::setKeyHeld(int index, bool held)
{
	inputsHeld[index] = held;
}

void Chip8::cycle()
{
	uint16_t opcode{ static_cast<uint16_t>((mem[pc] * 256) + mem[pc + 1]) };
	uint8_t firstNibble{ static_cast<uint8_t>(opcode / 4096) };
	uint8_t X{ static_cast<uint8_t>((opcode % 4096) / 256) };
	uint8_t Y{ static_cast<uint8_t>((opcode % 256) / 16) };
	uint16_t NNN{ static_cast<uint16_t>(opcode % 4096) };
	uint8_t NN{ static_cast<uint8_t>(opcode % 256) };
	uint8_t N{ static_cast<uint8_t>(opcode % 16) };

	pc += 2;

	switch (firstNibble)
	{
	case 0:
		switch (opcode)
		{
		case 0x00E0:
			// clear screen
			for (bool& pixel : display) pixel = false; break;
		case 0x00EE:
			// return from subroutine
			// set pc to top value of stack, pop value from stack, decrement stack pointer

			if (sp == 0)
			{
				std::cout << "Warning: return from subroutine (stack pop) while stack pointer is at 0.\n";
				break;
			}

			pc = stack[sp - 1];
			sp--;
			break;
		default:
			// either unknown opcode or call machine-code routine opcode, unimplemented
			printUnknownOpcode(opcode);
			break;
		}
		break;
	case 1:
		// jump, set program counter
		pc = NNN;
		break;
	case 2:
		// call a subroutine

		// save current pc onto stack, warn if full
		if (sp == 16)
		{
			std::cout << "Stack is full! Tried to add current program counter to stack while stack was full at " << std::hex << pc - 2 << std::dec << '\n';
			break;
		}

		stack[sp] = pc;
		sp++;

		// jump to NNN (last three hex digits)
		pc = NNN;
		break;
	case 3:
		// skip if V[second digit] equals last two digits
		if (V[X] == NN)
			pc += 2;

		break;
	case 4:
		// skip if V[second digit] does not equal last two digits
		if (V[X] != NN)
			pc += 2;

		break;
	case 5:
		// if last digit isn't 0, not a valid instruction
		if (N != 0)
		{
			printUnknownOpcode(opcode);
			break;
		}

		// skip if register V[second digit] == V[third digit]
		if (V[X] == V[Y])
			pc += 2;

		break;
	case 6:
		// set register V[second hex digit] to last two hex digits
		V[X] = NN;
		break;
	case 7:
		// add constant value (last two hex digits) to value already in V[second hex digit]
		V[X] += NN;
		break;
	case 8:
		// arithmetic
	{
		uint8_t flag{ 0 };

		switch (N)
		{
		case 0:
			V[X] = V[Y];
			break;
		case 1:
			V[X] = V[X] | V[Y];
			break;
		case 2:
			V[X] = V[X] & V[Y];
			break;
		case 3:
			V[X] = V[X] ^ V[Y];
			break;
		case 4:
			// set special flag if addition wrapped over 255
			if (V[X] + V[Y] > 255)
				flag = 1;

			V[X] = V[X] + V[Y];
			V[0xF] = flag;
			break;
		case 5:
			// set special flag if number being subtracted FROM is bigger than the number being subtraced
			if (V[X] >= V[Y])
				flag = 1;

			V[X] = V[X] - V[Y];
			V[0xF] = flag;
			break;
		case 6:
			// shift right, put bit that fell off into V[0xF]
			flag = V[X] % 2;

			V[X] = V[X] / 2;
			V[0xF] = flag;
			break;
		case 7:
			// same as case 5, reversed
			if (V[Y] >= V[X])
				flag = 1;

			V[X] = V[Y] - V[X];
			V[0xF] = flag;
			break;
		case 0xE:
			// shift bits left, set leftmost bit (the wrapped one) to V[0xF]
			flag = (V[X] / 128) % 2;
			
			V[X] *= 2;
			V[0xF] = flag;
			break;
		default:
			printUnknownOpcode(opcode);
			break;
		}
	}
	break;
	case 9:
		// if last digit isn't 0, not a valid instruction
		if (N != 0)
		{
			printUnknownOpcode(opcode);
			break;
		}
		// skip if V[second digit] does not equal V[third digit]
		if (V[X] != V[Y])
			pc += 2;

		break;
	case 0xA:
		// set address register
		I = NNN;
		break;
	case 0xB:
		// jump to last three digits + what's in register 0
		pc = NNN + V[0];
		break;
	case 0xC:
		// generate random number from rng engine and bitwise AND with last two digits
		V[X] = (rng() % 256) & NN;
		break;
	case 0xD:
	{
		// draw sprite to x y coordinate
		uint8_t	x{ V[X] };
		x = x % 64;
		uint8_t y{ V[Y] };
		y = y % 32;

		// set special register used for flags to 0
		V[0xF] = 0;

		// N many rows, always 8 columns (8 bits)
		for (int row{ 0 }; row < N; row++)
		{
			int py{ y + row };
			uint8_t placeValue{ 128 };

			for (int col{ 0 }; col <= 7; col++)
			{
				int px{ x + col };

				// extract the current bit from the byte at the memory index
				int bit{ (mem[I + row] / placeValue) % 2 };
				placeValue /= 2;

				// if off-screen
				if (px >= 64 || py >= 32)
					continue;

				// if a collission happened; bit at location on and draw sprite bit also on
				if (bit && display[(py) * 64 + (px)])
					V[0xF] = 1;

				display[(py) * 64 + (px)] ^= bit;
			}
		}

		break;
	}
	case 0xE:
	{
		switch (NN)
		{
			case 0x9E:
				// skip next instruction if key number in register X is held
				if (inputsHeld[V[X] % 16])
					pc += 2;

				break;
			case 0xA1:
				// skip next instruction if key number in register X is NOT held
				if (!inputsHeld[V[X] % 16])
					pc += 2;
				break;
			default:
				printUnknownOpcode(opcode);
				break;
				
		}
		break;
	}
	case 0xF:
	{
		switch (NN)
		{
			case 0x07:
				// copy delay timer into register X
				V[X] = dt;
				break;
			case 0x0A:
			{
				// wait for a full key press down, then released
				if (keyWaitingFor == -1)
				{
					// not waiting on a key yet, look for one being held
					for (int i{ 0 }; i < 16; i++)
					{
						if (inputsHeld[i])
						{
							keyWaitingFor = i;
							break;
						}
					}

					pc -= 2;   // either way run this instruction again next cycle
				}
				else if (inputsHeld[keyWaitingFor])
				{
					// key still held, keep waiting
					pc -= 2;
				}
				else
				{
					// key released, hand it to the program and stop waiting
					V[X] = keyWaitingFor;
					keyWaitingFor = -1;
				}

				break;
			}
			case 0x15:
				// set delay timer to register X
				dt = V[X];
				break;
			case 0x18:
				// set sound timer to register X
				st = V[X];
				break;
			case 0x29:
				// set I to where register X's font digit starts
				I = FONT_START + ((V[X] % 16) * 5);
				break;
			case 0x33:
				// take the value in register X and put its HUNDREDS (line 1), TENS (line 2), AND ONES (line 3) place value into memory starting at the index register
				mem[I] = (V[X] / 100);
				mem[I + 1] = (V[X] / 10) % 10;
				mem[I + 2] = V[X] % 10;
				break;
			case 0x55:
				// copy registers 0 to X into memory (opposite of case 0x65) starting at index register
				for (int i{ 0 }; i <= X; i++)
				{
					mem[I + i] = V[i];
				}
				break;
			case 0x65:
				// copy from memory into registers 0 through X inclusive starting at index register
				for (int i{ 0 }; i <= X; i++)
				{
					V[i] = mem[I + i];
				}
				break;
			case 0x1E:
				// add register X to index register
				I += V[X];
				break;
			default:
				printUnknownOpcode(opcode);
				break;
		}
		break;
	}
	default:
		printUnknownOpcode(opcode);
		break;
	}
}