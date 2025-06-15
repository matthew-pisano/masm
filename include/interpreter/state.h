//
// Created by matthew on 6/8/25.
//

#ifndef STATE_H
#define STATE_H
#include <cstdint>
#include <string>

#include "cp0.h"
#include "cp1.h"
#include "cpu.h"
#include "heap.h"


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

    /**
     * Constructor for the State class
     * @param useLittleEndian Whether to use little-endian memory layout (default is big-endian)
     */
    explicit State(const bool useLittleEndian = false) : memory(useLittleEndian) {}
};

#endif // STATE_H
