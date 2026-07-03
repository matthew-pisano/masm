//
// Created by matthew on 5/19/25.
//

#include <masm/simulator/heap.hpp>

#include <masm/exceptions.hpp>
#include <numeric>
#include <sstream>


uint32_t HeapAllocator::nextFree(const uint32_t size) const {
    uint32_t ptr = HEAP_BASE_ADDR;

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


uint32_t HeapAllocator::allocate(uint32_t size) {
    if (size == 0)
        throw ExecExcept("Cannot allocate zero bytes", EXCEPT_CODE::SYSCALL_EXCEPTION);

    // Round up to the nearest block size
    size = (size - 1) / HEAP_BLOCK_SIZE * HEAP_BLOCK_SIZE + HEAP_BLOCK_SIZE;

    // Get the next available base address
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


void HeapAllocator::deallocate(const uint32_t address) {
    const auto addrIt = std::ranges::find(blockAddresses, address);

    if (addrIt == blockAddresses.end()) {
        std::stringstream ss;
        ss << "Invalid free of address: 0x" << std::hex << address;
        throw ExecExcept(ss.str(), EXCEPT_CODE::SYSCALL_EXCEPTION);
    }

    blockAddresses.erase(addrIt);
    blockSizes.erase(blockSizes.begin() + (addrIt - blockAddresses.begin()));

    // Reset heap pointer if there are no blocks allocated
    if (blockAddresses.empty()) {
        heapPointer = HEAP_BASE_ADDR;
        return;
    }

    // Set the current heap pointer to the end of the last block
    const size_t lastBlockIdx = blockAddresses.size() - 1;
    const uint32_t currentHeapTop = blockAddresses.at(lastBlockIdx) + blockSizes.at(lastBlockIdx);
    if (currentHeapTop < heapPointer)
        heapPointer = currentHeapTop;
}


size_t HeapAllocator::allocated() const { return std::accumulate(blockSizes.begin(), blockSizes.end(), 0U); }

uint32_t HeapAllocator::top() const { return heapPointer; }
