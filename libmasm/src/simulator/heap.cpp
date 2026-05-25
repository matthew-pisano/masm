//
// Created by matthew on 5/19/25.
//

#include <masm/simulator/heap.hpp>

#include <masm/exceptions.hpp>
#include <numeric>


const uint32_t HEAP_BASE = memSectionOffset(MemSection::HEAP);


uint32_t HeapAllocator::nextFree(const uint32_t size) const {
    uint32_t ptr = HEAP_BASE;

    // Walk up through all allocated blocks to find a large enough free gap
    // If no gap exists, the heap grows up towards the stack
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
    // Grow heap pointer if more memory is needed
    if (address + size > heapPointer)
        heapPointer = address + size;

    // Insert new block sequentially before the block with the next greatest address
    for (size_t i = 0; i < blockAddresses.size(); i++) {
        if (blockAddresses[i] > address) {
            blockAddresses.insert(blockAddresses.begin() + static_cast<int32_t>(i), address);
            blockSizes.insert(blockSizes.begin() + static_cast<int32_t>(i), size);
            return address;
        }
    }

    // Push first block to the heap if it is empty
    blockAddresses.push_back(address);
    blockSizes.push_back(size);
    return address;
}


size_t HeapAllocator::allocated() const { return std::accumulate(blockSizes.begin(), blockSizes.end(), 0U); }

uint32_t HeapAllocator::top() const { return heapPointer; }
