//
// Created by matthew on 4/24/25.
//

#include "interpreter.h"


int Interpreter::interpret(const MemLayout& layout) {
    state.memory.loadProgram(layout);
    // Initialize PC to the start of the text section
    state.registers[Register::PC] = memSectionOffset(MemSection::TEXT);
    state.registers[Register::SP] = 0x7FFFFFFC;
    state.registers[Register::GP] = 0x10008000;

    return 0;
}
