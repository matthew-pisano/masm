//
// Created by matthew on 6/7/25.
//

#include "interpreter/cp1.h"

#include <cmath>
#include <stdexcept>

#include "interpreter/cpu.h"
#include "interpreter/memory.h"
#include "parser/instruction.h"


// Coprocessor 1 (floating point) register access
float32_t Coproc1RegisterFile::getFloat(const uint32_t index) const {
    return *reinterpret_cast<const float32_t*>(&registers.at(index));
}

void Coproc1RegisterFile::setFloat(const uint32_t index, const float32_t value) {
    registers.at(index) = *reinterpret_cast<const int32_t*>(&value);
}

float32_t Coproc1RegisterFile::getFloat(const Coproc1Register index) const {
    return getFloat(static_cast<uint32_t>(index));
}

void Coproc1RegisterFile::setFloat(const Coproc1Register index, const float32_t value) {
    setFloat(static_cast<uint32_t>(index), value);
}

float64_t Coproc1RegisterFile::getDouble(const uint32_t index) const {
    if (index % 2 != 0)
        throw std::runtime_error("Invalid double precision register: f" + std::to_string(index));

    const int32_t lower = registers.at(index); // Lower 32 bits
    const int32_t upper = registers.at(index + 1); // Upper 32 bits
    const int64_t valueInt = static_cast<int64_t>(upper) << 32 | static_cast<uint32_t>(lower);
    return *reinterpret_cast<const float64_t*>(&valueInt);
}

void Coproc1RegisterFile::setDouble(const uint32_t index, const float64_t value) {
    if (index % 2 != 0)
        throw std::runtime_error("Invalid double precision register: f" + std::to_string(index));

    const int64_t valueInt = *reinterpret_cast<const int64_t*>(&value);
    registers.at(index) = static_cast<int32_t>(valueInt & 0xFFFFFFFF); // Lower 32 bits
    registers.at(index + 1) = static_cast<int32_t>(valueInt >> 32 & 0xFFFFFFFF); // Upper 32 bits
}

float64_t Coproc1RegisterFile::getDouble(const Coproc1Register index) const {
    return getDouble(static_cast<uint32_t>(index));
}

void Coproc1RegisterFile::setDouble(const Coproc1Register index, const float64_t value) {
    setDouble(static_cast<uint32_t>(index), value);
}

int Coproc1RegisterFile::indexFromName(const std::string& name) {
    if (name.starts_with("f") && isSignedInteger(name.substr(1)) &&
        std::stoi(name.substr(1)) >= 0) {
        // Handle floating point registers ($f0-$f31)
        return std::stoi(name.substr(1));
    }
    throw std::runtime_error("Unknown register " + name);
}

int32_t Coproc1RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc1RegisterFile::operator[](const uint32_t index) { return registers.at(index); }
int32_t Coproc1RegisterFile::operator[](const Coproc1Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& Coproc1RegisterFile::operator[](const Coproc1Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


// Coprocessor 1 flags access
bool Coproc1RegisterFile::getFlag(const uint32_t index) const { return flags.at(index); }
void Coproc1RegisterFile::setFlag(const uint32_t index, const bool value) {
    flags.at(index) = value;
}


void execCP1RegType(Coproc1RegisterFile& cp1, const uint32_t fmt, const uint32_t ft,
                    const uint32_t fs, const uint32_t fd, const uint32_t func) {
    const bool singlePrecision = fmt == 0x10; // Single precision format
    switch (static_cast<InstructionCode>(func)) {
        case InstructionCode::FP_ABS: {
            if (singlePrecision)
                cp1.setFloat(fd, std::abs(cp1.getFloat(fs)));
            else
                cp1.setDouble(fd, std::abs(cp1.getDouble(fs)));
            break;
        }
        case InstructionCode::FP_ADD: {
            if (singlePrecision) {
                const float32_t result = cp1.getFloat(fs) + cp1.getFloat(ft);
                cp1.setFloat(fd, result);
            } else {
                const float64_t result = cp1.getDouble(fs) + cp1.getDouble(ft);
                cp1.setDouble(fd, result);
            }
            break;
        }
        case InstructionCode::FP_CVT_D: {
            // Must be single to double conversion
            const float64_t result = cp1.getFloat(fs);
            cp1.setDouble(fd, result);
            break;
        }
        case InstructionCode::FP_CVT_S: {
            // Must be double to single conversion
            const float32_t result = static_cast<float32_t>(cp1.getDouble(fs));
            cp1.setFloat(fd, result);
            break;
        }
        case InstructionCode::FP_DIV: {
            if (singlePrecision) {
                const float32_t result = cp1.getFloat(fs) / cp1.getFloat(ft);
                cp1.setFloat(fd, result);
            } else {
                const float64_t result = cp1.getDouble(fs) / cp1.getDouble(ft);
                cp1.setDouble(fd, result);
            }
            break;
        }
        case InstructionCode::FP_MOV: {
            // Move operation, just copy the value
            if (singlePrecision)
                cp1.setFloat(fd, cp1.getFloat(fs));
            else
                cp1.setDouble(fd, cp1.getDouble(fs));
            break;
        }
        case InstructionCode::FP_MUL: {
            if (singlePrecision) {
                const float32_t result = cp1.getFloat(fs) * cp1.getFloat(ft);
                cp1.setFloat(fd, result);
            } else {
                const float64_t result = cp1.getDouble(fs) * cp1.getDouble(ft);
                cp1.setDouble(fd, result);
            }
            break;
        }
        case InstructionCode::FP_NEG: {
            if (singlePrecision)
                cp1.setFloat(fd, -cp1.getFloat(fs));
            else
                cp1.setDouble(fd, -cp1.getDouble(fs));
            break;
        }
        case InstructionCode::FP_SQRT: {
            if (singlePrecision) {
                const float32_t result = std::sqrt(cp1.getFloat(fs));
                cp1.setFloat(fd, result);
            } else {
                const float64_t result = std::sqrt(cp1.getDouble(fs));
                cp1.setDouble(fd, result);
            }
            break;
        }
        case InstructionCode::FP_SUB: {
            if (singlePrecision) {
                const float32_t result = cp1.getFloat(fs) - cp1.getFloat(ft);
                cp1.setFloat(fd, result);
            } else {
                const float64_t result = cp1.getDouble(fs) - cp1.getDouble(ft);
                cp1.setDouble(fd, result);
            }
            break;
        }
        // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 reg type instruction " +
                                     std::to_string(func));
    }
}


void execCP1RegImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, const uint32_t sub,
                       const uint32_t rt, const uint32_t fs) {
    switch (static_cast<InstructionCode>(sub)) {
        case InstructionCode::FP_MFC1: {
            // Move from CP1 to CPU register
            registers[rt] = cp1[fs];
            break;
        }
        case InstructionCode::FP_MTC1: {
            // Move from CPU register to CP1
            cp1[fs] = registers[rt];
            break;
        }
        // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 reg immediate type instruction " +
                                     std::to_string(sub));
    }
}


