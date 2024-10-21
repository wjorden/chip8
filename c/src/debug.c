#ifdef DEBUG
#include <stdio.h>
#include "../include/debug.h"

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
