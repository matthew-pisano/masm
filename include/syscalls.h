//
// Created by matthew on 4/26/25.
//

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "interpreter.h"


/**
 * Enumeration of the system calls available in the MIPS architecture
 */
enum class Syscall {
    // Keyboard/Display Syscalls
    PRINT_INT = 1,
    PRINT_STRING = 4,
    READ_INT = 5,
    READ_STRING = 8,
    EXIT = 10,
    PRINT_CHAR = 11,
    READ_CHAR = 12,
    EXIT_VAL = 17,

    // MARS Extended Syscalls
    TIME = 30,
    MIDI = 31,
    SLEEP = 32,
    MIDI_SYNC = 33,
    PRINT_INT_HEX = 34,
    PRINT_INT_BIN = 35,
    PRINT_UINT = 36,
    SET_SEED = 40,
    RAND_INT = 41,
    RAND_INT_RANGE = 42
};


/**
 * Executes the system call based on the value in the $v0 register
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 * @param ostream The output stream to print to
 */
void execSyscall(State& state, std::istream& istream, std::ostream& ostream);


/**
 * Prints the integer stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printIntSyscall(const State& state, std::ostream& ostream);

/**
 * Prints the null-terminated string stored in the memory at the address in $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printStringSyscall(const State& state, std::ostream& ostream);

/**
 * Reads an integer from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readIntSyscall(State& state, std::istream& istream);

/**
 * Reads a string from the console and stores it in the memory at the address in $a0 up to the
 * length in $a1
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readStringSyscall(State& state, std::istream& istream);

/**
 * Exits the program with the exit code 0
 */
void exitSyscall();

/**
 * Prints the character stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printCharSyscall(const State& state, std::ostream& ostream);

/**
 * Reads a character from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readCharSyscall(State& state, std::istream& istream);

/**
 * Exits the program with the exit code stored in $a0
 * @param state The current state of the interpreter
 */
void exitValSyscall(const State& state);

#endif // SYSCALLS_H
