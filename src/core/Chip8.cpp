#include "Chip8.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iosfwd>

// This is the main constructor to create the Chip8 CPU
// Initialize the fontset within the desired range and then
// Since the Chip8 has random number generate instructions
// I have to mimic that using the c++ standard library and used it as
// Initialize list when the function start
Chip8::Chip8()
    : delayTimer(0), soundTimer(0), _stackPointer(0), opcode(0), _addressI(0),
      _programCounter(0x200), _startAddress(0x200),
      randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    const unsigned int FONTSET_SIZE = 80;
    const unsigned int FONTSET_START_ADDRESS = 0x50;
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
        _memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }
    // Only store the address pointer to function
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xnn;
    table[0x4] = &Chip8::OP_4xnn;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xnn;
    table[0x7] = &Chip8::OP_7xnn;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxnn;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF;

    // fill in the unused slot first
    for (int i = 0; i <= 0xF; i++) {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
    }
    // overwrite the opcode slot : table0
    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    // overwrite the opcode slot : table8
    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    // overwrite the opcode slot :tableE
    tableE[0xE] = &Chip8::OP_Ex9E;
    tableE[0x1] = &Chip8::OP_ExA1;

    // fill the unused slots for tableF
    for (int i = 0; i <= 0x65; i++) {
        tableF[i] = &Chip8::OP_NULL;
    }

    // overwrite the opcode slot :tableF
    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;
};

// Emulation of cycle
void Chip8::Cycle() {
    // fetch two bytes from memory to form 16bit opcode
    opcode = _memory[_programCounter] << 8u | _memory[_programCounter + 1];
    _programCounter += 2;
    // take out the first byte and shift  right by 12bit
    // e.g: 1010 0000 0000 0000 = 0xA000;
    //      0000 0000 0000 0000 = 0x000A;
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    if (delayTimer > 0) {
        --delayTimer;
    }
    if (soundTimer > 0) {
        --soundTimer;
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
            _memory[_startAddress + i] = buffer[i];
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
    byte val = (opcode & 0x00FFu);
    _registers[Vx] += val;
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
                if (*screenPixel == 0xFFFFFFFF) { // collision detect
                    _registers[0xF] = 1;
                }
                // xor if on ?? off : on;
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

// Skip to next instructions if key with Vx is pressed :Ex9E
void Chip8::OP_Ex9E() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte key = _registers[Vx];
    if (key < 16 && keypad[key]) {
        _programCounter += 2;
    }
};

// SKip to next instructions if key with Vx is not pressed :ExA1
void Chip8::OP_ExA1() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte key = _registers[Vx];

    if (key < 16 && !keypad[key]) {
        _programCounter += 2;
    }
}

// Set the Vx value to delay timer: Fx07
void Chip8::OP_Fx07() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    _registers[Vx] = delayTimer;
}

// Awaits as long as the key is not pressed: Fx0A
void Chip8::OP_Fx0A() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    for (int key = 0; key < 16; ++key) {
        if (keypad[key]) {
            _registers[Vx] = key;
            return;
        }
    }
    _programCounter -= 2;
}
// Set the delaytime to Vx :Fx15
void Chip8::OP_Fx15() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    delayTimer = _registers[Vx];
}

// Set the soundTimer to Vx: Fx18
void Chip8::OP_Fx18() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    soundTimer = _registers[Vx];
}

// Plus the memory pointer with Vx value: Fx1E
void Chip8::OP_Fx1E() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    _addressI += _registers[Vx];
}

// Set the fontset_start_addr to the Vx digit :Fx29
void Chip8::OP_Fx29() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte digit = _registers[Vx];
    // the fontset_start_addr = 0x50;
    _addressI = 0x50 + (5 * digit);
}

// Store Vx value's the hundred-digit at I, and 10th-digit at I+1 and single-digit t I+2:Fx33
void Chip8::OP_Fx33() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    byte value = _registers[Vx];
    _memory[_addressI + 2] = value % 10;
    value /= 10;
    _memory[_addressI + 1] = value % 10;
    value /= 10;
    _memory[_addressI] = value % 10;
};

// Store the V0 to Vx in memory :Fx55
// Regsiter dump
void Chip8::OP_Fx55() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    for (byte i = 0; i <= Vx; ++i) {
        _memory[_addressI + i] = _registers[i];
    }
}

// Store V0 to Vx with memory values :Fx65
// Register Load
void Chip8::OP_Fx65() {
    byte Vx = (opcode & 0x0F00u) >> 8u;
    for (byte i = 0; i <= Vx; ++i) {
        _registers[i] = _memory[_addressI + i];
    }
};

// Table Map Implementation

void Chip8::Table0() { ((*this).*(table0[opcode & 0x000Fu]))(); }
void Chip8::Table8() { ((*this).*(table8[opcode & 0x000Fu]))(); }
void Chip8::TableE() { ((*this).*(tableE[opcode & 0x000Fu]))(); }
void Chip8::TableF() { ((*this).*(tableF[opcode & 0x00FFu]))(); } // take the least last two hex
void Chip8::OP_NULL() {};
