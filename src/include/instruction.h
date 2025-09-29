#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include "chip8.h"

extern bool draw_flag;

uint16_t fetch(Memory *mem, CPU *cpu);

void decode(uint16_t instruction, uint8_t *opcode, uint8_t *X, uint8_t *Y, uint16_t *NNN, uint8_t *NN, uint8_t *N);

void execute(uint8_t opcode, uint8_t X, uint8_t Y, uint16_t NNN, uint8_t NN, uint8_t N,
             CPU *cpu, Memory *mem, Stack *stack, uint8_t *keys, uint8_t *display, SDL_Renderer *renderer,
             bool *waiting_for_key, int *key_pressed, uint8_t *waiting_register);

#endif