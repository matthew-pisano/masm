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

public:
    int interpret(const MemLayout& layout);
};

#endif // INTERPRETER_H
