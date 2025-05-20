//
// Created by matthew on 5/19/25.
//

#include "heap.h"

#include "exceptions.h"


uint32_t HeapAllocator::nextFree(const uint32_t size) const {
    uint32_t ptr = HEAP_BASE;

    for (size_t i = 0; i < blockAddresses.size(); i++) {
        const uint32_t blkAddr = blockAddresses[i];
        const uint32_t blkSize = blockSizes[i];

        if (blkAddr - ptr >= size)
            return ptr;

        ptr = blkAddr + blkSize;
    }

    return ptr;
}


uint32_t HeapAllocator::allocate(const uint32_t size) {
    if (size == 0)
        throw MasmRuntimeError("Cannot allocate zero bytes");
    const uint32_t address = nextFree(size);
    if (address >= HEAP_BASE + HEAP_SIZE)
        throw MasmRuntimeError("Heap overflow");

    for (size_t i = 0; i < blockAddresses.size(); i++) {
        if (blockAddresses[i] > address) {
            blockAddresses.insert(blockAddresses.begin() + i, address);
            blockSizes.insert(blockSizes.begin() + i, size);
            return address;
        }
    }

    blockAddresses.push_back(address);
    blockSizes.push_back(size);
    return address;
}
