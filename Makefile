CFLAGS = -Wall -Wextra -Iinclude/ -Iexternal/include -g
LDFLAGS = -lSDL2 -g -ldl -lGL

all: main.o chip8.o imgui.o imgui_demo.o imgui_draw.o imgui_widgets.o imgui_impl_sdl.o imgui_impl_opengl2.o glad.o
	g++ $^ -o chip8emu $(LDFLAGS)

%.o: src/%.cpp
	g++ -c $< -o $@ $(CFLAGS)

%.o: external/src/%.cpp
	g++ -c $< -o $@ $(CFLAGS)
