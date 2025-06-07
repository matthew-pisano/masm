//
// Created by matthew on 6/7/25.
//

#ifndef CP1_H
#define CP1_H
#include <array>
#include <cstdint>

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
    std::array<int32_t, 32> registers = {};
    std::array<bool, 8> flags = {};

public:
    [[nodiscard]] bool getFlag(uint32_t index) const;
    void setFlag(uint32_t index, bool value);

    [[nodiscard]] float32_t getFloat(uint32_t index) const;
    void setFloat(uint32_t index, float32_t value);

    [[nodiscard]] float32_t getFloat(Coproc1Register index) const;
    void setFloat(Coproc1Register index, float32_t value);

    [[nodiscard]] float64_t getDouble(uint32_t index) const;
    void setDouble(uint32_t index, float64_t value);

    [[nodiscard]] float64_t getDouble(Coproc1Register index) const;
    void setDouble(Coproc1Register index, float64_t value);

    int32_t operator[](uint32_t index) const;
    int32_t& operator[](uint32_t index);

    int32_t operator[](Coproc1Register index) const;
    int32_t& operator[](Coproc1Register index);
};

#endif //CP1_H
