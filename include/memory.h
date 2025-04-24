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
using MemLayout = std::map<MemSection, std::vector<uint8_t>>;


class Memory {
    // Map that can store up to ~4G of memory
    std::unordered_map<uint32_t, int8_t> memory;

public:
    void loadProgram(const MemLayout& layout);

    uint8_t operator[](int index) const;
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
