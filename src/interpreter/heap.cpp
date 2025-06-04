//
// Created by matthew on 5/19/25.
//

#include "interpreter/heap.h"

#include "exceptions.h"


uint32_t HeapAllocator::nextFree(const uint32_t size) const {
    uint32_t ptr = HEAP_BASE;

    for (size_t i = 0; i < blockAddresses.size(); i++) {
        const uint32_t blkAddr = blockAddresses[i];
        const uint32_t blkSize = blockSizes[i];

        // Check if there is enough space between the pointer and the next block
        if (blkAddr - ptr >= size)
            return ptr;

        // Move pointer immediately after this block
        ptr = blkAddr + blkSize;
    }

    return ptr;
}


uint32_t HeapAllocator::allocate(const uint32_t size) {
    if (size == 0)
        throw ExecExcept("Cannot allocate zero bytes", EXCEPT_CODE::SYSCALL_EXCEPTION);

    const uint32_t address = nextFree(size);
    if (address >= HEAP_BASE + HEAP_SIZE)
        throw ExecExcept("Heap overflow", EXCEPT_CODE::SYSCALL_EXCEPTION);

    // Insert new block sequentially before the block with the next greatest address
    for (size_t i = 0; i < blockAddresses.size(); i++) {
        if (blockAddresses[i] > address) {
            blockAddresses.insert(blockAddresses.begin() + i, address);
            blockSizes.insert(blockSizes.begin() + i, size);
            return address;
        }
    }

    // Push first block to the heap if it is empty
    blockAddresses.push_back(address);
    blockSizes.push_back(size);
    return address;
}
