#include "chip8.hpp"
#include <string.h>
#include <stdio.h>
#include <assert.h>

Chip8::Chip8()
{
    memset(regs, 0, sizeof(regs));
    memset(memory, 0, sizeof(memory));
    memset(screen, 0, sizeof(screen));

    // "0"
    memory[0] = 0xF0;
    memory[1] = 0x90;
    memory[2] = 0x90;
    memory[3] = 0x90;
    memory[4] = 0xF0;

    // "1"
    memory[5] = 0x20;
    memory[6] = 0x60;
    memory[7] = 0x20;
    memory[8] = 0x20;
    memory[9] = 0x70;

    // "2"
    memory[10] = 0xF0;
    memory[11] = 0x10;
    memory[12] = 0xF0;
    memory[13] = 0x80;
    memory[14] = 0xF0;

    // "3"
    memory[15] = 0xF0;
    memory[16] = 0x10;
    memory[17] = 0xF0;
    memory[18] = 0x10;
    memory[19] = 0xF0;

    // "4"
    memory[20] = 0x90;
    memory[21] = 0x90;
    memory[22] = 0xF0;
    memory[23] = 0x10;
    memory[24] = 0x10;

    // "5"
    memory[25] = 0xF0;
    memory[26] = 0x80;
    memory[27] = 0xF0;
    memory[28] = 0x10;
    memory[29] = 0xF0;

    // "6"
    memory[30] = 0xF0;
    memory[31] = 0x80;
    memory[32] = 0xF0;
    memory[33] = 0x90;
    memory[34] = 0xF0;

    // "7"
    memory[35] = 0xF0;
    memory[36] = 0x10;
    memory[37] = 0x20;
    memory[38] = 0x40;
    memory[39] = 0x40;

    // "8"
    memory[40] = 0xF0;
    memory[41] = 0x90;
    memory[42] = 0xF0;
    memory[43] = 0x90;
    memory[44] = 0xF0;

    // "9"
    memory[45] = 0xF0;
    memory[46] = 0x90;
    memory[47] = 0xF0;
    memory[48] = 0x10;
    memory[49] = 0xF0;

    // "A"
    memory[50] = 0xF0;
    memory[51] = 0x90;
    memory[52] = 0xF0;
    memory[53] = 0x90;
    memory[54] = 0x90;

    // "B"
    memory[55] = 0xE0;
    memory[56] = 0x90;
    memory[57] = 0xE0;
    memory[58] = 0x90;
    memory[59] = 0xE0;

    // "C"
    memory[60] = 0xF0;
    memory[61] = 0x80;
    memory[62] = 0x80;
    memory[63] = 0x80;
    memory[64] = 0xF0;

    // "D"
    memory[65] = 0xE0;
    memory[66] = 0x90;
    memory[67] = 0x90;
    memory[68] = 0x90;
    memory[69] = 0xE0;

    // "E"
    memory[70] = 0xF0;
    memory[71] = 0x80;
    memory[72] = 0xF0;
    memory[73] = 0x80;
    memory[74] = 0xF0;

    // "F"
    memory[75] = 0xF0;
    memory[76] = 0x80;
    memory[77] = 0xF0;
    memory[78] = 0x80;
    memory[79] = 0x80;

    keys = 0;
    dt = 0;
    pc = 0x200;
}

void Chip8::load(std::string rompath)
{
    FILE* fp = fopen(rompath.c_str(), "r");
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (fread(&memory[0x200], 1, size, fp) != size) {
        perror("fread: ");
        exit(1);
    }
}

