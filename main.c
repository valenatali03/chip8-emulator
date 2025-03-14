#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define ROM_NAME "IBMLogo.ch8"

typedef struct Memory
{
    uint8_t ram[4096];
    uint16_t pc;
    uint16_t I;
    uint8_t V[16];
} Memory;

uint8_t display[64 * 32] = {0};

const SDL_KeyCode keyboard[16] = {
     SDLK_1, SDLK_2, SDLK_3, SDLK_4,
     SDLK_q, SDLK_w, SDLK_e, SDLK_r,
     SDLK_a, SDLK_s, SDLK_d, SDLK_f,
     SDLK_z, SDLK_x, SDLK_c, SDLK_v
    };

uint16_t fetch(Memory *mem);

void decode(uint16_t instruction, uint8_t *opcode, uint8_t *X, uint8_t *Y, uint16_t *NNN, uint8_t *NN, uint8_t *N);

void loadROM(Memory *mem);

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
        SDL_Log("Unable to initialize window: %s", SDL_GetError());
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

    Memory mem = {0};
    mem.pc = 0x200;

    loadROM(&mem);

    uint16_t instruction;

    uint8_t opcode;
    uint8_t X;
    uint8_t Y;
    uint16_t NNN;
    uint8_t NN;
    uint8_t N;

    uint8_t keys[16] = {0};

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == keyboard[i]) {
                        keys[i] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                    }
                }
            }
        }

        instruction = fetch(&mem);
        decode(instruction, &opcode, &X, &Y, &NNN, &NN, &N);

        switch (opcode)
        {
        case 0x0:
            switch (NNN)
            {
            case 0x0E0:
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);
                memset(display, 0, sizeof(display));
                break;

            default:
                break;
            }
            break;

        case 0x1:
            mem.pc = NNN;
            break;

        case 0x6:
            mem.V[X] = NN;
            break;

        case 0x7:
            mem.V[X] += NN;
            break;

        case 0xA:
            mem.I = NNN;
            break;

        case 0xD:
            uint8_t x = mem.V[X] % 64;
            uint8_t y = mem.V[Y] % 32;

            mem.V[0xF] = 0;

            for(int row = 0; row < N; row++) {
                uint8_t sprite_data = mem.ram[mem.I + row];

                for (int col = 0; col < 8; col++)
                {
                    uint8_t pixel = (sprite_data >> (7 - col)) & 1;
                    int index = (y + row) * 64 + (x + col);

                    if (pixel == 1) {
                        if (display[index] == 1) {
                            mem.V[0xF] = 1;
                        }
                        display[index] ^= 1;
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int i = 0; i < 64*32; i++)
            {
                if(display[i]) {
                    SDL_Rect pixelRect = {(i % 64) * 10, (i/64) * 10, 10, 10};
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
            }
            
            break;

        default:
            break;
        }

        SDL_Delay(2);
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

void loadROM(Memory *mem)
{
    FILE *rom = fopen(ROM_NAME, "rb");

    if (!rom) {
        printf("No se pudo abrir la rom\n");
        return;
    }

    fseek(rom, 0, SEEK_END);
    long size = ftell(rom);
    rewind(rom);

    if (size > (4096 - 0x200)) {  // Verificar que la ROM no sea demasiado grande
        printf("Error: ROM demasiado grande\n");
        fclose(rom);
        return;
    }

    fread(&mem->ram[0x200], size, 1, rom);
    fclose(rom);
}