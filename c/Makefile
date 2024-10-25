CC 		:= clang
CFLAGS 	:= -std=c2x -Wall -Wextra -Werror -ggdb
CFLAGS	+= -I./include
SRCS	:= $(wildcard *.c) $(wildcard src/*.c)

all: 
	$(CC) $(SRCS) -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs`

debug: 
	$(CC) $(SRCS) -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs` -DDEBUG

