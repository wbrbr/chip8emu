#ifndef CHIP8_HPP
#define CHIP8_HPP
#include <string>
#include <stdint.h>
#include <vector>

struct SideEffects
{
    bool clear;
    bool wait;
    int wait_reg;
    int draw_x;
    int draw_y;
    int draw_n;
};

struct Chip8
{
public:
    Chip8();
    void load(std::string rompath);
    SideEffects cycle();
    void dumpState();

    uint8_t regs[16];
    uint16_t ir;
    uint8_t memory[0xFFF];
    uint8_t dt;
    int pc;
    uint16_t keys;
    // TODO: remove this, use sp instead
    std::vector<int> stack;
    // TODO: glyphs
    //
    // uint8_t screen[64*32/8]; // 256
    bool screen[64*32];
};
#endif
