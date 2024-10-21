#ifndef INIT_H
#define INIT_H

#include "typedefs.h"

bool init_sdl(sdl_t *sdl, config_t config);
bool set_config_args(config_t *config, const int argc, char **argv);
bool init_c8(chip8_t *c8, char rom_name[]);
#endif
