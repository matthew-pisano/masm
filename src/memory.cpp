//
// Created by matthew on 4/16/25.
//

#include "memory.h"

#include <stdexcept>


void Memory::loadProgram(const MemLayout& layout) {
    for (const std::pair<MemSection, std::vector<std::byte>> pair : layout)
        for (size_t i = 0; i < pair.second.size(); i++)
            memory[memSectionOffset(pair.first) + i] = pair.second[i];
}


std::byte& Memory::operator[](const uint32_t index) { return memory[index]; }


MemSection nameToMemSection(const std::string& name) {
    if (name == "text")
        return MemSection::TEXT;
    if (name == "data")
        return MemSection::DATA;
    throw std::runtime_error("Unknown memory directive " + name);
}


uint32_t memSectionOffset(const MemSection section) {
    switch (section) {
        case MemSection::DATA:
            return 0x10010000;
        case MemSection::TEXT:
            return 0x00400000;
    }

    throw std::runtime_error("Unknown memory section " + std::to_string(static_cast<int>(section)));
}
