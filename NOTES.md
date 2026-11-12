# CHIP-8 notes

## Next action

Keyboard input: EX9E (skip if key pressed), EXA1 (skip if key not pressed), FX0A (wait for a key). Map the PC keys `1234 / QWER / ASDF / ZXCV` to the 16-key hex keypad. Then run Pong.

## Rulings (ambiguous opcodes)

| Opcode | Decision | Reason |
|---|---|---|
| 8XY6 / 8XYE | Shift VX in place, ignore VY | Modern behaviour; most ROMs expect it |
| BNNN | Jump to NNN + V0 | Original behaviour; safest for classic ROMs |
| FX55 / FX65 | Leave I unchanged | Modern behaviour; most ROMs expect it |

## Other decisions

- The program counter advances by 2 during fetch, before execute. So 2NNN pushes the address of the next instruction, and a skip is one more +2.
- The stack pointer points at the next empty slot: push = write then increment, pop = decrement then read.
- In every flag-setting instruction, VF is written last, so the flag wins if X is F.
- DXYN: the start position wraps, and the sprite body clips at the edges.
- FX29 uses only the last hex digit of VX (`% 16`) so it stays inside the font.
- RNG: one `std::mt19937` member, seeded once from `std::random_device`.
- Opcode fields (X, Y, N, NN, NNN) are decoded once at the top of `cycle()`. NNN is 16-bit.
- Unknown opcodes all go through `printUnknownOpcode()` (address + opcode).
- 0NNN is not implemented, on purpose.
- **Frame loop:** each frame runs 10 instructions (about 600 a second), ticks the timers once (60 Hz), draws, then sleeps about 16 ms. The 10 is the speed knob if a game runs too fast or too slow.
- **Renderer:** builds the whole frame in one string (reserved up front), starting with cursor-home (`\033[H`), and prints it once with a flush. No full clear per frame, because that flickers.
- **Setup in `main`, once:** clear the screen (`\033[2J`) and hide the cursor (`\033[?25l`).
- **The console window must be at least 128 columns × 33 rows** (maximise it), or the frame scrolls.

## Known issues / to do

- **Timing runs slightly slow** (a bit over 1 second per second), because `sleep_for(16ms)` doesn't account for each frame's work time, and Windows sleep overshoots. Fix later: sleep *until* a deadline taken from `steady_clock`, moved forward 16.67 ms each frame.
- **The register-check test ROMs can't be read any more.** `main` now loops forever and the register dump is gone. Add a test mode (run N cycles, then dump) before re-running them.
- DXYN reads `mem[I + row]` with no bounds check. A buggy ROM could read past 4095.
- 64 and 32 are repeated in DXYN and `renderDisplay()`. Make them named constants (`DISPLAY_WIDTH`, `DISPLAY_HEIGHT`).
- Still to build: EX9E, EXA1, FX0A (keyboard), sound.

## Test ROM expected results

| ROM | How to run | Expected |
|---|---|---|
| IBM Logo | Frame loop | Logo on screen, perfectly still |
| timer-test | Frame loop | Counts 9 down to 0, one per second, then stays on 0 |
| control-flow-test | Test mode, 50 cycles | pc 234, sp 0, V10–V13 = 2, V14 = 3 |
| arithmetic-test | Test mode, 150 cycles | V14 = 21, V13 = 0, pc 2fa |
| misc-test | Test mode, 200 cycles | V14 = 25, V13 = 0, pc 32e, I 8c |

For the register-check ROMs, V13 = the ID of the last failed check (0 = all passed), and V14 = the number of checks passed. Each ROM has a listing file showing what each check tests.

## Progress log

- 8/27: started. 9/10: toy-machine lab (a 4-instruction interpreter I designed).
- 9/14: machine state + font in memory.
- 9/17: binary ROM loading, verified with a hex dump.
- 9/23: fetch/decode loop. 00E0 and 00EE implemented.
- 10/15: 1NNN, 6XNN, 7XNN, ANNN.
- 10/24: DXYN + terminal renderer. **IBM logo renders.**
- 10/25: control flow (2NNN, skips). Test ROMs moved into tests/.
- 10/29: the whole 8XY_ family, plus the field-decoding refactor.
- 11/6–11/8: BNNN, CXNN, FX1E, FX29, FX33, FX55, FX65, FX07, FX15, FX18.
- 11/10: README + NOTES added.
- 11/11: real-time frame loop (10 instructions, timer tick, draw, sleep). **The countdown runs.** Laptop set up as a second machine.
- 11/12: renderer rewritten: cursor-home + one string per frame + hidden cursor. Smooth, no flicker.

## Lessons (bugs that taught me something)

- **Storing a 12-bit address in a `uint8_t`** silently chopped off its top digit. The `static_cast` hid the narrowing error that would have caught it. Only `pc` in the register dump revealed it, so check every expected value, not just the pass count.
- **Declaring variables inside a `case`** needs braces around the case body. Otherwise a jump to a later label skips their initialisation, and the compiler refuses.
- **A `break` inside a nested switch** only exits the inner switch. The outer case needs its own.
- **Writing VF before the result** breaks when X is F: the result overwrites the flag. Copy the inputs first and write the flag last.
- **The subtraction flag is inverted:** VF = 1 means no borrow. Add and subtract don't mirror each other, so check each spec separately.
- **Operator placement:** `a * 5 % 16` applies the `%` to the product. Put the parentheses where you mean them.
- **`case 65` is decimal.** Opcode case labels must be written in hex (`0x65`).
- **Arithmetic on small types happens as `int`.** That's integer promotion, so `uint8_t + uint8_t` is an `int`, and brace-initialising a smaller type from it is a narrowing error.
- **MSVC Debug builds check `std::array` bounds** and stop on the bad index. Release builds don't, so develop in Debug.
- **`rng()` calls `operator()`** on an existing object. It isn't a constructor. The seed lives inside the engine object, not anywhere global.
- **Batch at the expensive boundary:** thousands of small `cout` writes per frame are slow and flicker, because each one is a trip to the console. Build in memory, then write once.
- **A `const` member function can't modify members.** A member scratch buffer either becomes a local variable or is marked `mutable`.
- **Escape codes are terminal commands, not text:** `\033[H` (cursor home), `\033[2J` (clear), `\033[?25l` (hide cursor), and they date back to the VT100. They only work if the window is big enough for what's printed.
- **Two machines:** `git pull` when you sit down, push when you get up. A project made in VS 2026 (toolset v145) won't build in VS 2022 (v143).
