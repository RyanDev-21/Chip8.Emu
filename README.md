# CHIP-8 Emulator

A CHIP-8 emulator/interpreter written in C++17 using SDL2.

This project follows [Austin Morlan's excellent CHIP-8 emulator guide](https://austinmorlan.com/posts/chip8_emulator/) — a great resource for anyone looking to learn about emulation development.

The main thing I changed was the timing. Austin's version uses a delay argument to control how fast instructions run, but I wasn't entirely sure about that approach — the games felt off and I couldn't quite get it right — so I switched to a fixed 60Hz frame rate with 10 instructions per frame, targeting ~600 instructions/sec (the commonly cited range is 500–700 IPS for CHIP-8).

## Building

### Dependencies

- [CMake](https://cmake.org/) >= 3.16
- C++17 compiler (GCC, Clang, MSVC, etc.)
- [SDL2](https://www.libsdl.org/) development libraries

### Linux / macOS

```bash
# Install SDL2 (Ubuntu/Debian)
sudo apt install libsdl2-dev

# Install SDL2 (Fedora)
sudo dnf install SDL2-devel

# Install SDL2 (macOS)
brew install sdl2

# Build
mkdir build && cd build
cmake ..
make
```

### Windows

Using MSYS2 with MinGW:

```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
make
```

Using MSVC with vcpkg:

```powershell
vcpkg install sdl2
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

## Usage

```bash
./Chip8 <Scale> <ROM>
```

| Argument | Description |
|----------|-------------|
| `Scale`  | Window scale multiplier (e.g., `10` for 640×320) |
| `ROM`    | Path to a CHIP-8 ROM file |

### Examples

```bash
./Chip8 10 c8games/PONG
./Chip8 10 ~/roms/chip8/INVADERS
./Chip8 10 /home/user/games/BLITZ
./Chip8 10 "C:\Users\me\roms\TETRIS"
```

### Key Mapping

The original CHIP-8 hex keypad maps to your keyboard as follows:

```
CHIP-8 Keypad    Keyboard
+-+-+-+-+        +-+-+-+-+
|1|2|3|C|        |1|2|3|4|
+-+-+-+-+        +-+-+-+-+
|4|5|6|D|        |Q|W|E|R|
+-+-+-+-+   =>   +-+-+-+-+
|7|8|9|E|        |A|S|D|F|
+-+-+-+-+        +-+-+-+-+
|A|0|B|F|        |Z|X|C|V|
+-+-+-+-+        +-+-+-+-+
```

Press `ESC` to quit.
