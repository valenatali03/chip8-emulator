#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Memory
{
    uint8_t ram[4096];
    uint16_t pc;
    uint16_t I;
    uint8_t V[16];
} Memory;

uint8_t display[64 * 32] = {0};

uint16_t fetch(Memory *mem);

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Log("SDL initialized successfully!");

    SDL_Window *window = SDL_CreateWindow("chip-8-emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, 0);

    if (!window)
    {
        SDL_Log("Unable to initialize window", SDL_GetError());
        return 1;
    }

    SDL_Surface *window_surface = SDL_GetWindowSurface(window);

    if (!window_surface)
    {
        SDL_Log("Unable to obtain window surface", SDL_GetError());
    }

    SDL_Event e;
    while (SDL_PollEvent(&e) > 0)
    {
        SDL_UpdateWindowSurface(window);
    }

    bool keep_window_open = true;
    while (keep_window_open)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e) > 0)
        {
            switch (e.type)
            {
            case SDL_QUIT:
                keep_window_open = false;
                break;
            }

            SDL_UpdateWindowSurface(window);
        }
    }

    return 0;
}

uint16_t fetch(Memory *mem)
{
    uint16_t instruction = (mem->ram[mem->pc] << 8) | (mem->ram[mem->pc + 1])
}
