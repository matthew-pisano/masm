//
// Created by matthew on 4/16/25.
//

#ifndef MEMORY_H
#define MEMORY_H
#include <cstdint>
#include <string>


/**
 * Class representing valid, named sections of memory
 */
enum class MemSection { DATA, TEXT };


/**
 * Returns the memory section associated with a mnemonic
 * @param name The name of the memory section
 * @return The associated memory section
 */
MemSection nameToMemSection(const std::string& name);


/**
 * Returns the static offset of a named memory section
 * @param section The section of memory
 * @return The memory offset in bytes
 */
uint32_t memSectionOffset(MemSection section);

#endif // MEMORY_H
