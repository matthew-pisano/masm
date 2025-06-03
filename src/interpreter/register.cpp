//
// Created by matthew on 4/15/25.
//

#include "interpreter/register.h"

#include <stdexcept>


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
