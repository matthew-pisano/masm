//
// Created by matthew on 4/16/25.
//

#ifndef MEMORY_H
#define MEMORY_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>


/**
 * The upper bound for the text segment address space
 */
constexpr int32_t TEXT_SEC_END = 0x10000000;


/**
 * Class representing valid, named sections of memory
 */
enum class MemSection { DATA, HEAP, GLOBAL, STACK, TEXT, KTEXT, KDATA, MMIO };


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


/**
 * Struct representing the location of a source line in the program
 */
struct SourceLocator {
    std::string filename;
    size_t lineno;
};


/**
 * Struct representing the memory layout of a program along with the locations in the source files
 */
struct MemLayout {
    /**
     * The memory sections and their associated data
     */
    std::map<MemSection, std::vector<std::byte>> data;

    /**
     * The source line locators associated with each byte of memory
     */
    std::map<MemSection, std::vector<std::shared_ptr<SourceLocator>>> debugInfo;
};


/**
 * Class representing main memory
 */
class Memory {
    /**
     * The main memory map between indices and bytes
     */
    std::unordered_map<uint32_t, std::byte> memory;

    /**
     * Whether to use a little endian memory layout
     */
    bool useLittleEndian;

    /**
     * Gets the byte at the given address or zero if not allocated (without triggering side
     * effects). Only to be used for privileged reads
     * @param index The address to read from
     * @return The byte stored at the given address or zero if not allocated
     */
    std::byte _sysByteAt(uint32_t index) const;

    /**
     * Processes any side effects from reading from an address, such as updating the MMIO ready bit
     * @param index The address to read from
     */
    void readSideEffect(uint32_t index);

    /**
     * Processes any side effects from writing to an address, such as updating the MMIO ready bit
     * @param index The address to write to
     */
    void writeSideEffect(uint32_t index);

public:
    /**
     * Constructor for the Memory class
     * @param useLittleEndian Whether to use little endian memory layout
     */
    explicit Memory(const bool useLittleEndian = false) : useLittleEndian(useLittleEndian) {}

    /**
     * Gets the word stored at the given word-aligned memory address (without triggering side
     * effects).  Only to be used for privileged reads
     * @param index The word-aligned address to read from
     * @return The word stored at the given address
     */
    int32_t _sysWordAt(uint32_t index) const;

    /**
     * Sets the word at the given word-aligned memory address (without triggering side effects).
     * Only to be used for privileged writes
     * @param index The word-aligned address to write to
     * @param value The word to write
     */
    void _sysWordTo(uint32_t index, int32_t value);

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
    void halfTo(uint32_t index, int16_t value);

    /**
     * Sets the byte at the given byte-aligned memory address
     * @param index The byte-aligned address to write to
     * @param value The byte to write
     */
    void byteTo(uint32_t index, int8_t value);

    /**
     * Checks if the given index is initialized and valid
     * @param index The index to check
     * @return True if the index is valid, false otherwise
     */
    bool isValid(uint32_t index) const;

    std::byte operator[](uint32_t index) const;
    std::byte& operator[](uint32_t index);
};


/**
 * Checks if a memory section is executable
 * @param section The section of memory to check
 * @return True if the section is executable, false otherwise
 */
bool isSectionExecutable(MemSection section);

#endif // MEMORY_H
