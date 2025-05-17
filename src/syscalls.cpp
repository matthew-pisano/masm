//
// Created by matthew on 4/26/25.
//

#include "syscalls.h"

#include <iostream>
#include <ostream>
#include <unistd.h>

#include "exceptions.h"


void execSyscall(State& state, std::istream& istream, std::ostream& ostream) {
    int32_t syscallCode = state.registers[Register::V0];
    switch (static_cast<Syscall>(syscallCode)) {
        case Syscall::PRINT_INT:
            printIntSyscall(state, ostream);
            break;
        case Syscall::PRINT_STRING:
            printStringSyscall(state, ostream);
            break;
        case Syscall::READ_INT:
            readIntSyscall(state, istream);
            break;
        case Syscall::READ_STRING:
            readStringSyscall(state, istream);
            break;
        case Syscall::EXIT:
            exitSyscall();
            break;
        case Syscall::PRINT_CHAR:
            printCharSyscall(state, ostream);
            break;
        case Syscall::READ_CHAR:
            readCharSyscall(state, istream);
            break;
        case Syscall::EXIT_VAL:
            exitValSyscall(state);
            break;
        default:
            throw std::runtime_error("Unknown syscall " + std::to_string(syscallCode));
    }
}


void printIntSyscall(const State& state, std::ostream& ostream) {
    const int32_t value = state.registers[Register::A0];
    ostream << value;
}


void printStringSyscall(const State& state, std::ostream& ostream) {
    int32_t address = state.registers[Register::A0];
    while (true) {
        const unsigned char c = state.memory.byteAt(address);
        if (c == '\0')
            break;
        ostream << c;
        address++;
    }
    ostream.flush();
}


void readIntSyscall(State& state, std::istream& istream) {
    std::string input;
    std::getline(istream, input);
    try {
        state.registers[Register::V0] = std::stoi(input);
    } catch (const std::invalid_argument&) {
        throw std::runtime_error("Invalid input: " + input);
    } catch (const std::out_of_range&) {
        throw std::runtime_error("Input out of range: " + input);
    }
}


void readStringSyscall(State& state, std::istream& istream) {
    const int32_t address = state.registers[Register::A0];
    const int32_t length = state.registers[Register::A1];
    int currLen = 0;
    while (currLen < length) {
        const char c = istream.get();
        if (c == '\n')
            break;
        state.memory.byteTo(address + currLen, c);
        currLen++;
    }
}


void exitSyscall() { throw ExecExit("Program exited with code " + std::to_string(0), 0); }


void printCharSyscall(const State& state, std::ostream& ostream) {
    const char c = static_cast<char>(state.registers[Register::A0]);
    ostream << c;
}


void readCharSyscall(State& state, std::istream& istream) {
    char c;
    if (&istream != &std::cin)
        istream.get(c);
    else {
        read(STDIN_FILENO, &c, 1);
        std::cout.flush();
    }
    state.registers[Register::V0] = 0xFF & static_cast<int32_t>(c);
}


void exitValSyscall(const State& state) {
    const int32_t exitCode = state.registers[Register::A0];
    throw ExecExit("Program exited with code " + std::to_string(exitCode), exitCode);
}
