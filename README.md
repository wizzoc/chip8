# CHIP-8 Interpreter

A CHIP-8 interpreter in C++ that runs entirely in the Windows console: no graphics library and no dependencies. Every instruction group was verified against test ROMs with exact expected register values before I moved on to the next.

This is the second in a series of from-scratch systems projects. The first is [Three Long Nights](https://github.com/wizzoc/three-long-nights), a console survival game. Next is a software rasterizer aimed at rendering Quake's first level.

## Highlights

- **Console rendering.** The 64×32 display is drawn as text at two characters per pixel, so images keep their proportions without SDL or any other graphics library.
- **Test-driven, one instruction group at a time.** Each group was checked against a register-check test ROM before the next was started. Each ROM counts passed checks in one register and records the ID of the last failure in another, so a single register dump says exactly what broke.
- **Explicit rulings on the ambiguous instructions.** Several CHIP-8 instructions behave differently across historical interpreters. Each one is a deliberate, documented decision rather than whatever the first tutorial did.

## Status

**In progress:** 31 of 34 instructions implemented and passing their tests. Still to come are the real-time loop (60 Hz timers) and keyboard input, which is what makes games playable.

## Bugs worth telling

**All 25 checks passed, and the program counter was still wrong.** After refactoring the decoder to extract each opcode field once, a test ROM reported every check passing, but the register dump showed the program counter at `0x154` instead of `0x2FA`. The 12-bit address field had been stored in an 8-bit variable. The ROM's final jump to `0x2FA` lost its top digit and went to `0xFA`, empty memory, and ran through 45 cycles of blank instructions. A cast I'd added had silenced the compiler warning that would have caught it. *Lesson: check every expected value, not just the pass count, and don't cast just to make a warning go away.*

**The flag that overwrote itself.** Arithmetic instructions write a carry or borrow flag into register `VF`, but a program can also target `VF` as the destination. My first version set the flag first and stored the result second, so when the destination *was* `VF`, the result overwrote the flag. A test that targets `VF` caught it. *Fix: copy the inputs first, store the result, and write the flag last.*

**Subtraction isn't addition in reverse.** I assumed the borrow flag would mirror the carry flag, 1 meaning "it borrowed." In CHIP-8 it's inverted: 1 means *no* borrow, inherited from the original hardware's CPU. *Lesson: when two operations look like a pair, read each spec separately.*

## Design decisions

| Instruction | Behaviour chosen | Why |
|---|---|---|
| `8XY6` / `8XYE` (shifts) | Shift `VX` in place and ignore `VY` | Modern (CHIP-48 / SUPER-CHIP) behaviour, which most available ROMs expect |
| `BNNN` (jump with offset) | Offset by `V0` | Original COSMAC VIP behaviour, safest for classic ROMs |
| `FX55` / `FX65` (store/load registers) | Leave `I` unchanged | Modern behaviour, which most available ROMs expect |

Also:
- Sprite start positions wrap around the screen, and sprite bodies clip at the edges.
- Stack overflow and underflow are reported instead of corrupting memory.
- Unknown opcodes are reported with their address instead of being silently ignored.

## Testing

| ROM | Covers | Result |
|---|---|---|
| `IBM Logo.ch8` | Loading, decoding, drawing, rendering | Logo renders correctly |
| `control-flow-test.ch8` | Calls and returns (two levels deep), all four conditional skips | Pass |
| `arithmetic-test.ch8` | The whole `8XY_` family, including carry/borrow flags and `VF` as a target | 21 / 21 |
| `misc-test.ch8` | `BNNN`, `CXNN`, `FX1E` `FX29` `FX33` `FX55` `FX65` | 25 / 25 |

The ROMs are in `tests/`.

## Building and running

Built with Visual Studio (MSVC) on Windows. Open the solution, build x64, run it, and type a ROM path when prompted, e.g. `tests/IBM Logo.ch8`.

Until the real-time loop is finished, the program runs a fixed number of cycles as a test harness, then renders the display and prints a register dump.

## Roadmap

1. Real-time loop: about 10 instructions per frame at 60 frames per second, with the display redrawn every frame.
2. Keyboard input (`1234` / `QWER` / `ASDF` / `ZXCV` mapped to the 16-key hex keypad).
3. The community test suites, then Pong and Tetris.

## How it works

All machine state lives in one `Chip8` class: 4 KB of memory (font at `0x050`, programs at `0x200`), 16 eight-bit registers, a 16-bit index register and program counter, a 16-entry stack, two timers, and a 64×32 display stored as a flat array.

Each cycle fetches two bytes and combines them into a 16-bit opcode, advances the program counter, extracts every opcode field once, then dispatches on the first hex digit. The `0`, `8` and `F` families dispatch a second time on their remaining digits.

## About this project

All interpreter code is my own. I used Claude (Anthropic) strictly as a mentor and code reviewer while building it. It explained concepts, reviewed my code and generated the custom register-check test ROMs. The IBM logo ROM is a widely distributed CHIP-8 test program.

References: Cowgod's *Chip-8 Technical Reference v1.0*; Tobias V. Langhoff, *Guide to making a CHIP-8 emulator*.
