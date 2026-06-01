#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iosfwd>
#include <random>
// Screen width and height
inline constexpr unsigned int VIDEO_WIDTH = 64;
inline constexpr unsigned int VIDEO_HEIGHT = 32;

// Declartion for custom type
// byte == 8 bit
// word == 16 bit
typedef uint8_t byte;
typedef uint16_t word;

// Chip8 internal
class Chip8 {
  private:
    byte _registers[16]{};
    byte _memory[0xFFF]{};
    byte _stack[16]{};
    byte _stackPointer{};
    byte delayTimer{};
    byte soundTimer{};
    byte keypad[16]{};
    uint32_t screenData[32 * 64]{}; // Make this  a 32bit for sdl
    word opcode{};
    word _addressI{};
    word _programCounter{};
    const word _startAddress = 0x200;
    std::default_random_engine randGen;
    std::uniform_int_distribution<byte> randByte;

  public:
    Chip8();
    void LoadRom(char const *filename);
    void OP_00E0();
    void OP_00EE();
    void OP_1nnn();
    void OP_2nnn();
    void OP_3xnn();
    void OP_4xnn();
    void OP_5xy0();
    void OP_6xnn();
    void OP_7xnn();
    void OP_8xy0();
    void OP_8xy1();
    void OP_8xy2();
    void OP_8xy3();
    void OP_8xy4();
    void OP_8xy5();
    void OP_8xy6();
    void OP_8xy7();
    void OP_8xyE();
    void OP_9xy0();
    void OP_Annn();
    void OP_Bnnn();
    void OP_Cxnn();
    void OP_Dxyn();
};
// This is the main constructor to create the Chip8 CPU
// Initialize the fontset within the desired range and then
// Since the Chip8 has random number generate instructions
// I have to mimic that using the c++ standard library and used it as
// Initialize list when the function start
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    word pc = _startAddress;
    const unsigned int FONTSET_SIZE = 80;
    const unsigned int FONTSET_SIZE_ADDRESS = 0x50;
    randByte = std::uniform_int_distribution<byte>(0, 255U);
    // this is nothing but a dot to form letters
    byte fontset[FONTSET_SIZE] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < FONTSET_SIZE; i++) {
        _memory[FONTSET_SIZE_ADDRESS + 1] = fontset[i];
    }
};

// this load the rom file at the predefined legal address range
void Chip8::LoadRom(char const *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        char *buffer = new char[size];
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();
        for (int i = 0; i < size; i++) {
            _memory[_startAddress + 1] = buffer[i];
        }

        delete[] buffer;
    }
}

// Instructons for opcode

// To Clear Screen
void Chip8::OP_00E0() { memset(screenData, 0, sizeof(screenData)); }

// Return From subroutine
void Chip8::OP_00EE() {
    --_stackPointer;
    _programCounter = _stack[_stackPointer];
}

// Jump to Address Location:1NNN
void Chip8::OP_1nnn() {
    word addr = opcode & 0x0FFF;
    _programCounter = addr;
}

// Call Address :2NNN
void Chip8::OP_2nnn() {
    word addr = opcode & 0x0FFF;
    _stack[_stackPointer] = _programCounter;
    ++_stackPointer;
    _programCounter = addr;
}

// Skip next instruction if equal:3xkk
// Vx == kk : skip
void Chip8::OP_3xnn() {
    byte kk = opcode & 0x00FFu;
    byte Vx = (opcode & 0x0F00u) >> 8u;
    if (_registers[Vx] == kk) {
        _programCounter += 2;
    }
}

// Skip next instruction if not equal:4xkk
// Vx != kk : skip
void Chip8::OP_4xnn() {
    byte kk = opcode & 0x00FFu;
    byte Vx = (opcode & 0x0F00u) >> 8u;
    if (_registers[Vx] != kk) {
        _programCounter += 2;
    }
}

// Skip next instructoin if equal:5xy0
// vx == vk : skip
void Chip8::OP_5xy0() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    if (_registers[Vx] == _registers[Vy]) {
        _programCounter += 2;
    }
}

// Set instruction to vx: 6xnn
// vx = nn
void Chip8::OP_6xnn() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte inst = (opcode & 0x00FFu);
    _registers[Vx] = inst;
}

// Add the byte to the register:7xnn
// vx +=nn
void Chip8::OP_7xnn() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte byte = (opcode & 0x00FFu);
    _registers[Vx] += byte;
}

