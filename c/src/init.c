#include "../include/init.h"
#include <stdio.h>

// init all required systems
bool init_sdl(sdl_t *sdl, config_t config) {
  // subsystems will be initialized in different code
  // but will be here for now
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_Log("Failed to init SDL! Error:  %s\n", SDL_GetError());
    return false;
  }
  sdl->window =
      SDL_CreateWindow("Testing", config.window_width * config.scaler,
                       config.window_height * config.scaler, SDL_WINDOW_OPENGL);
  if (!sdl->window) {
    SDL_Log("Failed to create Window! Error: %s\n", SDL_GetError());
    return false;
  }
  sdl->renderer = SDL_CreateRenderer(sdl->window, NULL);
  if (!sdl->renderer) {
    SDL_Log("Failed to create Renderer! Error: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

// get default configs or override with args
bool set_config_args(config_t *config, const int argc, char **argv) {
  // defaults
  *config = (config_t){
      .window_height = 32,  // chip8 default height
      .window_width = 64,   // chip8 default width
      .fcolor = 0xA1A1A1FF, // dark grey
      .bcolor = 0xFFFFFFFF, // white
      .scaler = 15,         // scale window size, ideally get display size
  };
  // TODO: override defaults by arguments
  for (int i = 1; i < argc; i++) {
    (void)argv[i];
  }
  return true;
}

// init chip8
bool init_c8(chip8_t *c8, char rom_name[]) {
  // Defaults
  c8->state = LOADING; // start emulation and load
  SDL_Log("Loading data to RAM.");
  c8->rom_name = rom_name;      // set c8 rom to the passed rom
  const uint32_t entry = 0x200; // beginning of c8 memory (can be 0x000)
  const uint8_t font[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };
  memcpy(&c8->ram[0], font, sizeof(font)); // copy font to mem

  // load rom
  FILE *rom = fopen(rom_name, "rb");
  if (!rom) {
    SDL_Log("Failed to open file %s. Please check the path.\n", rom_name);
    return false;
  }

  fseek(rom, 0, SEEK_END);
  const size_t rom_s = ftell(rom);
  const size_t max_s = sizeof c8->ram - entry;
  rewind(rom);

  if (rom_s > max_s) {
    SDL_Log("Error! ROM is larger than available memory! Max: %zu, ROM: %zu\n",
            max_s, rom_s);
    return false;
  }

  if (fread(&c8->ram[entry], rom_s, 1, rom) != 1) {
    SDL_Log("Could not read %s rom into memory.\n", rom_name);
    return false;
  }

  fclose(rom);
  c8->state = RUNNING; // change state and start game
  SDL_Log("Emulator is now running!");
  c8->PC = entry;          // start program counter entry
  c8->rom_name = rom_name; // set chip8 rom
  c8->SP = &c8->stack[0];  // set stack ptr to top of stack
  return true;             // successful start-up
}
