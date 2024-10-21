#ifndef E_TPYEDEFS_H
#define E_TYPEDEFS_H

#include "SDL3/SDL.h"

// easier to keep everything collected
typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

// allows for customizing
typedef struct {
    uint32_t window_width;      
    uint32_t window_height;
    uint32_t fcolor;            // fg color RGBA8888
    uint32_t bcolor;            // bg color RGBA8888
    uint32_t scaler;            // scale window size up
} config_t;

// chip8 states
typedef enum{
    QUIT,                       // shut down
    RUNNING,                    // execute
    PAUSED,                     // NOP
    LOADING,                    // load data
} emualtor_state_t;

// opcode defs
typedef struct {
    uint16_t opcode;
    uint16_t NNN;               // 12 bit addr/const
    uint8_t NN;                 // 8 bit const
    uint8_t N;                  // 4 bit const
    uint8_t X;                  // 4 bit reg ID
    uint8_t Y;                  // 4 bit reg ID
} instruction_t;

// chip8 layout
typedef struct {
    emualtor_state_t state;     // is chip8 running?    4B
    uint8_t ram[4096];          // chip8 ram            2B
    bool display[64*32];        // original rez         1B
    uint16_t stack[12];         // subroutines          4B
    uint8_t V[16];              // data register V0-VF  2B
    uint16_t I;                 // index                4B
    uint16_t PC;                // program counter      4B
    uint16_t *SP;               // stack pointer        4B
    uint8_t delay_timer;        // vx | (60Hz > 0)      2B
    uint8_t sound_timer;        // ^ will play sound    2B
    bool keys[16];              // 0x0-0xF              2B
    char *rom_name;             // current rom          1B
    instruction_t instruction;  // current instruction  
} chip8_t;

#endif
