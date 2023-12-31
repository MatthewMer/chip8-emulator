#ifndef CHIP8_H
#define CHIP8_8

#include "config.h"
#include "Memory.h"
#include "Keyboard.h"
#include "Screen.h"
#include "charset.h"
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <vector>
#include <chrono>

using namespace std::chrono;

typedef struct registers {
    u8 V[CHIP8_REGISTERS_SIZE] = {};	// 16 registers
    u16 I = 0x00;		// mainly memory addresses
    u8 dt = 0x00;		// delay timer
    u8 st = 0x00;		// sound timer
    u16 PC = 0x00;		// program counter
    u8 SP = 0x00;		// stack pointer
    u8* VF = &V[0xf];   // VF (Carry)
    u8* V0 = V;         // V0
};


class Chip8 {
private:
    void exec_0(u16 data);
    void exec_8(u16 data);
    void exec_e(u16 data);
    void exec_f(u16 data);

    // memory
    u8 memory[CHIP8_MEMORY_SIZE] = {};
    int mem_size;

    // stack
    u16 stack[CHIP8_STACK_SIZE] = {};
    
    // registers
    registers reg;

    // await key
    u8* key_dest;

    // isntruction
    u16 instr;
    u8 error_flags = 0x00;

    // timer
    steady_clock::time_point delaytimer_prev;
    steady_clock::time_point delaytimer_cur;
    int dt_time;

    // clock
    steady_clock::time_point clock_prev;
    steady_clock::time_point clock_cur;
    int clock_time;

public:
    Chip8();

    // CPU
    u8 exec();
    void load(std::vector<char> &buf);

    // MEMORY
    u16 next_instr();
    void write_8(u16 addr, u8 val);
    u8 read_8(u16 addr);

    // STACK
    void stack_push(u16 data);
    u16 stack_pop();

    Keyboard keys;
    Screen screen;

    // access cpu components
    u8* get_memory();
    int get_mem_size();
    u16* get_stack();
    registers* get_registers();

    // rendering/sound/key press
    bool render;
    bool sound;
    bool awaits_key;
};

#endif