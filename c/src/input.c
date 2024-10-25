#include "../include/input.h"

// all input
void input_handler(chip8_t *c8) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      c8->state = QUIT;
      return;
    case SDL_EVENT_KEY_DOWN:
      switch (event.key.key) {
      case SDLK_ESCAPE:
        c8->state = QUIT;
        return;
      case SDLK_SEMICOLON:
        if (c8->state == RUNNING) {
          c8->state = PAUSED;
          SDL_Log("===PAUSED===");
        } else {
          SDL_Log("===RESUME===");
          c8->state = RUNNING;
        }
        break;
      // Map chip8 keys
      case SDLK_1:
        c8->keys[0x1] = true;
        break;
      case SDLK_2:
        c8->keys[0x2] = true;
        break;
      case SDLK_3:
        c8->keys[0x3] = true;
        break;
      case SDLK_4:
        c8->keys[0xC] = true;
        break;
      case SDLK_Q:
        c8->keys[0x4] = true;
        break;
      case SDLK_W:
        c8->keys[0x5] = true;
        break;
      case SDLK_E:
        c8->keys[0x6] = true;
        break;
      case SDLK_R:
        c8->keys[0xD] = true;
        break;
      case SDLK_A:
        c8->keys[0x7] = true;
        break;
      case SDLK_S:
        c8->keys[0x8] = true;
        break;
      case SDLK_D:
        c8->keys[0x9] = true;
        break;
      case SDLK_F:
        c8->keys[0xE] = true;
        break;
      case SDLK_Z:
        c8->keys[0xA] = true;
        break;
      case SDLK_X:
        c8->keys[0x0] = true;
        break;
      case SDLK_C:
        c8->keys[0xB] = true;
        break;
      case SDLK_V:
        c8->keys[0xF] = true;
        break;
      default:
        break;
      }
      break;
    case SDL_EVENT_KEY_UP:
      switch (event.key.key) {
      // Map chip8 keys
      case SDLK_1:
        c8->keys[0x1] = false;
        break;
      case SDLK_2:
        c8->keys[0x2] = false;
        break;
      case SDLK_3:
        c8->keys[0x3] = false;
        break;
      case SDLK_4:
        c8->keys[0xC] = false;
        break;
      case SDLK_Q:
        c8->keys[0x4] = false;
        break;
      case SDLK_W:
        c8->keys[0x5] = false;
        break;
      case SDLK_E:
        c8->keys[0x6] = false;
        break;
      case SDLK_R:
        c8->keys[0xD] = false;
        break;
      case SDLK_A:
        c8->keys[0x7] = false;
        break;
      case SDLK_S:
        c8->keys[0x8] = false;
        break;
      case SDLK_D:
        c8->keys[0x9] = false;
        break;
      case SDLK_F:
        c8->keys[0xE] = false;
        break;
      case SDLK_Z:
        c8->keys[0xA] = false;
        break;
      case SDLK_X:
        c8->keys[0x0] = false;
        break;
      case SDLK_C:
        c8->keys[0xB] = false;
        break;
      case SDLK_V:
        c8->keys[0xF] = false;
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