void execCP1ImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, Memory& memory,
                    const uint32_t op, const uint32_t base, const uint32_t ft,
                    const uint32_t offset) {
    const uint32_t address = registers[base] + offset;
    switch (static_cast<InstructionCode>(op)) {
        case InstructionCode::FP_LDC1: {
            if (ft % 2 != 0)
                throw std::runtime_error("Invalid double precision register: f" +
                                         std::to_string(ft));

            cp1[ft] = memory.wordAt(address); // Lower 32 bits
            cp1[ft + 1] = memory.wordAt(address + 4); // Upper 32 bits
            break;
        }
        case InstructionCode::FP_LWC1:
            cp1[ft] = memory.wordAt(address);
            break;
        case InstructionCode::FP_SDC1: {
            if (ft % 2 != 0)
                throw std::runtime_error("Invalid double precision register: f" +
                                         std::to_string(ft));

            memory.wordTo(address, cp1[ft]); // Lower 32 bits
            memory.wordTo(address + 4, cp1[ft + 1]); // Upper 32 bits
            break;
        }
        case InstructionCode::FP_SWC1: {
            memory.wordTo(address, cp1[ft]);
            break;
        }
        // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 immediate type instruction " +
                                     std::to_string(op));
    }
}


void execCP1CondType(Coproc1RegisterFile& cp1, const uint32_t fmt, const uint32_t ft,
                     const uint32_t fs, const uint32_t cond) {
    const bool singlePrecision = fmt == 0x10; // Single precision format
    float64_t fsVal;
    float64_t ftVal;

    if (singlePrecision) {
        fsVal = cp1.getFloat(fs);
        ftVal = cp1.getFloat(ft);
    } else {
        fsVal = cp1.getDouble(fs);
        ftVal = cp1.getDouble(ft);
    }

    switch (static_cast<InstructionCode>(cond)) {
        case InstructionCode::FP_C_EQ:
            cp1.setFlag(0, fsVal == ftVal);
            break;
        case InstructionCode::FP_C_LT:
            cp1.setFlag(0, fsVal < ftVal);
            break;
        case InstructionCode::FP_C_LE:
            cp1.setFlag(0, fsVal <= ftVal);
            break;
        // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 conditional instruction " +
                                     std::to_string(cond));
    }
}


void execCP1CondImmType(const Coproc1RegisterFile& cp1, RegisterFile& registers, const uint32_t tf,
                        const int32_t offset) {
    switch (static_cast<InstructionCode>(tf)) {
        case InstructionCode::FP_BC1F:
            if (!cp1.getFlag(0))
                registers[Register::PC] += offset << 2; // Offset is word-aligned, so shift by 2
            break;
        case InstructionCode::FP_BC1T:
            if (cp1.getFlag(0))
                registers[Register::PC] += offset << 2; // Offset is word-aligned, so shift by 2
            break;
        // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 conditional instruction " +
                                     std::to_string(tf));
    }
}
