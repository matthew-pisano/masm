//
// Created by matthew on 4/24/25.
//

#include "interpreter.h"


int Interpreter::interpret(const MemLayout& layout) {
    state.memory.loadProgram(layout);

    return 0;
}
