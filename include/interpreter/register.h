//
// Created by matthew on 4/15/25.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <array>
#include <cstdint>
#include <map>
#include <string>


/**
 * Enum representing the valid register numbers
 */
enum class Register {
    ZERO,
    /**
     * Assembler temporary register, used when executing decomposed pseudo instructions
     */
    AT,
    V0,
    V1,
    A0,
    A1,
    A2,
    A3,
    T0,
    T1,
    T2,
    T3,
    T4,
    T5,
    T6,
    T7,
    S0,
    S1,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    T8,
    T9,
    /**
     * Kernel-reserved register
     */
    K0,
    /**
     * Kernel-reserved register
     */
    K1,
    /**
     * Stores a pointer to the global area of mmeory
     */
    GP,
    /**
     * Stores the current address of the stack head
     */
    SP,
    /**
     * Stores the base address of the stack at the beginning of the current procedure
     */
    FP,
    /**
     * Stores the return address of the current procedure
     */
    RA,
    /**
     * The program counter, used to track which instruction is being executed
     */
    PC,
    /**
     * Stores the high-order bits of a 64-bit operation, like multiplication or division
     */
    HI,
    /**
     * Stores the low-order bits of a 64-bit operation, like multiplication or division
     */
    LO
};


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
 * Class representing the state and labels of registers
 */
class RegisterFile {
    /**
     * A mapping between register numbers and values stored in the 32 registers + PC, HI, and LO
     */
    std::array<int32_t, 35> registers = {};

    /**
     * A mapping between the common names of registers and their register numbers
     */
    static std::map<std::string, Register> nameToIndex;

public:
    /**
     * Returns the register number associated with a name
     * @param name The name of a register
     * @return The associated register number
     * @throw runtime_error When an invalid register is requested
     */
    static int indexFromName(const std::string& name);

    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Register index) const;
    int32_t& operator[](Register index);
};


/**
 * Class representing the state of the coprocessor 0 register file
 */
class Coproc0RegisterFile {
    std::array<int32_t, 16> registers = {};

public:
    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Coproc0Register index) const;
    int32_t& operator[](Coproc0Register index);
};

#endif // REGISTER_H
