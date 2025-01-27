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

void decode(uint16_t instruction, uint8_t *opcode, uint8_t *X, uint8_t *Y, uint16_t *NNN, uint8_t *NN, uint8_t *N);

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

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (!renderer)
    {
        printf("Error al crear el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    int running = 1;
    SDL_Event event;

    Memory *mem = {0};
    mem->pc = 0x200;

    uint16_t instruction;

    uint8_t opcode;
    uint8_t X;
    uint8_t Y;
    uint16_t NNN;
    uint8_t NN;
    uint8_t N;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }

        instruction = fetch(mem);
        decode(instruction, opcode, X, Y, NNN, NN, N);

        switch (opcode)
        {
        case 0x0:
            switch (NNN)
            {
            case 0x0E0:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                break;

            default:
                break;
            }
            break;

        case 0x1:
            mem->pc = NNN;
            break;

        case 0x6:
            mem->V[X] = NN;
            break;

        case 0x7:
            mem->V[X] += NN;
            break;

        case 0xA:
            mem->I = NNN;
            break;

        case 0xD:
            break;

        default:
            break;
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

uint16_t fetch(Memory *mem)
{
    uint16_t instruction = (mem->ram[mem->pc] << 8) | (mem->ram[mem->pc + 1]);
    mem->pc += 2;
    return instruction;
}

void decode(uint16_t instruction, uint8_t *opcode, uint8_t *X, uint8_t *Y, uint16_t *NNN, uint8_t *NN, uint8_t *N)
{
    *opcode = instruction >> 12;
    *X = (instruction & 0x0F00) >> 8;
    *Y = (instruction & 0x00F0) >> 4;
    *NNN = instruction & 0x0FFF;
    *NN = instruction & 0x00FF;
    *N = instruction & 0x000F;
}
