#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "src/include/chip8.h"
#include "src/include/instruction.h"

bool waiting_for_key = false;
int key_pressed = -1;
uint8_t waiting_register;

const int CYCLES_PER_FRAME = 10;
const int FRAME_DELAY_MS = 1000 / 60;

void draw_screen(SDL_Renderer *renderer, uint8_t *display);

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
    for (int i = 0; i < 80; i++)
    {
        mem.ram[0x050 + i] = font_set[i];
    }

    CPU cpu = {0};
    cpu.pc = 0x200;
    cpu.delay_timer = 0;
    cpu.sound_timer = 0;

    load_rom(argv[1], &mem);

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

    uint32_t last_time = SDL_GetTicks();

    while (running)
    {
        uint32_t current_time = SDL_GetTicks();
        if (current_time - last_time >= FRAME_DELAY_MS)
        {
            last_time = current_time;

            for (int i = 0; i < CYCLES_PER_FRAME; i++)
            {
                instruction = fetch(&mem, &cpu);
                decode(instruction, &opcode, &X, &Y, &NNN, &NN, &N);
                execute(opcode, X, Y, NNN, NN, N, &cpu, &mem, stack, keys, display, renderer,
                        &waiting_for_key, &key_pressed, &waiting_register);
            }

            update_timers(&cpu);
            if (draw_flag)
            {
                draw_screen(renderer, display);
                draw_flag = false;
            }
        }

        update_timers(&cpu);

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
                    }
                }
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

void draw_screen(SDL_Renderer *renderer, uint8_t *display)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    int scale = 10;
    for (int py = 0; py < 32; py++)
    {
        for (int px = 0; px < 64; px++)
        {
            if (display[py * 64 + px])
            {
                SDL_Rect r = {px * scale, py * scale, scale, scale};
                SDL_RenderFillRect(renderer, &r);
            }
        }
    }
    SDL_RenderPresent(renderer);
}
