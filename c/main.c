// system
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
// user
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
    instruction_t instruction;       // current instruction  
} chip8_t;


// init all required systems
bool init_sys(sdl_t *sdl, const config_t config){
    // subsystems will be initialized in different code
    // but will be here for now
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)){
        SDL_Log("Failed to init SDL! Error:  %s\n", SDL_GetError());
        return false;       
    }
    sdl->window = SDL_CreateWindow("Testing", config.window_width * config.scaler, config.window_height * config.scaler, SDL_WINDOW_OPENGL);
    if(!sdl->window){
        SDL_Log("Failed to create Window! Error: %s\n", SDL_GetError());
        return false;
    }
    sdl->renderer = SDL_CreateRenderer(sdl->window, NULL);
    if(!sdl->renderer){
        SDL_Log("Failed to create Renderer! Error: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

// get default configs or override with args
bool set_config_args(config_t *config, const int argc, char **argv){
    // defaults
    *config = (config_t){
        .window_height = 32,        // chip8 default height
        .window_width = 64,         // chip8 default width 
        .fcolor = 0xA1A1A1FF,       // dark grey
        .bcolor = 0xFFFFFFFF,       // white
        .scaler = 15,               // scale window size, ideally get display size
    };
    //override defaults
    for(int i = 1; i < argc; i++){
        (void)argv[i];
    }
    return true;
}

// init chip8
bool init_c8(chip8_t *c8, char rom_name[]){
    // Defaults
    c8->state = LOADING;            // start emulation and load
    c8->rom_name = rom_name;        // set c8 rom to the passed rom     
    const uint32_t entry = 0x200;   // beginning of c8 memory (can be 0x000)
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0   
        0x20, 0x60, 0x20, 0x20, 0x70,   // 1  
        0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2 
        0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,   // 4    
        0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
        0xF0, 0x80, 0xF0, 0x80, 0x80,   // F
    };
    memcpy(&c8->ram[0], font, sizeof(font));        // copy font to mem
    
    // load rom
    FILE *rom = fopen(rom_name, "rb");
    if(!rom){
        SDL_Log("Failed to open file %s. Please check the path.\n", rom_name);
        return false;
    }
    
    fseek(rom, 0, SEEK_END);
    const size_t rom_s = ftell(rom);
    const size_t max_s = sizeof c8->ram - entry;
    rewind(rom);
    
    if(rom_s > max_s) {
        SDL_Log("Error! ROM is larger than available memory! Max: %zu, ROM: %zu\n", max_s, rom_s);
        return false;
    }
    
    if(fread(&c8->ram[entry], rom_s, 1, rom) != 1){
        SDL_Log("Could not read %s rom into memory.\n", rom_name);
        return false;
    }

    fclose(rom);
    c8->state = RUNNING;            // change state and start game
    c8->PC = entry;                 // start program counter entry
    c8->rom_name = rom_name;        // set chip8 rom
    c8->SP = &c8->stack[0];         // set stack ptr to top of stack
    return true;                    // successful start-up
}

#ifdef DEBUG
void print_debug_info(chip8_t *c8){
    printf("Current Addr: 0x%04X, OpCode: 0x%04X, Desc: ", c8->PC-2, c8->instruction.opcode );
    switch((c8->instruction.opcode >> 12) & 0x0F){
        case 0x00:
            if(c8->instruction.NN == 0xE0){
                // 0x00E0 clear screen
                printf("Clear Screen!\n");
            } else if(c8->instruction.NN == 0xEE){
                // 0x00EE return from subroutine
                printf("Return from Subroutine to Addr: 0x%04X\n", *(c8->SP-1));
            } else { 
                printf("NOOP!\n");
            } // do nothing, not implemented
            break;
        case 0x01:
            // 1NNN
            printf("Jump to Addr: NNN (0x%04X)\n", c8->instruction.NNN);
            break;
        case 0x02:
            // 2NNN
            printf("Call subroutine @ NNN (0x%04X)\n", c8->instruction.NNN);
            break;
        case 0x03:
            // 3XNN Conditional: skip if Vx == NN
            printf("Jump if equal V%X: 0x%02X , NN: 0x%02X\n", c8->instruction.X, c8->V[c8->instruction.X], c8->instruction.NN);
            break;
        case 0x04:
            // 4XNN Conditional: skip if VX != NN
            printf("Jump if not equal V%X: 0x%02X , NN: 0x%02X\n", c8->instruction.X, c8->V[c8->instruction.X], c8->instruction.NN);
            break;
        case 0x05:
            // 5XNN Conditional: skip if VX == VY
            printf("Jump if not equal V%X: 0x%02X , VY: 0x%02X\n", c8->instruction.X, c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
            break;
        case 0x06:
            // 6XNN set  VX = NN
            printf(" V%X equals NN: 0x%02X\n", c8->instruction.X, c8->instruction.NN);
            break;
        case 0x07:
            // 7XNN set VX += NN
            printf("Add V%X: 0x%02X + NN: 0x%02X\n", c8->instruction.X, c8->V[c8->instruction.X], c8->instruction.NN);
            break;
        case 0x08:
            switch(c8->instruction.N){
                case 0:
                    // set VX = VY
                    printf("V%X = V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 1:
                    printf("V%X |= V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 2:
                    printf("V%X &= V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 3:
                    printf("V%X ^= V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 4:
                    printf("V%X += V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 5:
                    printf("V%X -= V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 6:
                    printf("V%X >>= V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y]);
                    break;
                case 7:
                    printf("V%X = V%X - V%X\n", c8->V[c8->instruction.X], c8->V[c8->instruction.Y], c8->V[c8->instruction.X]);
                    break;
                case 0xE:
                    printf("V%X <<= 1\n", c8->V[c8->instruction.X]);
                    break;
            }
            break;
        case 0x09:
            // 4XNN Conditional: skip if VX != NN
            printf("Jump if not equal V%X: 0x%02X , NN: 0x%02X\n", c8->instruction.X, c8->V[c8->instruction.X], c8->instruction.NN);
            break;
        case 0x0A:
            // 0xANNN: set I to NNN
            printf("Set I register to NNN (0x%04X)\n", c8->instruction.NNN);
            break;
        case 0x0B:
            // Jump to V0 + NNN
            printf("Set PC(0x%04X) equal to V0 (0x%04X) plus NNN (0x%04X)\n", c8->PC, c8->V[c8->instruction.X], c8->instruction.NNN);
            break;
        case 0x0C:
            // set VX = rand() % 256 & NN
            printf("Set VX equal to random number from 0-255 & NN.\n");
            break;
        case 0x0D:
            // DXYN - draw
            printf("Draw pixel at [V%X, V%X] (0x%02X, 0x%02X) height of %X\n", c8->instruction.X, c8->instruction.Y, c8->V[c8->instruction.X], c8->V[c8->instruction.Y], c8->instruction.N);
            break;
        case 0x0E:
            switch(c8->instruction.NN){
                case 0x9E:
                    printf("Skip V%X if equal to Get_Key.\n", c8->V[c8->instruction.X]);
                    break;
                case 0xA1:
                    break;
            }
            break;
        case 0x0F:
            switch(c8->instruction.NN){
                case 0x07:
                    printf("Set V%01X to delay_timer (%02X)!\n",  c8->instruction.X, c8->delay_timer);
                    break;
                case 0x0A:
                    printf("Awaiting key press!\n");
                    break;
                case 0x15:
                    printf("Set delay timer!\n");
                    break;
                case 0x18:
                    printf("Set sound timer!\n");
                    break;
                case 0x1E:
                    printf("Add VX to I. Does not change VF!\n");
                    break;
                case 0x29:
                    printf("Sets I to the location of the sprite!\n");
                    break;
                case 0x33:
                    printf("Store binary coded decimal. I, I+1, I+2!\n");
                    break;
                case 0x55:
                    printf("Store V0 to VX in memory, starting at I (%04X)!\n", c8->I);
                    break;
                case 0x65:
                    printf("Restore V0 to VX from memory.\n");
                    break;
            }
            break;
        default:
            printf("Unimplemented OpCode!\n");
            break;
    }


}
#endif

void emulator(chip8_t *c8, const config_t config){
    c8->instruction.opcode = (c8->ram[c8->PC] << 8 | c8->ram[c8->PC+1]);//get opcode
    c8->PC += 2;                        // increment PC
    // fill chip8 opcode
    c8->instruction.NNN = c8->instruction.opcode & 0x0FFF;
    c8->instruction.NN = c8->instruction.opcode & 0x0FF;
    c8->instruction.N = c8->instruction.opcode & 0x0F;
    c8->instruction.X = (c8->instruction.opcode >> 8) & 0x0F;
    c8->instruction.Y = (c8->instruction.opcode >> 4) & 0x0F;

    #ifdef DEBUG
        print_debug_info(c8);
    #endif

    switch((c8->instruction.opcode >> 12) & 0x0F){
        case 0x00:
            if(c8->instruction.NN == 0xE0){
                // clear screen
                memset(&c8->display[0], false, sizeof c8->display);
            } else if(c8->instruction.NN == 0xEE){
                // return from subroutine
                c8->PC = *--c8->SP;
            } else { } // do nothing, not implemented
            break;
        case 0x01:
            c8->PC = c8->instruction.NNN;
            break;
        case 0x02:
            // call subroutine
            *c8->SP++ = c8->PC;
            c8->PC = c8->instruction.NNN;
            break;
        case 0x03:
            // if VX == NN, skip next instruction
            if(c8->V[c8->instruction.X] == c8->instruction.NN){
                c8->PC += 2;
            }           
            break;
        case 0x04:
            // if VX != NN, skip next instruction
            if(c8->V[c8->instruction.X] != c8->instruction.NN){
                c8->PC += 2;
            }           
            break;
        case 0x05:
            // if VX == VY, skip next instruction
            if(c8->V[c8->instruction.X] == c8->V[c8->instruction.Y]){
                c8->PC += 2;
            }           
            break;
        case 0x06:
            // set VX = NN
            c8->V[c8->instruction.X] = c8->instruction.NN;
            break;
        case 0x07: 
            // Add NN to VX
            c8->V[c8->instruction.X] += c8->instruction.NN;
            break;
        case 0x08: // bit operations
            switch(c8->instruction.N){
                case 0:
                    // set VX = VY
                    c8->V[c8->instruction.X] =  c8->V[c8->instruction.Y];
                    break;
                case 1:
                    // VX |= VY
                    c8->V[c8->instruction.X] |= c8->V[c8->instruction.Y];
                    break;
                case 2:
                    // VX &= VY
                   c8->V[c8->instruction.X] &= c8->V[c8->instruction.Y];
                    break;
                case 3:
                    // VX ^= VY
                    c8->V[c8->instruction.X] ^= c8->V[c8->instruction.Y];
                    break;
                case 4:
                    // VX += VY
                    c8->V[c8->instruction.X] += c8->V[c8->instruction.Y];
                    break;
                case 5:
                    // VX -= VY
                    c8->V[c8->instruction.X] -= c8->V[c8->instruction.Y];
                    break;
                case 6:
                    // VX >>= VY
                    c8->V[c8->instruction.X] >>= c8->V[c8->instruction.Y];
                    break;
                case 7:
                    // VX = VY - VX
                    c8->V[c8->instruction.X] =  c8->V[c8->instruction.Y] - c8->V[c8->instruction.X];
                    break;
                case 0xE:
                    // VX <<= 1
                    c8->V[c8->instruction.X] <<= 1;
                    break;
            }
            break;
        case 0x09:
            // if VX != VY, skip next instruction
            if(c8->V[c8->instruction.X] != c8->V[c8->instruction.Y]){
                c8->PC += 2;
            }           
            break;
        case 0x0A:
            // 0xANNN: set I to NNN
            c8->I = c8->instruction.NNN;
            break;
        case 0x0B:
            // set PC = VX + NNN
            c8->PC = (c8->V[c8->instruction.X] + c8->instruction.NNN);
            break;
        case 0x0C:
            // set VX = rand() & NN
            c8->V[c8->instruction.X] = (rand() % 256) & c8->instruction.NN;
            break;
        case 0x0D:
            // DXYN: draw at [VX,VY] with a height of N
            // const original location
            const uint8_t oX_coord = c8->V[c8->instruction.X] % config.window_width;
            // mutable locations 
            uint8_t X_coord = c8->V[c8->instruction.X] % config.window_width;
            uint8_t Y_coord = c8->V[c8->instruction.Y] % config.window_height;
            
            c8->V[0xF] = 0; // init carry flag to 0
            // loop N rows of sprite
            for(uint8_t i = 0; i < c8->instruction.N; i++){
                uint8_t sprite_d = c8->ram[c8->I + i];
                X_coord = oX_coord;
                for(int j = 7; j >= 0; j--){
                    if((sprite_d & (1 << j)) && c8->display[Y_coord * config.window_width + X_coord]){
                        c8->V[0x0F] = 1;
                    }
                    c8->display[Y_coord * config.window_width + X_coord] ^= (sprite_d & (1 << j));

                    // stop if right of screen
                    if(++X_coord >= config.window_width) break;
                }
                // stop if bottom of screen
                if(++Y_coord >= config.window_height) break;
            }
            break;
        case 0x0E:
            // if VX = Key, skip next instruction
            // else VX != Key, skip this instruction
            break;
        case 0x0F:
            switch(c8->instruction.NN){
                case 0x07:
                    // set VX to  delay_timer value
                    c8->V[c8->instruction.X] = c8->delay_timer;
                    break;
                case 0x0A:
                    // set VX = key pressed
                    break;
                case 0x15:
                    // set delay timer
                    c8->delay_timer = c8->V[c8->instruction.X];
                    break;
                case 0x18:
                    // set sound timer
                    c8->sound_timer = c8->V[c8->instruction.X];
                    break;
                case 0x1E:
                    //set I to VX
                    c8->I = c8->V[c8->instruction.X];
                    // amiga sets carry flag, 1 known game relies on it so...
                    if(c8->I > 0x0FFF) c8->V[0x0F] = 1;
                    break;
                case 0x29:
                    // set I to location of sprite
                    break;
                case 0x33:
                    // Binary Coded Decimal of VX:
                    // 100's in I
                    // 10's in I+1
                    // 1's in I+2
                    break;
                case 0x55:
                    // dump register values into memory
                    break;
                case 0x65:
                    // restore registers from memory
                    break;
            }
            break;
        default:
            printf("Error! OpCode not exist/implemented! 0x%04X\n", c8->instruction.opcode);
            break;
    }
}

void prep_screen(const config_t config, const sdl_t sdl){
    const uint8_t r = (config.bcolor >> 24) & 0xFF;
    const uint8_t g = (config.bcolor >> 16) & 0xFF;
    const uint8_t b = (config.bcolor >> 8) & 0xFF;
    const uint8_t a = (config.bcolor >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r,g,b,a);
    SDL_RenderPresent(sdl.renderer);
}

void update_screen(const sdl_t sdl, const config_t config, chip8_t *c8){
    SDL_FRect rect  = {.x = 0, .y = 0, .w = config.scaler, .h = config.scaler};
    
    const uint8_t fg_r = (config.fcolor >> 24) & 0xFF;
    const uint8_t fg_g = (config.fcolor >> 16) & 0xFF;
    const uint8_t fg_b = (config.fcolor >> 8) & 0xFF;
    const uint8_t fg_a = (config.fcolor >> 0) & 0xFF;
    
    const uint8_t bg_r = (config.bcolor >> 24) & 0xFF;
    const uint8_t bg_g = (config.bcolor >> 16) & 0xFF;
    const uint8_t bg_b = (config.bcolor >> 8) & 0xFF;
    const uint8_t bg_a = (config.bcolor >> 0) & 0xFF;

    for(uint32_t i = 0; i < sizeof c8->display; i++){
        rect.x = (i % config.window_width) * config.scaler;
        rect.y = (i / config.window_width) * config.scaler;

        if(c8->display[i]){
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        } else {
            SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
    }
    SDL_RenderPresent(sdl.renderer);
}

// all input 
void input_handler(chip8_t *c8){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_EVENT_QUIT:
                c8->state = QUIT;
                return;
            case SDL_EVENT_KEY_DOWN:
                switch(event.key.key){
                    case SDLK_ESCAPE:
                        c8->state = QUIT;
                        return;
                    case SDLK_SEMICOLON:
                        if(c8->state == RUNNING){
                            c8->state = PAUSED;
                            SDL_Log("===PAUSED===");
                        } else {
                            SDL_Log("===RESUME===");
                            c8->state = RUNNING;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                break;
            default:
                break;
        }
    }
}

// shut down emulation
void cleanup(const sdl_t sdl){
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
}


int main(int argc, char **argv){
     if(argc < 2){
        fprintf(stderr, "Usage: %s [rom]\n", argv[0]);
        exit(EXIT_FAILURE);
    } 
    // inits
    config_t config = {0};
    if(!set_config_args(&config, argc, argv)) exit(EXIT_FAILURE);
    sdl_t sdl = {0};
    if(!init_sys(&sdl, config)) exit(EXIT_FAILURE);
    chip8_t c8 = {0};
    if(!init_c8(&c8, argv[1])) exit(EXIT_FAILURE);
    // if all above passes
    prep_screen(config, sdl);
    // loop
    // make sure chip8 is done loading AND not shutting down
    while(c8.state != QUIT && c8.state != LOADING){
        input_handler(&c8);             // input
        if(c8.state == PAUSED) continue;// is PAUSED, goto top
        emulator(&c8, config);          // emulation
        SDL_Delay(16);                  // framerate (60Hz)
        update_screen(sdl, config, &c8);// display window
    }

    // close
    cleanup(sdl);
    return 0;
}
