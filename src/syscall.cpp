//
// Created by matthew on 4/26/25.
//

#include "syscall.h"

#include <iostream>
#include <ostream>


void printIntSyscall(State state, std::ostream& ostream) {
    const int32_t value = state.registers[Register::A0];
    ostream << value;
}


void printStringSyscall(State state, std::ostream& ostream) {
    int32_t address = state.registers[Register::A0];
    while (true) {
        const char c = state.memory.byteAt(address);
        if (c == '\0')
            break;
        ostream << c;
        address++;
    }
}


void readIntSyscall(State state, std::istream& istream) {
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


void readStringSyscall(State state, std::istream& istream) {
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


void printCharSyscall(State state, std::ostream& ostream) {
    const char c = static_cast<char>(state.registers[Register::A0]);
    ostream << c;
}


void readCharSyscall(State state, std::istream& istream) {
    char c;
    istream.get(c);
    state.registers[Register::V0] = static_cast<uint32_t>(c);
}


void exitValSyscall(State state) {
    const int32_t exitCode = state.registers[Register::A0];
    throw ExecExit("Program exited with code " + std::to_string(exitCode), exitCode);
}
