//
// Created by matthew on 7/3/26.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <masm/simulator/heap.hpp>


TEST_CASE("Test Init Allocator") {
    const HeapAllocator allocator;
    REQUIRE(allocator.allocated() == 0);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR);
}


TEST_CASE("Test Zero Allocation") {
    HeapAllocator allocator;
    REQUIRE_THROWS_MATCHES(allocator.allocate(0), std::runtime_error,
                           Catch::Matchers::Message("Cannot allocate zero bytes"));
}


TEST_CASE("Test Sub Block Allocation") {
    HeapAllocator allocator;
    const uint32_t addr = allocator.allocate(1);
    REQUIRE(addr == HEAP_BASE_ADDR);
    REQUIRE(allocator.allocated() == HEAP_BLOCK_SIZE);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE);
}


TEST_CASE("Test Over Block Allocation") {
    HeapAllocator allocator;
    const uint32_t addr = allocator.allocate(HEAP_BLOCK_SIZE + 1);
    REQUIRE(addr == HEAP_BASE_ADDR);
    REQUIRE(allocator.allocated() == HEAP_BLOCK_SIZE * 2);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 2);
}


TEST_CASE("Test Two Allocations") {
    HeapAllocator allocator;
    const uint32_t addr1 = allocator.allocate(HEAP_BLOCK_SIZE + 1);
    const uint32_t addr2 = allocator.allocate(1);

    REQUIRE(addr1 == HEAP_BASE_ADDR);
    REQUIRE(addr2 == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 2);
    REQUIRE(allocator.allocated() == HEAP_BLOCK_SIZE * 3);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 3);
}


TEST_CASE("Test Bad Deallocation") {
    HeapAllocator allocator;
    // Deallocate with nothing allocated
    REQUIRE_THROWS_MATCHES(allocator.deallocate(0), std::runtime_error,
                           Catch::Matchers::Message("Invalid free of address: 0x0"));

    // Deallocate with one allocation
    (void) allocator.allocate(1);
    REQUIRE_THROWS_MATCHES(allocator.deallocate(0), std::runtime_error,
                           Catch::Matchers::Message("Invalid free of address: 0x0"));
}


TEST_CASE("Test Allocate Deallocate") {
    HeapAllocator allocator;
    const uint32_t addr = allocator.allocate(HEAP_BLOCK_SIZE);
    allocator.deallocate(addr);

    REQUIRE(allocator.allocated() == 0);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR);
}


TEST_CASE("Test Two Allocate Deallocate") {
    HeapAllocator allocator;
    (void) allocator.allocate(HEAP_BLOCK_SIZE);
    const uint32_t addr2 = allocator.allocate(HEAP_BLOCK_SIZE);
    allocator.deallocate(addr2);

    REQUIRE(allocator.allocated() == HEAP_BLOCK_SIZE);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE);
}

TEST_CASE("Test Allocate Replace") {
    HeapAllocator allocator;
    const uint32_t addr1 = allocator.allocate(HEAP_BLOCK_SIZE);
    (void) allocator.allocate(HEAP_BLOCK_SIZE);
    allocator.deallocate(addr1);
    const uint32_t addr2 = allocator.allocate(HEAP_BLOCK_SIZE);

    REQUIRE(addr2 == HEAP_BASE_ADDR);
    REQUIRE(allocator.allocated() == HEAP_BLOCK_SIZE * 2);
    REQUIRE(allocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 2);
}


TEST_CASE("Test Heap Overflow") {
    HeapAllocator allocator;
    const uint32_t maxAllocation = HEAP_MAX_ADDR - HEAP_BASE_ADDR - HEAP_BLOCK_SIZE;
    REQUIRE_THROWS_MATCHES(allocator.allocate(maxAllocation + HEAP_BLOCK_SIZE), std::runtime_error,
                           Catch::Matchers::Message("Heap Overflow"));
}
