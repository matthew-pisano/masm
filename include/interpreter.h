//
// Created by matthew on 4/24/25.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <stdexcept>


#include "memory.h"
#include "register.h"


/**
 * The state of the interpreter, which includes the register file and memory
 */
struct State {
    RegisterFile registers;
    Memory memory;
};


/**
 * The interpreter class, which is responsible for executing MIPS instructions
 */
class Interpreter {
    /**
     * The current state of program memory and registers
     */
    State state;

    /**
     * The input stream for the program to use
     */
    std::istream& istream;

    /**
     * The output stream for the program to use
     */
    std::ostream& ostream;

    /**
     * Executes the given R-Type instruction
     * @param funct The function code of the instruction
     * @param rs The first source register
     * @param rt The second source register
     * @param rd The destination register
     * @param shamt The shift amount
     */
    void execRType(uint32_t funct, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt);

    /**
     * Executes the given I-Type instruction
     * @param opCode The opcode of the instruction
     * @param rs The first source register
     * @param rt The second source register
     * @param immediate The immediate value
     */
    void execIType(uint32_t opCode, uint32_t rs, uint32_t rt, int32_t immediate);

    /**
     * Executes the given J-Type instruction
     * @param opCode The opcode of the instruction
     * @param address The address to jump to
     */
    void execJType(uint32_t opCode, uint32_t address);

public:
    Interpreter() : istream(std::cin), ostream(std::cout) {}
    Interpreter(std::istream& input, std::ostream& output) : istream(input), ostream(output) {}

    /**
     * Executes a single program instruction at the current program state
     */
    void step();

    /**
     * Executes the program until an exit syscall or exception occurs
     * @param layout The initial memory layout to use for loading in the program and data
     * @return The exit code of the program
     */
    int interpret(const MemLayout& layout);
};

#endif // INTERPRETER_H
