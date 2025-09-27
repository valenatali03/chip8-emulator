#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct CPU
{
    uint16_t pc;
    uint16_t I;
    uint8_t V[16];
    uint8_t delay_timer;
    uint8_t sound_timer;
} CPU;

typedef struct Memory
{
    uint8_t ram[4096];
} Memory;

typedef struct Stack
{
    int top;
    uint16_t s[1024];
} Stack;

extern const SDL_Keycode keyboard[16];

void load_rom(char *fname, Memory *mem);

void push(Stack *stack, uint16_t elem);

uint16_t pull(Stack *stack);

void update_timers(CPU *cpu);

#endif