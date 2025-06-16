//
// Created by matthew on 6/7/25.
//

#ifndef CP1_H
#define CP1_H
#include <array>
#include <cstdint>

#include "cpu.h"
#include "utils.h"

/**
 * Enum representing the valid coprocessor 1 (floating point) registers
 */
enum class Coproc1Register {
    F0,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    F25,
    F26,
    F27,
    F28,
    F29,
    F30,
    F31
};


/**
 * Class representing the state of the coprocessor 1 (floating point) register file
 */
class Coproc1RegisterFile {
    /**
     * A mapping between CP1 register numbers and values stored in the 32 CP1 registers
     */
    std::array<int32_t, 32> registers = {};

    /**
     * A mapping between CP1 flag numbers and the values stored in the 8 CP1 flags
     */
    std::array<bool, 8> flags = {};

public:
    /**
     * Gets the boolean value stored in one of the flags
     * @param index The index of the flag to get
     * @return The value within the CP1 flag
     */
    [[nodiscard]] bool getFlag(uint32_t index) const;

    /**
     * Sets the given flag to the given value
     * @param index The index of the flag to modify
     * @param value The value to set the flag to
     */
    void setFlag(uint32_t index, bool value);

    /**
     * Gets the float value stored in one of the CP1 registers
     * @param index The index of the register
     * @return The float value stored in the register
     */
    [[nodiscard]] float32_t getFloat(uint32_t index) const;

    /**
     * Sets the float value within one of the CP1 registers
     * @param index The index of the register to modify
     * @param value The new value of the register
     */
    void setFloat(uint32_t index, float32_t value);

    /**
     * Gets the float value stored in one of the CP1 registers
     * @param index The index of the register
     * @return The float value stored in the register
     */
    [[nodiscard]] float32_t getFloat(Coproc1Register index) const;

    /**
     * Sets the float value within one of the CP1 registers
     * @param index The index of the register to modify
     * @param value The new value of the register
     */
    void setFloat(Coproc1Register index, float32_t value);

    /**
     * Gets the double value stored in two of the CP1 registers
     * @param index The even index of the first FP register
     * @return The double value stored in the registers
     */
    [[nodiscard]] float64_t getDouble(uint32_t index) const;

    /**
     * Sets the double value within two of the CP1 registers
     * @param index The even index of the first FP register to modify
     * @param value The new value of the registers
     */
    void setDouble(uint32_t index, float64_t value);

    /**
     * Gets the double value stored in two of the CP1 registers
     * @param index The even index of the first FP register
     * @return The double value stored in the registers
     */
    [[nodiscard]] float64_t getDouble(Coproc1Register index) const;

    /**
     * Sets the double value within two of the CP1 registers
     * @param index The even index of the first FP register to modify
     * @param value The new value of the registers
     */
    void setDouble(Coproc1Register index, float64_t value);

    /**
     * Returns the register number associated with a name
     * @param name The name of a register
     * @return The associated register number
     * @throw runtime_error When an invalid register is requested
     */
    static int indexFromName(const std::string& name);

    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Coproc1Register index) const;
    int32_t& operator[](Coproc1Register index);
};


/**
 * Executes the given register type FP instruction
 * @param cp1 The Co-Processor One registers to operate on
 * @param fmt The format of the instruction (single, double)
 * @param ft The first source register
 * @param fs The second source register
 * @param fd The destination register
 * @param func The function code of the instruction
 */
void execCP1RegType(Coproc1RegisterFile& cp1, uint32_t fmt, uint32_t ft, uint32_t fs, uint32_t fd,
                    uint32_t func);


/**
 * Executes the given register immediate type FP instruction
 * @param cp1 The Co-Processor One registers to operate on
 * @param registers The CPU registers to operate on
 * @param sub The instuction subtype
 * @param rt The CPU source register
 * @param fs The CP1 source register
 */
void execCP1RegImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, uint32_t sub, uint32_t rt,
                       uint32_t fs);


/**
 * Executes the given immediate type FP instruction
 * @param cp1 The Co-Processor One registers to operate on
 * @param registers The CPU registers to operate on
 * @param memory The memory state to operate on
 * @param op The opcode of the instruction
 * @param base The base register of the instruction
 * @param ft The first source register
 * @param offset The offset from the base register
 */
void execCP1ImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, Memory& memory, uint32_t op,
                    uint32_t base, uint32_t ft, uint32_t offset);


/**
 *
 * @param cp1 The Co-Processor One registers to operate on
 * @param fmt The format of the instruction (single, double)
 * @param ft The first source register
 * @param fs The second source register
 * @param cond The condition type
 */
void execCP1CondType(Coproc1RegisterFile& cp1, uint32_t fmt, uint32_t ft, uint32_t fs,
                     uint32_t cond);


/**
 *
 * @param cp1 The Co-Processor One registers to operate on
 * @param registers The CPU registers to operate on
 * @param tf The source register
 * @param offset The offset from the instruction location
 */
void execCP1CondImmType(const Coproc1RegisterFile& cp1, RegisterFile& registers, uint32_t tf,
                        int32_t offset);

#endif // CP1_H
