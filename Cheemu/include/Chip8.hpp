#pragma once

#include "ExampleAppLog.hpp"

struct C8CPU {
    void setLogSink( ExampleAppLog& log );
    void reset();
    void loadROM( const char * path );
    void executeInstruction();

    // memory
    unsigned char memory[4096]; // 8-bit memory

    // stack
    unsigned short stack[16]; // stack only stores return addresses
    unsigned short sp; // stack pointer;
    unsigned short pc; // program counter
    unsigned short opcode; // 16-bit opcode

    // registers
    unsigned char VRegister[16]; // 16 8-bit registers
    unsigned short IRegister; // I register is 16-bits

    // timers
    unsigned short delayTimer;
    unsigned short soundTimer; 

    // input
    char keyboard[16];
    char lastKey = 0;
    short numPressed = 0;

    // graphics
    // screen origin is top left
    unsigned char screen[64][32];

    const char * romPath = nullptr;
};

