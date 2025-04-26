//
// Created by matthew on 4/16/25.
//

#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>


/**
 * Class representing valid, named sections of memory
 */
enum class MemSection { DATA, TEXT };


/**
 * A type alias for an object containing memory allocations from the parser
 */
using MemLayout = std::map<MemSection, std::vector<std::byte>>;


class Memory {
    // Map that can store up to ~4G of memory
    std::unordered_map<uint32_t, std::byte> memory;

public:
    int32_t wordAt(uint32_t index);
    uint16_t halfAt(uint32_t index);
    uint8_t byteAt(uint32_t index);

    void wordTo(uint32_t index, int32_t value);
    void halfTo(uint32_t index, uint16_t value);
    void byteTo(uint32_t index, uint8_t value);

    void loadProgram(const MemLayout& layout);

    std::byte& operator[](uint32_t index);
};


/**
 * Returns the memory section associated with a mnemonic
 * @param name The name of the memory section
 * @return The associated memory section
 * @throw runtime_error When an invalid memory section is named
 */
MemSection nameToMemSection(const std::string& name);


/**
 * Returns the static offset of a named memory section
 * @param section The section of memory
 * @return The memory offset in bytes
 * @throw runtime_error When an invalid memory section is passed
 */
uint32_t memSectionOffset(MemSection section);

#endif // MEMORY_H
