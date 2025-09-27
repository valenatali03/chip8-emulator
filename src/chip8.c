#include <time.h>
#include "include/chip8.h"

const SDL_Keycode keyboard[16] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v};

void load_rom(char *fname, Memory *mem)
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
    uint16_t elem = stack->s[stack->top];
    stack->top--;
    return elem;
}

void update_timers(CPU *cpu)
{
    static clock_t last_tick = 0;
    clock_t now = clock();

    if ((double)(now - last_tick) / CLOCKS_PER_SEC >= (1.0 / 60.0))
    {
        if (cpu->delay_timer > 0)
            cpu->delay_timer--;
        if (cpu->sound_timer > 0)
            cpu->sound_timer--;
        last_tick = now;
    }
}