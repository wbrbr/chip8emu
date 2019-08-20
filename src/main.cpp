#include "glad/glad.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include "chip8.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"

bool translateKey(int scancode, int* keycode)
{
    switch(scancode) {
        case SDL_SCANCODE_X:
            *keycode = 0;
            break;
        case SDL_SCANCODE_1:
            *keycode = 1;
            break;
        case SDL_SCANCODE_2:
            *keycode = 2;
            break;
        case SDL_SCANCODE_3:
            *keycode = 3;
            break;
        case SDL_SCANCODE_Q:
            *keycode = 4;
            break;
        case SDL_SCANCODE_W:
            *keycode = 5;
            break;
        case SDL_SCANCODE_E:
            *keycode = 6;
            break;

        case SDL_SCANCODE_A:
            *keycode = 7;
            break;

        case SDL_SCANCODE_S:
            *keycode = 8;
            break;

        case SDL_SCANCODE_D:
            *keycode = 9;
            break;

        case SDL_SCANCODE_Z:
            *keycode = 10;
            break;

        case SDL_SCANCODE_C:
            *keycode = 11;
            break;

        case SDL_SCANCODE_4:
            *keycode = 12;
            break;

        case SDL_SCANCODE_R:
            *keycode = 13;
            break;

        case SDL_SCANCODE_F:
            *keycode = 14;
            break;

        case SDL_SCANCODE_V:
            *keycode = 15;
            break;

        default:
            return false;
    }
    return true;
}

void drawDebugWindow(Chip8& chip8)
{
    char buf[10];
    for (int i = 0; i <= 0xf; i++)
    {
        snprintf(buf, 10, "V%x = 0x%x", i, chip8.regs[i]);
        ImGui::Text(buf);
    }

    snprintf(buf, 10, "I  = 0x%x", chip8.ir);
    ImGui::Text(buf);

    ImGui::Separator();
    uint16_t instr = (chip8.memory[chip8.pc] << 8) | chip8.memory[chip8.pc+1];
    snprintf(buf, 10, "%04x", instr);
    ImGui::Text(buf);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <rom file>\n", argv[0]);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init: %s", SDL_GetError());
        return 1;
    }

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("chip8emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 640, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    gladLoadGL();

	IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();	


    uint32_t pixels[64*32];
    memset(pixels, 0, sizeof(pixels));
    glEnable(GL_TEXTURE_2D);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 64, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    SDL_Event e;

    Chip8 chip8;
    chip8.load(std::string(argv[1]));

    uint32_t last_cycle = SDL_GetTicks();
    uint32_t last_frame = SDL_GetTicks();
    bool waiting = false;
    int wait_reg;
    bool step_go = false;
    bool stepmode = true;

    while (true) {

        uint32_t current = SDL_GetTicks();

        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) break;
        if (e.type == SDL_KEYDOWN) {
            switch(e.key.keysym.scancode) {
                case SDL_SCANCODE_SPACE:
                    stepmode = !stepmode;
                    break;

                case SDL_SCANCODE_N:
                    step_go = true;
                    break;

                default:
                    break;
            }
        }
		ImGui_ImplSDL2_ProcessEvent(&e);

        int keycode;
        if (waiting && e.type == SDL_KEYDOWN && translateKey(e.key.keysym.scancode, &keycode)) {
            waiting = false;
            chip8.regs[wait_reg] = keycode;
        }
        
        if (!(stepmode && !step_go) && !waiting && current - last_cycle >= 2) {
            step_go = false;
            last_cycle = current;
            SideEffects eff = chip8.cycle();

            if (eff.clear) {
                memset(pixels, 0, sizeof(pixels));
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            } else if (eff.draw_n > 0) {
                SDL_Rect r;
                r.y = eff.draw_y;
                r.h = eff.draw_n;
                if (eff.draw_x <= 56) {
                    r.x = eff.draw_x;
                    r.w = 8;
                } else {
                    r.x = 0;
                    r.w = 64;
                }

                bool* screen = chip8.screen;

                for (int j = 0; j < r.h; j++)
                {
                    for (int i = 0; i < r.w; i++)
                    {
                        pixels[(r.y+j)*64+r.x+i] = screen[(r.y+j)*64+r.x+i] * 0xFFFFFFFF;
                    }
                }
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            }

            if (eff.wait) {
                waiting = true;
                wait_reg = eff.wait_reg;
            }

            uint16_t keys;
            const uint8_t* state = SDL_GetKeyboardState(NULL);
            keys = (state[SDL_SCANCODE_X] << 0)
                | (state[SDL_SCANCODE_1] << 1)
                | (state[SDL_SCANCODE_2] << 2)
                | (state[SDL_SCANCODE_3] << 3)
                | (state[SDL_SCANCODE_Q] << 4)
                | (state[SDL_SCANCODE_W] << 5)
                | (state[SDL_SCANCODE_E] << 6)
                | (state[SDL_SCANCODE_A] << 7)
                | (state[SDL_SCANCODE_S] << 8)
                | (state[SDL_SCANCODE_D] << 9)
                | (state[SDL_SCANCODE_Z] << 10)
                | (state[SDL_SCANCODE_C] << 11)
                | (state[SDL_SCANCODE_4] << 12)
                | (state[SDL_SCANCODE_R] << 13)
                | (state[SDL_SCANCODE_F] << 14)
                | (state[SDL_SCANCODE_V] << 15);
            chip8.keys = keys;
        }

        if (current - last_frame >= 17) {
            if (chip8.dt > 0) chip8.dt--;
            last_frame = current;

			ImGui_ImplOpenGL2_NewFrame();
			ImGui_ImplSDL2_NewFrame(window);
			ImGui::NewFrame();
			if (stepmode) {
                drawDebugWindow(chip8);
            }
			ImGui::Render();

            glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
            glClearColor(0.f, 0.f, 0.f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT);

            glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(-1.f, 1.f);
            glTexCoord2f(1.0f, 0.0f);
            glVertex2f(1.f, 1.f);
            glTexCoord2f(1.0f, 1.0f);
            glVertex2f(1.f, -1.f);
            glTexCoord2f(.0f, 1.0f);
            glVertex2f(-1.f, -1.f);

            glEnd();


            ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(window);
        }
    }


    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
