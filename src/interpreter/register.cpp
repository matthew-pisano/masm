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


int32_t RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t RegisterFile::operator[](const Register index) const {
    return registers.at(static_cast<size_t>(index));
}
int32_t& RegisterFile::operator[](const Register index) {
    return registers.at(static_cast<size_t>(index));
}


int32_t Coproc0RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc0RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t Coproc0RegisterFile::operator[](const Register index) const {
    return registers.at(static_cast<size_t>(index));
}
int32_t& Coproc0RegisterFile::operator[](const Register index) {
    return registers.at(static_cast<size_t>(index));
}
