#ifndef Chip8_H
#define Chip8_H

#include <cstdint>
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
    byte _memory[4096]{};
    byte _stack[16]{};

    byte delayTimer;
    byte soundTimer;
    byte _stackPointer;
    word opcode;
    word _addressI;
    word _programCounter;
    const word _startAddress;
    std::default_random_engine randGen;
    std::uniform_int_distribution<byte> randByte;

    // Alias for  member pointer function
    typedef void (Chip8::*Chip8Func)();
    // the Chip8(1970) has only up to E
    // e.g::::: table[0xE+1];
    // just to prevent  crashing for some bug instructons code and 1990 super chip8 added
    // instructions
    Chip8Func table[0xF + 1];
    Chip8Func table0[0xF + 1];
    Chip8Func table8[0xF + 1];
    Chip8Func tableE[0xF + 1];
    Chip8Func tableF[0x65 + 1];

    void Table0();
    void Table8();
    void TableE();
    void TableF();
    void OP_NULL();

  public:
    uint32_t screenData[32 * 64]{}; // Make this  a 32bit for sdl
    byte keypad[16]{};

    Chip8();
    void Cycle();
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
    void OP_Ex9E();
    void OP_ExA1();
    void OP_Fx07();
    void OP_Fx0A();
    void OP_Fx15();
    void OP_Fx18();
    void OP_Fx1E();
    void OP_Fx29();
    void OP_Fx33();
    void OP_Fx55();
    void OP_Fx65();
};

#endif
