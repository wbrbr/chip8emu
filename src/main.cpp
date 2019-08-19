#include <SDL2/SDL.h>
#include "chip8.hpp"

void clear(SDL_Texture* texture)
{
    void* buf;
    int pitch;
    SDL_LockTexture(texture, NULL, &buf, &pitch);
    memset(buf, 0, 32*pitch);
    SDL_UnlockTexture(texture);
}

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

    SDL_Window* window = SDL_CreateWindow("chip8emu", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    clear(texture);

    SDL_Event e;

    Chip8 chip8;
    chip8.load(std::string(argv[1]));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    uint32_t last_cycle = SDL_GetTicks();
    uint32_t last_frame = SDL_GetTicks();
    bool waiting = false;
    int wait_reg;

    while (true) {

        uint32_t current = SDL_GetTicks();

        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT) break;

        int keycode;
        if (waiting && e.type == SDL_KEYDOWN && translateKey(e.key.keysym.scancode, &keycode)) {
            waiting = false;
            chip8.regs[wait_reg] = keycode;
        }
        
        if (!waiting && current - last_cycle >= 2) {
            last_cycle = current;
            SideEffects eff = chip8.cycle();

            if (eff.clear) {
                clear(texture);
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
                void* buf;
                int pitch;
                SDL_LockTexture(texture, &r, &buf, &pitch);
                bool* screen = chip8.screen;

                uint32_t* pixels = (uint32_t*)buf;

                for (int j = 0; j < r.h; j++)
                {
                    for (int i = 0; i < r.w; i++)
                    {
                        pixels[j*pitch/4+i] = screen[(r.y+j)*64+r.x+i] * 0xFFFFFFFF;
                    }
                }
                SDL_UnlockTexture(texture);
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
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            last_frame = current;
        }

        // eff.wait = true; // single stepping
        //

    }

    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
