//
// Created by matthew on 4/16/25.
//

#include "memory.h"

#include <stdexcept>


std::byte Memory::memAt(const uint32_t index) const {
    if (!memory.contains(index))
        return static_cast<std::byte>(0);
    return memory.at(index);
}


void Memory::readSideEffect(const uint32_t index) {
    const uint32_t input_ready = memSectionOffset(MemSection::MMIO);
    const uint32_t input_data = input_ready + 4;

    if (index == input_data)
        // Reset ready bit
        wordTo(input_ready, 0);
}


int32_t Memory::wordAt(const uint32_t index) {
    if (index % 4 != 0)
        throw std::runtime_error("Invalid word access at " + std::to_string(index));

    readSideEffect(index);
    return static_cast<int32_t>(memAt(index)) << 24 | static_cast<int32_t>(memAt(index + 1)) << 16 |
           static_cast<int32_t>(memAt(index + 2)) << 8 | static_cast<int32_t>(memAt(index + 3));
}


uint16_t Memory::halfAt(const uint32_t index) {
    if (index % 2 != 0)
        throw std::runtime_error("Invalid half-word access at " + std::to_string(index));

    readSideEffect(index);
    return static_cast<uint16_t>(memAt(index)) << 8 | static_cast<uint16_t>(memAt(index + 1));
}


uint8_t Memory::byteAt(const uint32_t index) {
    readSideEffect(index);
    return static_cast<uint8_t>(memAt(index));
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


std::byte Memory::operator[](const uint32_t index) const { return memory.at(index); }
std::byte& Memory::operator[](const uint32_t index) { return memory.at(index); }


MemSection nameToMemSection(const std::string& name) {
    if (name == "text")
        return MemSection::TEXT;
    if (name == "data")
        return MemSection::DATA;
    if (name == "heap")
        return MemSection::HEAP;
    if (name == "ktext")
        return MemSection::KTEXT;
    if (name == "kdata")
        return MemSection::KDATA;
    if (name == "mmio")
        return MemSection::MMIO;
    // Should never be reached
    throw std::runtime_error("Unknown memory directive " + name);
}


uint32_t memSectionOffset(const MemSection section) {
    switch (section) {
        case MemSection::DATA:
            return 0x10010000;
        case MemSection::HEAP:
            return 0x10040000;
        case MemSection::TEXT:
            return 0x00400000;
        case MemSection::KDATA:
            return 0x90000000;
        case MemSection::KTEXT:
            return 0x80000000;
        case MemSection::MMIO:
            return 0xffff0000;
    }
    // Should never be reached
    throw std::runtime_error("Unknown memory section " + std::to_string(static_cast<int>(section)));
}
