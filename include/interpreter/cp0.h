//
// Created by matthew on 6/7/25.
//

#ifndef CP0_H
#define CP0_H
#include <array>
#include <cstdint>

#include "cpu.h"

/**
 * Enum representing the valid coprocessor 0 registers
 */
enum class Coproc0Register {
    /**
     * Memory address at which bad virtual address exception occurred
     */
    VADDR = 8,
    /**
     * Status register, contains the interrupt mask and enable bits
     */
    STATUS = 12,
    /**
     * Cause register, contains the cause of the last exception
     */
    CAUSE = 13,
    /**
     * Exception Program Counter, contains the address last executed before the exception
     */
    EPC = 14
};


/**
 * Class representing the state of the coprocessor 0 register file
 */
class Coproc0RegisterFile {
    /**
     * A mapping between CP0, register numbers and values stored in the 4 CP0 registers
     */
    std::array<int32_t, 16> registers = {};

public:
    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Coproc0Register index) const;
    int32_t& operator[](Coproc0Register index);
};


/**
 * Executes the given CP0-Type instruction
 * @param cp0 The Co-Processor Zero registers to operate on
 * @param registers The CPU registers to operate on
 * @param rs The first source register
 * @param rt The second source register
 * @param rd The destination register
 */
void execCP0Type(Coproc0RegisterFile& cp0, RegisterFile& registers, uint32_t rs, uint32_t rt,
                 uint32_t rd);


/**
 * Executes the ERET instruction, which returns from an exception handler
 * @param cp0 The Co-Processor Zero registers to operate on
 * @param registers The CPU registers to operate on
 */
void execEret(Coproc0RegisterFile& cp0, RegisterFile& registers);

#endif // CP0_H
