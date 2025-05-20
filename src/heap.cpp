//
// Created by matthew on 5/19/25.
//

#include "heap.h"

#include "exceptions.h"


uint32_t HeapAllocator::allocate(const uint32_t size) {
    if (size == 0)
        throw MasmRuntimeError("Cannot allocate zero bytes");
    if (heapHead + size >= HEAP_BASE + HEAP_SIZE)
        throw MasmRuntimeError("Heap overflow");

    const uint32_t address = heapHead;
    heapHead += size;
    return address;
}
