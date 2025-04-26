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


/**
 * Class representing main memory
 */
class Memory {
    /**
     * The main memory map between indices and bytes
     */
    std::unordered_map<uint32_t, std::byte> memory;

public:
    /**
     * Gets the word stored at the given word-aligned memory address
     * @param index The word-aligned address to read from
     * @return The word stored at the given address
     */
    int32_t wordAt(uint32_t index);

    /**
     * Gets the halfword stored at the given halfword-aligned memory address
     * @param index The halfword-aligned address to read from
     * @return The halfword stored at the given address
     */
    uint16_t halfAt(uint32_t index);

    /**
     * Gets the byte stored at the given byte-aligned memory address
     * @param index The byte-aligned address to read from
     * @return The byte stored at the given address
     */
    uint8_t byteAt(uint32_t index);

    /**
     * Sets the word at the given word-aligned memory address
     * @param index The word-aligned address to write to
     * @param value The word to write
     */
    void wordTo(uint32_t index, int32_t value);

    /**
     * Sets the halfword at the given halfword-aligned memory address
     * @param index The halfword-aligned address to write to
     * @param value The halfword to write
     */
    void halfTo(uint32_t index, uint16_t value);

    /**
     * Sets the byte at the given byte-aligned memory address
     * @param index The byte-aligned address to write to
     * @param value The byte to write
     */
    void byteTo(uint32_t index, uint8_t value);

    /**
     * Loads a program and initial static data into memory
     * @param layout The memory layout to load
     */
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
