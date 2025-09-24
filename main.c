#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

typedef struct Memory
{
    uint8_t ram[4096];
    uint16_t pc;
    uint16_t I;
    uint8_t V[16];
} Memory;

typedef struct Stack
{
    int top;
    uint16_t s[1024];
} Stack;

const SDL_KeyCode keyboard[16] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v};

uint16_t fetch(Memory *mem);

void decode(uint16_t instruction, uint8_t *opcode, uint8_t *X, uint8_t *Y, uint16_t *NNN, uint8_t *NN, uint8_t *N);

void loadROM(char *fname, Memory *mem);

void push(Stack *stack, uint16_t elem);

uint16_t pull(Stack *stack);

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

    srand(time(NULL));

    int running = 1;
    SDL_Event event;

    Memory mem = {0};
    mem.pc = 0x200;

    loadROM(argv[1], &mem);

    uint16_t instruction;

    uint8_t opcode;
    uint8_t X;
    uint8_t Y;
    uint16_t NNN;
    uint8_t NN;
    uint8_t N;

    Stack *stack = malloc(sizeof(Stack));
    stack->top = 0;

    uint8_t keys[16] = {0};

    uint8_t display[64 * 32] = {0};

    bool key_pressed;

    while (running)
    {
        key_pressed = false;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                for (int i = 0; i < 16; i++)
                {
                    if (event.key.keysym.sym == keyboard[i])
                    {
                        keys[i] = (event.type == SDL_KEYDOWN) ? 1 : 0;
                        key_pressed = true;
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

            case 0x0EE:
                uint16_t pos = pull(stack);
                mem.pc = pos;
                break;

            default:
                break;
            }
            break;

        case 0x1:
            mem.pc = NNN;
            break;

        case 0x2:
            push(stack, mem.pc);
            mem.pc = NNN;
            break;

        case 0x3:
            if (mem.V[X] == NN)
            {
                mem.pc += 2;
            }
            break;

        case 0x4:
            if (mem.V[X] != NN)
            {
                mem.pc += 2;
            }
            break;

        case 0x5:
            if (mem.V[X] == mem.V[Y])
            {
                mem.pc += 2;
            }
            break;

        case 0x6:
            mem.V[X] = NN;
            break;

        case 0x7:
            mem.V[X] += NN;
            break;

        case 0x8:
            switch (N)
            {
            case 0x0:
                mem.V[X] = mem.V[Y];
                break;

            case 0x1:
                mem.V[X] |= mem.V[Y];
                break;

            case 0x2:
                mem.V[X] &= mem.V[Y];
                break;

            case 0x3:
                mem.V[X] ^= mem.V[Y];
                break;

            case 0x4:
                mem.V[0xF] = (mem.V[X] + mem.V[Y] > 0xFF) ? 1 : 0;
                mem.V[X] += mem.V[Y];
                break;

            case 0x5:
                mem.V[0xF] = (mem.V[X] >= mem.V[Y]) ? 1 : 0;
                mem.V[X] -= mem.V[Y];
                break;

            case 0x6:
                mem.V[X] = mem.V[Y];
                mem.V[0xF] = mem.V[X] & 1;
                mem.V[X] >>= 1;
                break;

            case 0x7:
                mem.V[0xF] = (mem.V[Y] >= mem.V[X]) ? 1 : 0;
                mem.V[X] = mem.V[Y] - mem.V[X];
                break;

            case 0xE:
                mem.V[X] = mem.V[Y];
                mem.V[0xF] = mem.V[X] & 1;
                mem.V[X] <<= 1;
                break;

            default:
                break;
            }
            break;

        case 0x9:
            if (mem.V[X] != mem.V[Y])
            {
                mem.pc += 2;
            }
            break;

        case 0xA:
            mem.I = NNN;
            break;

        case 0xB:
            mem.pc = NNN + mem.V[0];
            break;

        case 0xC:
            mem.V[X] = rand() & NN;
            break;

        case 0xD:
            uint8_t x = mem.V[X] % 64;
            uint8_t y = mem.V[Y] % 32;

            mem.V[0xF] = 0;

            for (int row = 0; row < N; row++)
            {
                uint8_t sprite_data = mem.ram[mem.I + row];

                for (int col = 0; col < 8; col++)
                {
                    uint8_t pixel = (sprite_data >> (7 - col)) & 1;
                    int index = (y + row) * 64 + (x + col);

                    if (pixel == 1)
                    {
                        if (display[index] == 1)
                        {
                            mem.V[0xF] = 1;
                        }
                        display[index] ^= 1;
                    }
                }
            }

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int i = 0; i < 64 * 32; i++)
            {
                if (display[i])
                {
                    SDL_Rect pixelRect = {(i % 64) * 10, (i / 64) * 10, 10, 10};
                    SDL_RenderFillRect(renderer, &pixelRect);
                }
            }

            break;

        case 0xE:
            switch (NN)
            {
            case 0x9E:
                if (keys[mem.V[X]] == 1)
                {
                    mem.pc += 2;
                }

                break;

            case 0xA1:
                if (keys[mem.V[X]] == 0)
                {
                    mem.pc += 2;
                }

                break;

            default:
                break;
            }
            break;

        case 0xF:
            switch (NN)
            {

            case 0x33:
                mem.ram[mem.I] = ((mem.V[X] / 10) / 10) % 10;
                mem.ram[mem.I + 1] = (mem.V[X] / 10) % 10;
                mem.ram[mem.I + 2] = mem.V[X] % 10;
                break;

            case 0x55:
                for (int i = 0; i <= X; i++)
                {
                    mem.ram[mem.I + i] = mem.V[i];
                }
                break;

            case 0x65:
                for (int i = 0; i <= X; i++)
                {
                    mem.V[i] = mem.ram[mem.I + i];
                }

                break;

            case 0x1E:
                mem.I += mem.V[X];
                break;

            case 0x0A:
                if (key_pressed)
                {
                    for (int i = 0; i < 16; i++)
                    {
                        mem.V[X] = (keys[i] == 1) ? keys[i] : 0;
                    }
                }
                else
                {
                    mem.pc -= 2;
                }

                break;
            default:
                break;
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

void loadROM(char *fname, Memory *mem)
{
    FILE *rom = fopen(fname, "rb");

    if (!rom)
    {
        printf("No se pudo abrir la rom\n");
        return;
    }

    fseek(rom, 0, SEEK_END);
    long size = ftell(rom);
    rewind(rom);

    if (size > (4096 - 0x200))
    { // Verificar que la ROM no sea demasiado grande
        printf("Error: ROM demasiado grande\n");
        fclose(rom);
        return;
    }

    fread(&mem->ram[0x200], size, 1, rom);
    fclose(rom);
}

void push(Stack *stack, uint16_t elem)
{
    stack->top++;
    stack->s[stack->top] = elem;
}

uint16_t pull(Stack *stack)
{
    u_int16_t elem = stack->s[stack->top];
    stack->top--;
    return elem;
}