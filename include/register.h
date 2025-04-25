//
// Created by matthew on 4/15/25.
//

#ifndef REGISTER_H
#define REGISTER_H

#include <array>
#include <cstdint>
#include <map>
#include <string>


enum class Register {
    ZERO,
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
    K0,
    K1,
    GP,
    SP,
    FP,
    RA,
    PC,
    HI,
    LO
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
    std::map<std::string, Register> nameToIndex = {
            {"zero", Register::ZERO}, {"at", Register::AT}, {"v0", Register::V0},
            {"v1", Register::V1},     {"a0", Register::A0}, {"a1", Register::A1},
            {"a2", Register::A2},     {"a3", Register::A3}, {"t0", Register::T0},
            {"t1", Register::T1},     {"t2", Register::T2}, {"t3", Register::T3},
            {"t4", Register::T4},     {"t5", Register::T5}, {"t6", Register::T6},
            {"t7", Register::T7},     {"s0", Register::S0}, {"s1", Register::S1},
            {"s2", Register::S2},     {"s3", Register::S3}, {"s4", Register::S4},
            {"s5", Register::S5},     {"s6", Register::S6}, {"s7", Register::S7},
            {"t8", Register::T8},     {"t9", Register::T9}, {"k0", Register::K0},
            {"k1", Register::K1},     {"gp", Register::GP}, {"sp", Register::SP},
            {"fp", Register::FP},     {"ra", Register::RA}};

public:
    /**
     * Returns the register number associated with a name
     * @param name The name of a register
     * @return The associated register number
     * @throw runtime_error When an invalid register is requested
     */
    int indexFromName(const std::string& name);

    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Register index) const;
    int32_t& operator[](Register index);
};

#endif // REGISTER_H
