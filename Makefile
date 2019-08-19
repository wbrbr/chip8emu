CFLAGS = -Wall -Wextra -Iinclude/ -g -fsanitize=address,undefined
LDFLAGS = -lSDL2 -g -fsanitize=address,undefined

all: main.o chip8.o
	g++ $^ -o chip8emu $(LDFLAGS)

%.o: src/%.cpp
	g++ -c $< -o $@ $(CFLAGS)
