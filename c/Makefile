CFLAGS 	:= -std=c17 -Wall -Wextra -Werror -ggdb
CFLAGS	+= -I./include
SRCS	:= $(wildcard *.c) $(wildcard src/*.c)

all: 
	gcc $(SRCS) -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs`

debug: 
	gcc $(SRCS) -o c8 $(CFLAGS) `pkg-config sdl3 --cflags --libs` -DDEBUG

