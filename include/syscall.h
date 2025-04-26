//
// Created by matthew on 4/26/25.
//

#ifndef SYSCALL_H
#define SYSCALL_H
#include "interpreter.h"


/**
 * Enumeration of the system calls available in the MIPS architecture
 */
enum class Syscall {
    PRINT_INT = 1,
    PRINT_STRING = 4,
    READ_INT = 5,
    READ_STRING = 8,
    EXIT = 10,
    PRINT_CHAR = 11,
    READ_CHAR = 12,
    EXIT_VAL = 17
};


/**
 * Prints the integer stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printIntSyscall(State state, std::ostream& ostream);

/**
 * Prints the null-terminated string stored in the memory at the address in $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printStringSyscall(State state, std::ostream& ostream);

/**
 * Reads an integer from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readIntSyscall(State state, std::istream& istream);

/**
 * Reads a string from the console and stores it in the memory at the address in $a0 up to the
 * length in $a1
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readStringSyscall(State state, std::istream& istream);

/**
 * Exits the program with the exit code 0
 */
void exitSyscall();

/**
 * Prints the character stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printCharSyscall(State state, std::ostream& ostream);

/**
 * Reads a character from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readCharSyscall(State state, std::istream& istream);

/**
 * Exits the program with the exit code stored in $a0
 * @param state The current state of the interpreter
 */
void exitValSyscall(State state);

#endif // SYSCALL_H
