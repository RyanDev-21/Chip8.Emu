#include "./Platform/Platform.h"
#include "./core/Chip8.h"
#include <chrono>
#include <iostream>
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

    int videoPitch = sizeof(chip8.screenData[0] * VIDEO_WIDTH);
    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    if (!quit) {
        quit = platform.ProcessInput(chip8.keypad);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime -
                                                                                   lastCycleTime)
                       .count();
        if (dt > cycleDelay) {
            lastCycleTime = currentTime;
            chip8.Cycle();
            platform.Update(chip8.screenData, videoPitch);
        }
    }
    return 0;
}
