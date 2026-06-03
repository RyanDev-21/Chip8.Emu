#ifndef PLATFORM_H
#define PLATFORM_H
#include <SDL2/SDL.h>
class Platform {
  public:
    Platform(char const *title, int windowWidth, int windowHeight, int textureWidth,
             int textureHeight) {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING,
                                    textureWidth, textureHeight);
    }
    ~Platform() {
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void Update(void const *buffer, int pitch);
    bool ProcessInput(uint8_t *keys);

  private:
    SDL_Window *window{};
    SDL_Renderer *renderer{};
    SDL_Texture *texture{};
};

#endif
