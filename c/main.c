// system
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
// user
#include "include/init.h"
#ifdef DEBUG
#include "include/debug.h"
#endif

void emulator(chip8_t *c8, const config_t config){
    bool carry;
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
            // goto address
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
                    c8->V[0xF] = 0;
                    break;
                case 2:
                    // VX &= VY
                    c8->V[c8->instruction.X] &= c8->V[c8->instruction.Y];
                    c8->V[0xF] = 0;
                    break;
                case 3:
                    // VX ^= VY
                    c8->V[c8->instruction.X] ^= c8->V[c8->instruction.Y];
                    c8->V[0xF] = 0;
                    break;
                case 4:
                    carry = ((uint16_t)(c8->V[c8->instruction.X] + c8->V[c8->instruction.Y]) > 255);
                    // VX += VY
                    c8->V[c8->instruction.X] += c8->V[c8->instruction.Y];
                    c8->V[0xF] = carry;
                    break;
                case 5:
                    carry = (c8->V[c8->instruction.Y] <= c8->V[c8->instruction.X]);
                    // VX -= VY
                    c8->V[c8->instruction.X] -= c8->V[c8->instruction.Y];
                    c8->V[0xF] = carry;
                    break;
                case 6:
                    carry = c8->V[c8->instruction.Y] & 1;
                    // VX >>= VY
                    c8->V[c8->instruction.X] = c8->V[c8->instruction.Y] >> 1;
                    c8->V[0xF] = carry;
                    break;
                case 7:
                    carry = (c8->V[c8->instruction.X] <= c8->V[c8->instruction.Y]);
                    // VX = VY - VX
                    c8->V[c8->instruction.X] =  c8->V[c8->instruction.Y] - c8->V[c8->instruction.X];
                    c8->V[0xF] = carry;
                    break;
                case 0xE:
                    carry = (c8->V[c8->instruction.Y] & 0x80) >> 7;
                    // VX <<= 1
                    c8->V[c8->instruction.X] = c8->V[c8->instruction.Y] <<= 1;
                    c8->V[0xF] = carry;
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
            // set I to NNN
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
            //  draw at [VX,VY] with a height of N
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
            if(c8->instruction.NN == 0x9E){
                if(c8->keys[c8->V[c8->instruction.X]])
                    c8->PC += 2;
            } else if( c8->instruction.NN == 0xA1){
                if(!c8->keys[c8->V[c8->instruction.X]])
                    c8->PC += 2;
            }
            break;
        case 0x0F:
            switch(c8->instruction.NN){
                case 0x07:
                    // set VX to  delay_timer value
                    c8->V[c8->instruction.X] = c8->delay_timer;
                    break;
                case 0x0A:
                    // set VX = key pressed
                    static bool key_pressed = false;
                    static uint8_t key = 0xFF;

                    for(uint8_t i = 0; key == 0xFF && i < sizeof c8->keys; i++){
                        if(c8->keys[i]){
                            key = i;
                            key_pressed = true;
                            break;
                        }
                        if(!key_pressed) c8->PC -= 2;
                        else {
                            if (c8->keys[key])
                                // busy loop, wait for key up
                                c8->PC -=2;
                            else {
                                c8->V[c8->instruction.X] = key;
                                key = 0xFF;
                                key_pressed = false;
                            }
                        }
                    }
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
                    // set I to VX
                    c8->I = c8->V[c8->instruction.X];
                    // amiga sets carry flag, 1 known game relies on it so...
                    if(c8->I > 0x0FFF) c8->V[0x0F] = 1;
                    break;
                case 0x29:
                    // set I to location of sprite
                    c8->I = c8->V[c8->instruction.X] * 5;
                    break;
                case 0x33:
                    // Binary Coded Decimal of VX:
                    uint8_t bcd = c8->V[c8->instruction.X];
                    // 1's in I+2
                    c8->ram[c8->I+2] = bcd % 10;
                    bcd /= 10;
                    // 10's in I+1
                    c8->ram[c8->I+1] = bcd % 10;
                    bcd /= 10;
                    // 100's in I
                    c8->ram[c8->I] = bcd;
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

    // handle timers
    if(c8->delay_timer != 0){
        c8->delay_timer--;
    } else { c8->delay_timer = 0;}
    if(c8->sound_timer != 0){
        c8->sound_timer--;
    } else { c8->sound_timer = 0;}
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
                    // Map chip8 keys
                    case SDLK_1: c8->keys[0x1] = true; 
                        break;    
                    case SDLK_2: c8->keys[0x2] = true; 
                        break;    
                    case SDLK_3: c8->keys[0x3] = true; 
                        break;    
                    case SDLK_4: c8->keys[0xC] = true; 
                        break;    
                    case SDLK_Q: c8->keys[0x4] = true; 
                        break;    
                    case SDLK_W: c8->keys[0x5] = true; 
                        break;    
                    case SDLK_E: c8->keys[0x6] = true; 
                        break;    
                    case SDLK_R: c8->keys[0xD] = true; 
                        break;    
                    case SDLK_A: c8->keys[0x7] = true; 
                        break;    
                    case SDLK_S: c8->keys[0x8] = true; 
                        break;    
                    case SDLK_D: c8->keys[0x9] = true; 
                        break;    
                    case SDLK_F: c8->keys[0xE] = true; 
                        break;    
                    case SDLK_Z: c8->keys[0xA] = true; 
                        break;    
                    case SDLK_X: c8->keys[0x0] = true; 
                        break;    
                    case SDLK_C: c8->keys[0xB] = true; 
                        break;    
                    case SDLK_V: c8->keys[0xF] = true; 
                        break;    
                    default:
                        break;
                }
                break;
            case SDL_EVENT_KEY_UP:
                switch(event.key.key){
                    // Map chip8 keys
                    case SDLK_1: c8->keys[0x1] = false; 
                        break;    
                    case SDLK_2: c8->keys[0x2] = false; 
                        break;    
                    case SDLK_3: c8->keys[0x3] = false; 
                        break;    
                    case SDLK_4: c8->keys[0xC] = false; 
                        break;    
                    case SDLK_Q: c8->keys[0x4] = false; 
                        break;    
                    case SDLK_W: c8->keys[0x5] = false; 
                        break;    
                    case SDLK_E: c8->keys[0x6] = false; 
                        break;    
                    case SDLK_R: c8->keys[0xD] = false; 
                        break;    
                    case SDLK_A: c8->keys[0x7] = false; 
                        break;    
                    case SDLK_S: c8->keys[0x8] = false; 
                        break;    
                    case SDLK_D: c8->keys[0x9] = false; 
                        break;    
                    case SDLK_F: c8->keys[0xE] = false; 
                        break;    
                    case SDLK_Z: c8->keys[0xA] = false; 
                        break;    
                    case SDLK_X: c8->keys[0x0] = false; 
                        break;    
                    case SDLK_C: c8->keys[0xB] = false; 
                        break;    
                    case SDLK_V: c8->keys[0xF] = false; 
                        break;
                    default:
                        break;
                }
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
    if(!init_sdl(&sdl, config)) exit(EXIT_FAILURE);
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
