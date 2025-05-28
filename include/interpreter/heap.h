//
// Created by matthew on 5/19/25.
//

#ifndef HEAP_H
#define HEAP_H
#include <cstdint>

#include "memory.h"

const uint32_t HEAP_BASE = memSectionOffset(MemSection::HEAP);
constexpr uint32_t HEAP_SIZE = 0xfd00000; // 253 MiB

/**
 * Class representing a simple heap allocator
 */
class HeapAllocator {
    std::vector<uint32_t> blockAddresses;
    std::vector<uint32_t> blockSizes;

    [[nodiscard]] uint32_t nextFree(uint32_t size) const;

public:
    /**
     * Allocates a block of memory of the given size
     * @param size The size of the block to allocate
     * @return The address of the allocated block
     * @throw runtime_error if the allocation fails
     */
    uint32_t allocate(uint32_t size);
};

#endif // HEAP_H
