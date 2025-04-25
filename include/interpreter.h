//
// Created by matthew on 4/24/25.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "memory.h"
#include "register.h"


struct State {
    RegisterFile registers;
    Memory memory;
};


class Interpreter {
    State state;

    void execRType(uint32_t funct, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt);
    void execIType(uint32_t opCode, uint32_t rs, uint32_t rt, int32_t immediate);
    void execJType(uint32_t opCode, uint32_t address);

    void syscall();
    void step();

public:
    int interpret(const MemLayout& layout);
};

#endif // INTERPRETER_H
