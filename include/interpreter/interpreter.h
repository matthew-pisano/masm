//
// Created by matthew on 4/24/25.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <memory>


#include "memory.h"
#include "state.h"
#include "syscalls.h"


/**
 * The interpreter class, which is responsible for executing MIPS instructions
 */
class Interpreter {

    /**
     * The I/O mode of the interpreter, which determines how input/output is handled
     */
    IOMode ioMode;

    /**
     * The console handle for reading input and writing output to the console
     */
    StreamHandle& streamHandle;

    /**
     * The system handle for executing system calls
     */
    SystemHandle sysHandle;

    /**
     * Reads from the input stream and updates the MMIO input ready bit and data word
     * @return True if input was read successfully, false if no input is available
     */
    bool readMMIO();

    /**
     * Writes the MMIO output data word to output stream
     * @return True if output was written successfully, false if no output is available
     */
    bool writeMMIO();

    /**
     * Handles an interrupt by executing the exception handler
     * @param cause The value of the cause register to send to the interrupt handler
     */
    void interrupt(uint32_t cause);

    /**
     * Handles exceptions that occur during execution
     * @param cause The value of the cause register to send to the exception
     * @param excMsg A message to include in the exception
     * @throw MasmRuntimeError if no instruction is found at the interrupt handler address
     */
    void except(uint32_t cause, const std::string& excMsg);

    /**
     * Decodes an instructions and determines how to interpret its value
     * @param instruction The instruction to execute
     */
    void execInstruction(int32_t instruction);

protected:
    /**
     * The current state of program memory and registers
     */
    State state;

public:
    explicit Interpreter(const IOMode ioMode, StreamHandle& streamHandle) :
        ioMode(ioMode), streamHandle(streamHandle) {}

    /**
     * Initializes the program in the interpreter with the given memory layout
     * @param layout The initial memory layout to use for loading in the program and data
     */
    void initProgram(const MemLayout& layout);

    /**
     * Executes a single program instruction at the current program state
     * @throw ExecExit if the program exits normally
     * @throw MasmRuntimeError if an error occurs during execution
     */
    void step();

    /**
     * Initializes a program and steps until an exit syscall or exception occurs
     * @param layout The initial memory layout to use for loading in the program and data
     * @return The exit code of the program
     */
    int interpret(const MemLayout& layout);
};

#endif // INTERPRETER_H
