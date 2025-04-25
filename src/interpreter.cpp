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


void Interpreter::step() {
    const uint32_t instruction = state.memory.wordAt(state.registers[Register::PC]);
    state.registers[Register::PC] += 4;

    if (instruction == 0x0000000C) {
        // Syscall instruction
        syscall();
        return;
    }

    const uint32_t opCode = (instruction >> 26) & 0x3F;
    if (opCode == 0) {
        // R-Type instruction
        const uint32_t funct = instruction & 0x3F;
        const uint32_t rs = (instruction >> 21) & 0x1F;
        const uint32_t rt = (instruction >> 16) & 0x1F;
        const uint32_t rd = (instruction >> 11) & 0x1F;
        const uint32_t shamt = (instruction >> 6) & 0x1F;

        // Execute R-Type instruction
        execRType(funct, rs, rt, rd, shamt);
    } else if (opCode == 2 || opCode == 3) {
        // J-Type instruction
        execJType(opCode, instruction & 0x3FFFFFF);
    } else {
        // I-Type instruction
        const uint32_t rs = (instruction >> 21) & 0x1F;
        const uint32_t rt = (instruction >> 16) & 0x1F;
        int32_t immediate = instruction & 0xFFFF;

        // Sign-extend immediate value
        if (immediate & 0x8000)
            immediate |= 0xFFFF0000;

        // Execute I-Type instruction
        execIType(opCode, rs, rt, immediate);
    }
}


void Interpreter::execRType(uint32_t funct, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt) {
}


void Interpreter::execIType(uint32_t opCode, uint32_t rs, uint32_t rt, int32_t immediate) {}


void Interpreter::execJType(const uint32_t opCode, const uint32_t address) {
    if (opCode == 3)
        // Jump and link
        state.registers[Register::RA] = state.registers[Register::PC]; // PC incremented earlier

    // Jump to the target address
    state.registers[Register::PC] = (state.registers[Register::PC] & 0xF0000000) | (address << 2);
}


void Interpreter::syscall() {}
