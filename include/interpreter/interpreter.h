//
// Created by matthew on 4/24/25.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <memory>


#include "cp0.h"
#include "cp1.h"
#include "cpu.h"
#include "heap.h"
#include "memory.h"


/**
 * The possible interrupt codes for keyboard and display input/output (bit [8-9] of cause register)
 */
enum class INTERP_CODE { KEYBOARD_INTERP = 0x0100, DISPLAY_INTERP = 0x0200 };


/**
 * Converts the value of the cause register to a human-readable error string
 * @param cause The value of the cause register to convert
 * @return A string representation of the cause register value
 */
std::string causeToString(uint32_t cause);


/**
 * Enumeration of the I/O modes for the interpreter
 */
enum class IOMode {
    SYSCALL, // System call mode for reading/writing
    MMIO // Memory-mapped I/O mode for reading/writing MMIO registers
};


/**
 * The state of the interpreter, which includes the register file, memory, the heap, and debug info
 */
struct State {
    /**
     * The register file containing the values of all registers
     */
    RegisterFile registers;

    /**
     * The coprocessor 0 register file, which contains special registers for handling exceptions
     */
    Coproc0RegisterFile cp0;

    /**
     * The coprocessor 1 register file, which contains floating-point registers and operations
     */
    Coproc1RegisterFile cp1;

    /**
     * The main memory of the interpreter, which contains the program code and data
     */
    Memory memory;

    /**
     * The heap allocator for dynamic memory allocation
     */
    HeapAllocator heapAllocator;

    /**
     * The main memory map between indices and the locators for their source code, used for errors
     */
    std::unordered_map<uint32_t, std::shared_ptr<SourceLocator>> debugInfo;

    /**
     * Gets the source line locator for the given executable address
     * @param addr The address to get the source line for
     * @return The source line locator corresponding to the given address
     */
    SourceLocator getDebugInfo(uint32_t addr) const;

    /**
     * Loads a program and initial static data into memory, along with source locators for text
     * @param layout The memory layout to load
     */
    void loadProgram(const MemLayout& layout);
};


/**
 * The interpreter class, which is responsible for executing MIPS instructions
 */
class Interpreter {

    /**
     * The I/O mode of the interpreter, which determines how input/output is handled
     */
    IOMode ioMode;

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

    /**
     * Executes the ERET instruction, which returns from an exception handler
     */
    void execEret();

    /**
     * Executes the given CP0-Type instruction
     */
    void execCP0Type(uint32_t rs, uint32_t rt, uint32_t rd);

    void execCP1RegType(uint32_t fmt, uint32_t ft, uint32_t fs, uint32_t fd, uint32_t func);

    void execCP1RegImmType(uint32_t sub, uint32_t rt, uint32_t fs);

    void execCP1ImmType(uint32_t op, uint32_t base, uint32_t ft, uint32_t offset);

    void execCP1CondType(uint32_t fmt, uint32_t ft, uint32_t fs, uint32_t cond);

    void execCP1CondImmType(uint32_t tf, uint32_t offset);

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

    void execInstruction(int32_t instruction);


protected:
    /**
     * The current state of program memory and registers
     */
    State state;

public:
    explicit Interpreter(const IOMode ioMode) :
        ioMode(ioMode), istream(std::cin), ostream(std::cout) {}
    Interpreter(const IOMode ioMode, std::istream& input, std::ostream& output) :
        ioMode(ioMode), istream(input), ostream(output) {}

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
