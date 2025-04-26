//
// Created by matthew on 4/24/25.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H
#include <iostream>
#include <stdexcept>


#include "memory.h"
#include "register.h"


class ExecExit final : public std::runtime_error {
    int errorCode;

public:
    explicit ExecExit(const std::string& message, const int code) :
        std::runtime_error(message), errorCode(code) {}

    [[nodiscard]] int code() const { return errorCode; }
};


struct State {
    RegisterFile registers;
    Memory memory;
};


class Interpreter {
    State state;
    std::istream<char> istream = std::cin;
    std::ostream<char> ostream = std::cout;

    void execRType(uint32_t funct, uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt);
    void execIType(uint32_t opCode, uint32_t rs, uint32_t rt, int32_t immediate);
    void execJType(uint32_t opCode, uint32_t address);

    void syscall();
    void step();

public:
    Interpreter() = default;
    Interpreter(std::istream& input, std::ostream& output) : istream(input), ostream(output) {}

    int interpret(const MemLayout& layout);
};

#endif // INTERPRETER_H
