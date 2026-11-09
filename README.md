# CHIP-8 Emulator in C

A fully functional, accurate CHIP-8 emulator written in C using the [Raylib](https://www.raylib.com/) library. 

This project accurately emulates the original CHIP-8 instruction set, featuring a fixed-timestep execution loop, procedural sound generation, and support for historical hardware quirks.

## Features

* **Complete Instruction Set:** Implements all 34 standard CHIP-8 opcodes accurately.
* **Procedural Audio:** Generates the classic CHIP-8 "beep" using a procedurally generated square wave (no external `.wav` files required).
* **Accurate Timing:** Uses an accumulator-based fixed timestep loop to decouple the CPU speed from the visual framerate (60 FPS).
* **Historical Accuracy (Quirks):** Includes a toggle for original COSMAC VIP (1977) behaviors (e.g., bit shifting and memory index increments), making it highly compatible with older ROMs.
* **QWERTZ Layout:** Key mappings are optimized for central European QWERTZ keyboards.

## Controls

The original CHIP-8 utilized a 16-key hexadecimal keypad (0-F). This emulator maps them to the left side of a modern QWERTZ keyboard:

| Original Keypad | Emulator Keys (QWERTZ) |
| :---: | :---: |
| `1` `2` `3` `C` | `1` `2` `3` `4` |
| `4` `5` `6` `D` | `Q` `W` `E` `R` |
| `7` `8` `9` `E` | `A` `S` `D` `F` |
| `A` `0` `B` `F` | `Y` `X` `C` `V` |

## Configuration

You can easily tweak the emulator's behavior by modifying the global variables at the top of `main.c`:

* `rom_name`: Path to the ROM file you want to load (default: `"rom.ch8"`).
* `COSMAC_VIP_1977`: Set to `true` to use original 1977 quirks (recommended for most classic games). Set to `false` for SUPER-CHIP/modern behavior.
* `CPU_CYCLE_PER_FRAME`: Adjusts the emulation speed (default: `12` instructions per 60Hz frame).
* `beep_frequency`: Changes the pitch of the procedural sound (default: `440` Hz).
* `PIXEL_SCALE`: Adjusts the window size (default: `10`, making the 64x32 display 640x320 pixels).

## Building from Source

### Prerequisites
* A standard C compiler (GCC, Clang, or MSVC)
* [Raylib](https://www.raylib.com/) installed and linked

### Compilation (Example using GCC)
To compile the emulator from the terminal, run:

```bash
gcc main.c -o chip8.exe -lraylib -lopengl32 -lgdi32 -lwinmm
```
*(Note: Linker flags may vary depending on your operating system).*

## Usage
Place your target CHIP-8 ROM in the same directory as the executable, rename it to `rom.ch8` (or change the `rom_name` variable in the code), and launch the application!
