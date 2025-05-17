//
// Created by matthew on 4/16/25.
//

#include "memory.h"

#include <stdexcept>


int32_t Memory::wordAt(const uint32_t index) const {
    if (index % 4 != 0)
        throw std::runtime_error("Invalid word access at " + std::to_string(index));

    return static_cast<int32_t>(memory.at(index)) << 24 |
           static_cast<int32_t>(memory.at(index + 1)) << 16 |
           static_cast<int32_t>(memory.at(index + 2)) << 8 |
           static_cast<int32_t>(memory.at(index + 3));
}


uint16_t Memory::halfAt(const uint32_t index) const {
    if (index % 2 != 0)
        throw std::runtime_error("Invalid half-word access at " + std::to_string(index));

    return static_cast<uint16_t>(memory.at(index)) << 8 |
           static_cast<uint16_t>(memory.at(index + 1));
}


uint8_t Memory::byteAt(const uint32_t index) const {
    return static_cast<uint8_t>(memory.at(index));
}


void Memory::wordTo(const uint32_t index, const int32_t value) {
    if (index % 4 != 0)
        throw std::runtime_error("Invalid word access at " + std::to_string(index));

    memory[index] = static_cast<std::byte>(value >> 24);
    memory[index + 1] = static_cast<std::byte>(value >> 16);
    memory[index + 2] = static_cast<std::byte>(value >> 8);
    memory[index + 3] = static_cast<std::byte>(value);
}


void Memory::halfTo(const uint32_t index, const int16_t value) {
    if (index % 2 != 0)
        throw std::runtime_error("Invalid half-word access at " + std::to_string(index));

    memory[index] = static_cast<std::byte>(value >> 8);
    memory[index + 1] = static_cast<std::byte>(value);
}


void Memory::byteTo(const uint32_t index, const int8_t value) {
    memory[index] = static_cast<std::byte>(value);
}


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
    // Should never be reached
    throw std::runtime_error("Unknown memory directive " + name);
}


uint32_t memSectionOffset(const MemSection section) {
    switch (section) {
        case MemSection::DATA:
            return 0x10010000;
        case MemSection::TEXT:
            return 0x00400000;
    }
    // Should never be reached
    throw std::runtime_error("Unknown memory section " + std::to_string(static_cast<int>(section)));
}
