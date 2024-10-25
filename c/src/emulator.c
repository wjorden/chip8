#include "../include/emulator.h"
#include "../include/debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void emulator(chip8_t *c8, const config_t config) {
  bool carry;
  c8->instruction.opcode =
      (c8->ram[c8->PC] << 8 | c8->ram[c8->PC + 1]); // get opcode
  c8->PC += 2;                                      // increment PC
  // fill chip8 opcode
  c8->instruction.NNN = c8->instruction.opcode & 0x0FFF;
  c8->instruction.NN = c8->instruction.opcode & 0x0FF;
  c8->instruction.N = c8->instruction.opcode & 0x0F;
  c8->instruction.X = (c8->instruction.opcode >> 8) & 0x0F;
  c8->instruction.Y = (c8->instruction.opcode >> 4) & 0x0F;

#ifdef DEBUG
  print_debug_info(c8);
#endif

  switch ((c8->instruction.opcode >> 12) & 0x0F) {
  case 0x00:
    if (c8->instruction.NN == 0xE0) {
      // clear screen
      memset(&c8->display[0], false, sizeof c8->display);
    } else if (c8->instruction.NN == 0xEE) {
      // return from subroutine
      c8->PC = *--c8->SP;
    } else {
    } // do nothing, not implemented
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
    if (c8->V[c8->instruction.X] == c8->instruction.NN) {
      c8->PC += 2;
    }
    break;
  case 0x04:
    // if VX != NN, skip next instruction
    if (c8->V[c8->instruction.X] != c8->instruction.NN) {
      c8->PC += 2;
    }
    break;
  case 0x05:
    // if VX == VY, skip next instruction
    if (c8->V[c8->instruction.X] == c8->V[c8->instruction.Y]) {
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
    switch (c8->instruction.N) {
    case 0:
      // set VX = VY
      c8->V[c8->instruction.X] = c8->V[c8->instruction.Y];
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
      carry = ((uint16_t)(c8->V[c8->instruction.X] + c8->V[c8->instruction.Y]) >
               255);
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
      c8->V[c8->instruction.X] =
          c8->V[c8->instruction.Y] - c8->V[c8->instruction.X];
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
    if (c8->V[c8->instruction.X] != c8->V[c8->instruction.Y]) {
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
    // draw at [VX,VY] with a height of N
    // original location
    const uint8_t oX_coord = c8->V[c8->instruction.X] % config.window_width;
    // mutable locations
    uint8_t X_coord = c8->V[c8->instruction.X] % config.window_width;
    uint8_t Y_coord = c8->V[c8->instruction.Y] % config.window_height;

    c8->V[0xF] = 0; // init carry flag to 0
    // loop N rows (X) of sprite
    for (uint8_t i = 0; i < c8->instruction.N; i++) {
      // sprite data = I + loop [i]
      uint8_t sprite_d = c8->ram[c8->I + i];
      // return to og X position
      X_coord = oX_coord;
      // loop N columns (Y) of sprite
      for (int j = 7; j >= 0; j--) {
        // left shit 1 by loop and make sure it is still in the window
        if ((sprite_d & (1 << j)) &&
            c8->display[Y_coord * config.window_width + X_coord]) {
          // set carry flag
          c8->V[0x0F] = 1;
        }
        c8->display[Y_coord * config.window_width + X_coord] ^=
            (sprite_d & (1 << j));
        // stop if past right of screen
        if (++X_coord >= config.window_width)
          break;
      }
      // stop if bottom of screen
      if (++Y_coord >= config.window_height)
        break;
    }
    break;
  case 0x0E:
    // if VX = Key, skip next instruction
    // else VX != Key, skip this instruction
    if (c8->instruction.NN == 0x9E) {
      if (c8->keys[c8->V[c8->instruction.X]])
        c8->PC += 2;
    } else if (c8->instruction.NN == 0xA1) {
      if (!c8->keys[c8->V[c8->instruction.X]])
        c8->PC += 2;
    }
    break;
  case 0x0F:
    switch (c8->instruction.NN) {
    case 0x07:
      // set VX to  delay_timer value
      c8->V[c8->instruction.X] = c8->delay_timer;
      break;
    case 0x0A:
      // set VX = key pressed
      static bool key_pressed = false;
      static uint8_t key = 0xFF;

      for (uint8_t i = 0; key == 0xFF && i < sizeof c8->keys; i++) {
        if (c8->keys[i]) {
          key = i;
          key_pressed = true;
          break;
        }
        if (!key_pressed)
          c8->PC -= 2;
        else {
          if (c8->keys[key])
            // busy loop, wait for key up
            c8->PC -= 2;
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
      if (c8->I > 0x0FFF)
        c8->V[0x0F] = 1;
      break;
    case 0x29:
      // set I to location of sprite
      c8->I = c8->V[c8->instruction.X] * 5;
      break;
    case 0x33:
      // Binary Coded Decimal of VX:
      uint8_t bcd = c8->V[c8->instruction.X];
      // 1's in I+2
      c8->ram[c8->I + 2] = bcd % 10;
      bcd /= 10;
      // 10's in I+1
      c8->ram[c8->I + 1] = bcd % 10;
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
    printf("Error! OpCode not implemented! 0x%04X\n", c8->instruction.opcode);
    break;
  }

  // handle timers
  if (c8->delay_timer != 0) {
    c8->delay_timer--;
  } else {
    c8->delay_timer = 0;
  }
  if (c8->sound_timer != 0) {
    c8->sound_timer--;
  } else {
    c8->sound_timer = 0;
  }
}
