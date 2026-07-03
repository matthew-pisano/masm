//
// Created by matthew on 5/19/25.
//

#ifndef HEAP_H
#define HEAP_H
#include <cstdint>

#include <masm/assembler/memory.hpp>


/// The minimum allocatable size
constexpr uint32_t HEAP_BLOCK_SIZE = 256;

/// The base address for the heap
const uint32_t HEAP_BASE_ADDR = memSectionOffset(MemSection::HEAP);


/**
 * Class representing a simple heap allocator
 */
class HeapAllocator {
    /**
     * The stating addresses of all allocated blocks in the heap
     */
    std::vector<uint32_t> blockAddresses;

    /**
     * The sizes of all blocks in the heap
     */
    std::vector<uint32_t> blockSizes;

    /**
     * A pointer to the current top of heap memory
     */
    uint32_t heapPointer = HEAP_BASE_ADDR;

    /**
     * Finds the first unallocated space in the heap that can accommodate a block of the given size
     * @param size The size of the block to allocate
     * @return The first unallocated area that can fit the requested block
     */
    [[nodiscard]] uint32_t nextFree(uint32_t size) const;

public:
    /**
     * Allocates a block of memory of at least the given size in the heap
     * @param size The size of the block to allocate
     * @return The address of the allocated block
     * @throw runtime_error if the allocation fails
     */
    uint32_t allocate(uint32_t size);

    /**
     * Deallocates the block of memory at the given address
     *
     * Fails if the given address is not thr base address of a block
     *
     * @param address The address of the block to free
     */
    void deallocate(uint32_t address);

    /**
     * Gets the total number of bytes allocated on the heap
     */
    [[nodiscard]] size_t allocated() const;

    /**
     * Gets the pointer to the top of heap memory
     */
    [[nodiscard]] uint32_t top() const;
};

#endif // HEAP_H