// Assign Vy to Vx : 8xy0
// Vx = Vy
void Chip8::OP_8xy0() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    _registers[Vx] = _registers[Vy];
}

// Assign the Vy | Vx into Vx : 8xy1
// Vx = Vx | Vy
void Chip8::OP_8xy1() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    _registers[Vx] |= _registers[Vy];
}

// Assign the Vy & Vx into Vx : 8xy2
// Vx = Vx & Vy
void Chip8::OP_8xy2() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    _registers[Vx] &= _registers[Vy];
}

// Assign the Vy ^(xor) Vx into Vx : 8xy3
// Vx = Vx ^ Vy
void Chip8::OP_8xy3() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    _registers[Vx] ^= _registers[Vy];
}

// Assign the Vx + Vy into Vx :8xy4
// Vx = Vx + Vy
// when overflow set Vf = 1 : 0
void Chip8::OP_8xy4() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    byte sum = _registers[Vx] + _registers[Vy];
    if (sum > 255u) {
        _registers[0xF] = 1;
    } else {
        _registers[0xF] = 0;
    }
    _registers[Vx] = sum & 0xFFu;
}

// Assign the Vx - Vy into Vx:8xy5
// Vx = Vx - Vy
// when underflow set Vf = 0 : 1
void Chip8::OP_8xy5() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    if (_registers[Vx] > _registers[Vy]) {
        _registers[0xF] = 1;
    } else {
        _registers[0xF] = 0;
    }
    _registers[Vx] -= _registers[Vy];
}

// Store the least significant bit in Vf and then shift the vx by 1 right: 8xy6
void Chip8::OP_8xy6() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    _registers[0xF] = _registers[Vx] & 0x1u; // this takes out the last bit
    _registers[Vx] >>= 1u;
}

// Assign the Vy - Vx into Vx :8xy7
// Vx = Vy - Vx
// when there is underflow Vf =  0: 1
void Chip8::OP_8xy7() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    if (_registers[Vy] >= _registers[Vx]) {
        _registers[0xF] = 0;
    } else {
        _registers[0xF] = 1;
    }
    _registers[Vx] = _registers[Vy] - _registers[Vx];
}

// Store the most significant bit in Vf and then shift the Vx by 1 left :8xyE
void Chip8::OP_8xyE() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    _registers[0xF] =
        (_registers[Vx] & 0x80u) >> 7u; // take out the last bit and then shift to the start
    _registers[Vx] <<= 1u;
}

// Skip the next instructions if Vx != Vy:9xy0
void Chip8::OP_9xy0() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    if (_registers[Vx] == _registers[Vy]) {
        _programCounter += 2;
    };
};

// Set the addressPointer to given addr :Annn
void Chip8::OP_Annn() {
    word addr = opcode & 0x0FFFu;
    _addressI = addr;
}

// Set the program counter to V0 + nnn : Bnnn
void Chip8::OP_Bnnn() {
    word addr = opcode & 0x0FFFu;
    _programCounter = _registers[0x0] + addr;
}

// Set the Vx = rang() & NNN :Cxnn
void Chip8::OP_Cxnn() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    word addr = opcode & 0x00FFu;
    _registers[Vx] = randByte(randGen) & addr;
}

// Draw the corrdinate of Vx, Vy with height n : Dxyn
void Chip8::OP_Dxyn() {

    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte Vy = (opcode & 0x00F0u) >> 4u;
    byte height = (opcode & 0x000Fu);

    // has to modulo the width and heigth to wrap exceeding width and height
    byte xPos = _registers[Vx] % VIDEO_WIDTH;
    byte yPos = _registers[Vy] % VIDEO_HEIGHT;

    _registers[0xF] = 0; // no collision
    for (int row = 0; row < height; ++row) {
        // take out the each byte from the memory
        byte spriteByte = _memory[_addressI + row];
        // standard sprite width is 8
        for (int col = 0; col < 8; ++col) {
            // take out the bit of the sprite byte(pixel)
            byte spritePixel = spriteByte & (0x80u >> col);
            // calc x and y pos
            byte currentY = (yPos + row) % VIDEO_HEIGHT;
            byte curretnX = (xPos + col) % VIDEO_WIDTH;
            // calc screen pixel
            uint32_t *screenPixel = &screenData[currentY * VIDEO_WIDTH + curretnX];
            if (spritePixel) {
                if (*screenPixel == 0xFFFFFFFF) { // collision detect _registers[0xF] = 1;
                }
                // xor if on ?? off : on;
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}
