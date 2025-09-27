#include <stdbool.h>
#include "include/instruction.h"

uint16_t fetch(Memory *mem, CPU *cpu)
{
    uint16_t instruction = (mem->ram[cpu->pc] << 8) | (mem->ram[cpu->pc + 1]);
    cpu->pc += 2;
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

void execute(uint8_t opcode, uint8_t X, uint8_t Y, uint16_t NNN, uint8_t NN, uint8_t N,
             CPU *cpu, Memory *mem, Stack *stack, uint8_t *keys, uint8_t *display, SDL_Renderer *renderer,
             bool waiting_for_key, int key_pressed, uint8_t waiting_register)
{
    switch (opcode)
    {
    case 0x0:
        switch (NNN)
        {
        case 0x0E0:
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            memset(display, 0, 64 * 32 * sizeof(uint8_t));
            break;

        case 0x0EE:
            uint16_t pos = pull(stack);
            cpu->pc = pos;
            break;

        default:
            break;
        }
        break;

    case 0x1:
        cpu->pc = NNN;
        break;

    case 0x2:
        push(stack, cpu->pc);
        cpu->pc = NNN;
        break;

    case 0x3:
        if (cpu->V[X] == NN)
        {
            cpu->pc += 2;
        }
        break;

    case 0x4:
        if (cpu->V[X] != NN)
        {
            cpu->pc += 2;
        }
        break;

    case 0x5:
        if (cpu->V[X] == cpu->V[Y])
        {
            cpu->pc += 2;
        }
        break;

    case 0x6:
        cpu->V[X] = NN;
        break;

    case 0x7:
        cpu->V[X] += NN;
        break;

    case 0x8:
        uint8_t aux;
        switch (N)
        {
        case 0x0:
            cpu->V[X] = cpu->V[Y];
            break;

        case 0x1:
            cpu->V[X] |= cpu->V[Y];
            break;

        case 0x2:
            cpu->V[X] &= cpu->V[Y];
            break;

        case 0x3:
            cpu->V[X] ^= cpu->V[Y];
            break;

        case 0x4:
            aux = cpu->V[X];
            cpu->V[X] += cpu->V[Y];
            cpu->V[0xF] = (aux + cpu->V[Y] > 0xFF) ? 1 : 0;
            break;

        case 0x5:
            aux = cpu->V[X];
            cpu->V[X] -= cpu->V[Y];
            cpu->V[0xF] = (aux >= cpu->V[Y]) ? 1 : 0;

            break;

        case 0x6:
            cpu->V[X] = cpu->V[Y];
            aux = cpu->V[X];
            cpu->V[X] >>= 1;
            cpu->V[0xF] = aux & 1;
            break;

        case 0x7:
            aux = cpu->V[X];
            cpu->V[X] = cpu->V[Y] - cpu->V[X];
            cpu->V[0xF] = (cpu->V[Y] >= aux) ? 1 : 0;
            break;

        case 0xE:
            cpu->V[X] = cpu->V[Y];
            aux = cpu->V[X];
            cpu->V[X] <<= 1;
            cpu->V[0xF] = (aux & 0x80) >> 7;
            break;

        default:
            break;
        }
        break;

    case 0x9:
        if (cpu->V[X] != cpu->V[Y])
        {
            cpu->pc += 2;
        }
        break;

    case 0xA:
        cpu->I = NNN;
        break;

    case 0xB:
        cpu->pc = NNN + cpu->V[0];
        break;

    case 0xC:
        cpu->V[X] = rand() & NN;
        break;

    case 0xD:
        uint8_t x = cpu->V[X] % 64;
        uint8_t y = cpu->V[Y] % 32;

        cpu->V[0xF] = 0;

        for (int row = 0; row < N; row++)
        {
            uint8_t sprite_data = mem->ram[cpu->I + row];

            for (int col = 0; col < 8; col++)
            {
                uint8_t pixel = (sprite_data >> (7 - col)) & 1;
                int index = (y + row) * 64 + (x + col);

                if (pixel == 1)
                {
                    if (display[index] == 1)
                    {
                        cpu->V[0xF] = 1;
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
            if (keys[cpu->V[X]])
            {
                cpu->pc += 2;
            }

            break;

        case 0xA1:
            if (!keys[cpu->V[X]])
            {
                cpu->pc += 2;
            }

            break;

        default:
            break;
        }
        break;

    case 0xF:
        switch (NN)
        {
        case 0x07:
            cpu->V[X] = cpu->delay_timer;
            break;

        case 0x15:
            cpu->delay_timer = cpu->V[X];
            break;

        case 0x18:
            cpu->sound_timer = cpu->V[X];
            break;

        case 0x33:
            mem->ram[cpu->I] = ((cpu->V[X] / 10) / 10) % 10;
            mem->ram[cpu->I + 1] = (cpu->V[X] / 10) % 10;
            mem->ram[cpu->I + 2] = cpu->V[X] % 10;
            break;

        case 0x55:
            for (int i = 0; i <= X; i++)
            {
                mem->ram[cpu->I + i] = cpu->V[i];
            }
            break;

        case 0x65:
            for (int i = 0; i <= X; i++)
            {
                cpu->V[i] = mem->ram[cpu->I + i];
            }

            break;

        case 0x1E:
            cpu->I += cpu->V[X];
            break;

        case 0x0A:
            if (!waiting_for_key)
            {
                waiting_for_key = true;
                waiting_register = X;
                key_pressed = -1;
                cpu->pc -= 2;
            }
            else
            {
                if (key_pressed >= 0 && !keys[key_pressed])
                {
                    waiting_for_key = false;
                }

                for (int i = 0; i < 16; i++)
                {
                    if (keys[i])
                    {
                        cpu->V[waiting_register] = i;
                        key_pressed = i;
                    }
                }

                if (waiting_for_key)
                {
                    cpu->pc -= 2;
                }
            }
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}