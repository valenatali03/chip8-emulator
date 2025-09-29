#include <time.h>
#include "include/chip8.h"

const uint8_t font_set[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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
    static uint32_t last_timer = 0;
    uint32_t now = SDL_GetTicks();
    if (now - last_timer >= 16)
    {
        if (cpu->delay_timer > 0)
            cpu->delay_timer--;
        if (cpu->sound_timer > 0)
            cpu->sound_timer--;
        last_timer = now;
    }
}