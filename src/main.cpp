#include "./Platform/Platform.h"
#include "./core/Chip8.h"
#include <chrono>
#include <iostream>

// Chip8 games run 600-700inst per second
// 60 frame per sec and  10 inst per frame
const int INST_PER_FRAME = 10;
const float MS_PER_FRAME = 1000.0f / 60.0f;

int main(int argc, char **argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <Scale> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }
    int vdoScale = std::stoi(argv[1]);
    int cycleDelay = std::stoi(argv[2]);
    char const *romfilePath = argv[3];

    Platform platform("Chip8 Emulator", VIDEO_WIDTH * vdoScale, VIDEO_HEIGHT * vdoScale,
                      VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 chip8;
    chip8.LoadRom(romfilePath);

    int videoPitch = sizeof(chip8.screenData[0]) * VIDEO_WIDTH;
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit) {
        quit = platform.ProcessInput(chip8.keypad);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime -
                                                                                   lastCycleTime)
                       .count();
        if (dt >= MS_PER_FRAME) {

            lastCycleTime = currentTime;

            // only fetch decode and excute 10 per frame
            for (int i = 0; i < INST_PER_FRAME; ++i) {
                chip8.Cycle();
            }

            // If there is any delay set ,decrement it by 60Hz rate
            if (chip8.delayTimer > 0) {
                --chip8.delayTimer;
            }
            if (chip8.soundTimer > 0) {
                --chip8.soundTimer;
            }

            // Update the screen data at 60Hz
            platform.Update(chip8.screenData, videoPitch);
        }
    }
    return 0;
}
