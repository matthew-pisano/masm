//
// Created by matthew on 6/7/25.
//

#ifndef CP0_H
#define CP0_H
#include <array>
#include <cstdint>

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
    std::array<int32_t, 16> registers = {};

public:
    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Coproc0Register index) const;
    int32_t& operator[](Coproc0Register index);
};

#endif // CP0_H
