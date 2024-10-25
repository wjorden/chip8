#ifndef DISPLAY_H
#define DISPLAY_H

#include "typedefs.h"

void prep_screen(config_t config, sdl_t sdl);
void update_screen(sdl_t sdl, config_t config, chip8_t *c8);
#endif
