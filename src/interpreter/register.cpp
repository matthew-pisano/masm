//
// Created by matthew on 4/15/25.
//

#include "interpreter/register.h"

#include <stdexcept>


std::map<std::string, Register> RegisterFile::nameToIndex = {
        {"zero", Register::ZERO}, {"at", Register::AT}, {"v0", Register::V0}, {"v1", Register::V1},
        {"a0", Register::A0},     {"a1", Register::A1}, {"a2", Register::A2}, {"a3", Register::A3},
        {"t0", Register::T0},     {"t1", Register::T1}, {"t2", Register::T2}, {"t3", Register::T3},
        {"t4", Register::T4},     {"t5", Register::T5}, {"t6", Register::T6}, {"t7", Register::T7},
        {"s0", Register::S0},     {"s1", Register::S1}, {"s2", Register::S2}, {"s3", Register::S3},
        {"s4", Register::S4},     {"s5", Register::S5}, {"s6", Register::S6}, {"s7", Register::S7},
        {"t8", Register::T8},     {"t9", Register::T9}, {"k0", Register::K0}, {"k1", Register::K1},
        {"gp", Register::GP},     {"sp", Register::SP}, {"fp", Register::FP}, {"ra", Register::RA}};


int RegisterFile::indexFromName(const std::string& name) {
    if (!nameToIndex.contains(name))
        throw std::runtime_error("Unknown register " + name);

    return static_cast<int>(nameToIndex[name]);
}


// Core register access
int32_t RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t RegisterFile::operator[](const Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& RegisterFile::operator[](const Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


// Coprocessor 0 register access
int32_t Coproc0RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc0RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t Coproc0RegisterFile::operator[](const Coproc0Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& Coproc0RegisterFile::operator[](const Coproc0Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


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
    const float64_t lower = *reinterpret_cast<const float32_t*>(&registers.at(index));
    const float64_t upper = *reinterpret_cast<const float32_t*>(&registers.at(index + 1));
    return (upper * 0x100000000) + lower; // Combine the two 32-bit parts into a 64-bit float
}

void Coproc1RegisterFile::setDouble(const uint32_t index, const float64_t value) {
    const float32_t lower = static_cast<float32_t>(value);
    const float32_t upper = static_cast<float32_t>(value / 0x100000000);
    registers.at(index) = *reinterpret_cast<const int32_t*>(&lower);
    registers.at(index + 1) = *reinterpret_cast<const int32_t*>(&upper);
}

float64_t Coproc1RegisterFile::getDouble(const Coproc1Register index) const {
    return getDouble(static_cast<uint32_t>(index));
}

void Coproc1RegisterFile::setDouble(const Coproc1Register index, const float64_t value) {
    setDouble(static_cast<uint32_t>(index), value);
}

int32_t Coproc1RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc1RegisterFile::operator[](const uint32_t index) { return registers.at(index); }
int32_t Coproc1RegisterFile::operator[](const Coproc0Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& Coproc1RegisterFile::operator[](const Coproc0Register index) {
    return registers.at(static_cast<uint32_t>(index));
}


// Coprocessor 1 flags access
bool Coproc1RegisterFile::getFlag(const uint32_t index) const { return flags.at(index); }
void Coproc1RegisterFile::setFlag(const uint32_t index, const bool value) {
    flags.at(index) = value;
}
