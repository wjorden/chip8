CFLAGS := -std=c17 -Wall -Wextra -Werror -ggdb

all: 
	gcc main.c -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs`

debug: 
	gcc main.c -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs` -DDEBUG

