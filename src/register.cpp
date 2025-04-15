//
// Created by matthew on 4/15/25.
//

#include "register.h"


int RegisterFile::indexFromName(const std::string& name) { return nameToIndex[name]; }


uint32_t RegisterFile::operator[](const int index) const { return registers[index]; }
