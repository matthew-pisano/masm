//
// Created by matthew on 6/7/25.
//

#include "interpreter/cp0.h"


// Coprocessor 0 register access
int32_t Coproc0RegisterFile::operator[](const uint32_t index) const { return registers.at(index); }
int32_t& Coproc0RegisterFile::operator[](const uint32_t index) { return registers.at(index); }

int32_t Coproc0RegisterFile::operator[](const Coproc0Register index) const {
    return registers.at(static_cast<uint32_t>(index));
}
int32_t& Coproc0RegisterFile::operator[](const Coproc0Register index) {
    return registers.at(static_cast<uint32_t>(index));
}
