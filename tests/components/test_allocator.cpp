//
// Created by matthew on 7/3/26.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <masm/simulator/heap.hpp>


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
