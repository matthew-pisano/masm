//
// Created by matthew on 4/15/25.
//

#include "register.h"

#include <stdexcept>


int RegisterFile::indexFromName(const std::string& name) {
    if (!nameToIndex.contains(name))
        throw std::runtime_error("Unknown register " + name);

    return static_cast<int>(nameToIndex[name]);
}


uint32_t RegisterFile::operator[](const int index) const { return registers[index]; }
uint32_t& RegisterFile::operator[](const int index) {
    if (index < 0 || index >= 32)
        throw std::runtime_error("Register index out of bounds: " + std::to_string(index));
    return registers[index];
}
