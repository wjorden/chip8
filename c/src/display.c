#include "../include/display.h"
#include <stdint.h>

void prep_screen(const config_t config, const sdl_t sdl) {
  const uint8_t r = (config.bcolor >> 24) & 0xFF;
  const uint8_t g = (config.bcolor >> 16) & 0xFF;
  const uint8_t b = (config.bcolor >> 8) & 0xFF;
  const uint8_t a = (config.bcolor >> 0) & 0xFF;

  SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
  SDL_RenderPresent(sdl.renderer);
}

void update_screen(const sdl_t sdl, const config_t config, chip8_t *c8) {
  SDL_FRect rect = {.x = 0, .y = 0, .w = config.scaler, .h = config.scaler};

  const uint8_t fg_r = (config.fcolor >> 24) & 0xFF;
  const uint8_t fg_g = (config.fcolor >> 16) & 0xFF;
  const uint8_t fg_b = (config.fcolor >> 8) & 0xFF;
  const uint8_t fg_a = (config.fcolor >> 0) & 0xFF;

  const uint8_t bg_r = (config.bcolor >> 24) & 0xFF;
  const uint8_t bg_g = (config.bcolor >> 16) & 0xFF;
  const uint8_t bg_b = (config.bcolor >> 8) & 0xFF;
  const uint8_t bg_a = (config.bcolor >> 0) & 0xFF;

  for (uint32_t i = 0; i < sizeof c8->display; i++) {
    rect.x = (i % config.window_width) * config.scaler;
    rect.y = (i / config.window_width) * config.scaler;

    if (c8->display[i]) {
      SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
      SDL_RenderFillRect(sdl.renderer, &rect);
    } else {
      SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
      SDL_RenderFillRect(sdl.renderer, &rect);
    }
  }
  SDL_RenderPresent(sdl.renderer);
}
