//
// Created by matthew on 6/7/25.
//

#include "interpreter/cp1.h"

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
            // Should never be reached
        default:
            throw std::runtime_error("Unknown Co-Processor 1 reg type instruction " +
                                     std::to_string(func));
    }
}


void execCP1RegImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, const uint32_t sub,
                       const uint32_t rt, const uint32_t fs) {}


void execCP1ImmType(Coproc1RegisterFile& cp1, Memory& memory, const uint32_t op,
                    const uint32_t base, const uint32_t ft, const uint32_t offset) {}


void execCP1CondType(Coproc1RegisterFile& cp1, const uint32_t fmt, const uint32_t ft,
                     const uint32_t fs, const uint32_t cond) {}


void execCP1CondImmType(Coproc1RegisterFile& cp1, RegisterFile& registers, const uint32_t tf,
                        const uint32_t offset) {}