SideEffects Chip8::cycle()
{
    uint16_t instr = (memory[pc] << 8) | memory[pc+1];
    pc += 2;

    SideEffects eff;
    eff.clear = false;
    eff.wait = false;
    eff.draw_n = 0;

    if (instr == 0x00E0) { // CLS
        memset(screen, 0, sizeof(screen));
        eff.clear = true;
    } else if (instr == 0x00EE) { // RET
        pc = stack.back();
        stack.pop_back();
    } else if (instr >> 12 == 0) {
        fprintf(stderr, "Machine language subroutine are not supported\n");
        exit(1);
    } else if (instr >> 12 == 1) { // JP
        pc = instr & 0x0FFF;
    } else if (instr >> 12 == 2) { // CALL
        stack.push_back(pc);
        pc = instr & 0x0FFF;
    } else if (instr >> 12 == 3) { // SE
        uint8_t kk = instr & 0x00FF;
        uint8_t x = (instr >> 8) & 0x0F;
        if (regs[x] == kk) pc += 2;
    } else if (instr >> 12 == 4) { // SNE
        uint8_t kk = instr & 0x00FF;
        uint8_t x = (instr >> 8) & 0x0F;
        if (regs[x] != kk) pc += 2;
    } else if (instr >> 12 == 5) { // SE
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        if (regs[x] == regs[y]) pc += 2;
    } else if (instr >> 12 == 6) { // LD
        uint8_t kk = instr & 0x00FF;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = kk;
    } else if (instr >> 12 == 7) { // ADD
        uint8_t kk = instr & 0x00FF;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = regs[x] + kk; 
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 0) { // LD
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = regs[y];
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 1) { // OR
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = regs[x] | regs[y]; 
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 2) { // AND
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = regs[x] & regs[y];
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 3) { // XOR
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[x] = regs[x] ^ regs[y];
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 4) { // ADD
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        uint16_t r = regs[x] + regs[y];
        regs[x] = r & 0x00FF;
        regs[0xf] = r > 255;
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 5) { // SUB
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[0xf] = regs[x] > regs[y];
        regs[x] = regs[x] - regs[y];
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 6) { // SHR
        uint8_t x = (instr >> 8) & 0x0F;
        regs[0xf] = regs[x] & 1;
        regs[x] = regs[x] >> 1;
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 7) { // SUBN
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        regs[0xf] = regs[y] > regs[x];
        regs[x] = regs[y] - regs[x];
    } else if (instr >> 12 == 8 && (instr & 0x000F) == 0xE) { // SHL
        uint8_t x = (instr >> 8) & 0x0F;
        regs[0xf] = regs[x] >> 7;
        regs[x] = regs[x] << 1;
    } else if (instr >> 12 == 9 && (instr & 0x000F) == 0) { // SNE
        uint8_t y = (instr >> 4) & 0x00F;
        uint8_t x = (instr >> 8) & 0x0F;
        if (regs[x] != regs[y]) pc += 2; 
    } else if (instr >> 12 == 0xa) { // LD
        ir = instr & 0x0FFF;
    } else if (instr >> 12 == 0xb) { // JP
        pc = regs[0] + (instr & 0x0FFF);
    } else if (instr >> 12 == 0xc) { // RND
        uint8_t kk = instr & 0x00FF;
        uint8_t x = (instr >> 8) & 0xF;
        regs[x] = rand() & kk;
    } else if (instr >> 12 == 0xd) { // DRW
        uint8_t n = instr & 0x000F;
        uint8_t y = (instr >> 4) & 0xF;
        uint8_t x = (instr >> 8) & 0xF; 
        eff.draw_n = n;
        eff.draw_x = regs[x];
        eff.draw_y = regs[y];

        for (uint8_t i = 0; i < n; i++)
        {
            uint8_t yp = regs[y] + i;
            uint8_t byte = memory[ir+i];
            for (int j = 0; j < 8; j++)
            {
                bool r = (bool)((byte >> j) & 1);
                uint8_t xp = regs[x] + 7 - j;
                xp = xp % 64;
                regs[0xf] = r && screen[yp*64+xp];
                screen[yp*64+xp] ^= r;
            }
        }
    } else if (instr >> 12 == 0xe && (instr & 0xFF) == 0x9E) { // SKP
        uint8_t x = (instr >> 8) & 0xF;
        if ((keys >> regs[x]) & 1) pc += 2;
    } else if (instr >> 12 == 0xe && (instr & 0xFF) == 0xA1) { // SKNP
        uint8_t x = (instr >> 8) & 0xF;
        if (((keys >> regs[x]) & 1) == 0) pc += 2;
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x07) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        regs[x] = dt;
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x0A) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        eff.wait = true;
        eff.wait_reg = x;
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x15) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        dt = regs[x];
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x18) { // LD
        // TODO
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x1E) { // ADD
        uint8_t x = (instr >> 8) & 0xF;
        ir = ir + regs[x];
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x29) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        ir = 5*regs[x];
    } else if (instr >> 12 == 0xf && (instr & 0xFF) == 0x33) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        uint8_t v = regs[x];
        memory[ir] = v / 100;
        memory[ir+1] = (v / 10) % 10;
        memory[ir+2] = v % 10;
    } else if (instr >> 12 == 0xF && (instr & 0xFF) == 0x55) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        memcpy(&memory[ir], regs, x+1);
        ir += x + 1;
    } else if (instr >> 12 == 0xF && (instr & 0xFF) == 0x65) { // LD
        uint8_t x = (instr >> 8) & 0xF;
        for (int i = 0; i <= x; i++)
        {
            regs[i] = memory[ir+i];
        }
        ir += x + 1;
    } else {
        fprintf(stderr, "Unknown instruction: %x\n", instr);
        exit(1);
    }

    // dumpState();

    return eff;
}

void Chip8::dumpState()
{
    for (int i = 0; i < 16; i++)
    {
        printf("V%x = %d\n", i, regs[i]);
    }
    printf("I = %d\n\n", ir);
}
