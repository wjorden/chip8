// system
#include <stdio.h>
#include <stdlib.h>
// user
#include "include/display.h"
#include "include/emulator.h"
#include "include/init.h"
#include "include/input.h"

#ifdef DEBUG
#include "include/debug.h"
#endif

// shut down emulation
void cleanup(sdl_t *sdl) {
  SDL_DestroyRenderer(sdl->renderer);
  SDL_DestroyWindow(sdl->window);
  SDL_Quit();
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s [rom]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // inits
  config_t config = {0};
  if (!set_config_args(&config, argc, argv))
    exit(EXIT_FAILURE);
  sdl_t sdl = {0};
  if (!init_sdl(&sdl, config))
    exit(EXIT_FAILURE);
  chip8_t c8 = {0};
  if (!init_c8(&c8, argv[1]))
    exit(EXIT_FAILURE);
  // if all above passes
  prep_screen(config, sdl);
  // loop
  // make sure chip8 is done loading AND not shutting down
  while (c8.state != QUIT && c8.state != LOADING) {
    input_handler(&c8); // input
    if (c8.state == PAUSED)
      continue;                      // is PAUSED, goto top
    emulator(&c8, config);           // emulation
    SDL_Delay(16);                   // framerate (60Hz)
    update_screen(sdl, config, &c8); // display window
  }

  // close
  cleanup(&sdl);
  return 0;
}
