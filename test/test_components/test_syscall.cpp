//
// Created by matthew on 5/18/25.
//

#include <catch2/catch_test_macros.hpp>

#include "interpreter/interpreter.h"
#include "exceptions.h"
#include "interpreter/syscalls.h"


uint32_t writeStringToMem(State& state, const std::string& string) {
    const uint32_t dataAddr = memSectionOffset(MemSection::DATA);
    for (size_t i = 0; i < string.size(); i++)
        state.memory.byteTo(dataAddr + i, string[i]);
    state.memory.byteTo(dataAddr + string.size(), '\0');
    return dataAddr;
}


TEST_CASE("Test Print Int Syscall") {
    const std::string expected = "69";
    State state;
    state.registers[Register::A0] = std::stoi(expected);
    std::stringstream ostream;
    printIntSyscall(state, ostream);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print String Syscall") {
    const std::string expected = "Hello, world!";
    State state;
    const uint32_t strAddr = writeStringToMem(state, expected);
    state.registers[Register::A0] = static_cast<int32_t>(strAddr);
    std::stringstream ostream;
    printStringSyscall(state, ostream);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Read Int Syscall") {
    const std::string input = "42";
    std::stringstream istream(input);
    State state;
    readIntSyscall(state, istream);

    REQUIRE(state.registers[Register::V0] == 42);
}


TEST_CASE("Test Read String Syscall") {
    const std::string input = "Hello, world!";
    std::stringstream istream(input);
    State state;
    const uint32_t strAddr = memSectionOffset(MemSection::DATA);
    state.registers[Register::A0] = static_cast<int32_t>(strAddr);
    state.registers[Register::A1] = static_cast<int32_t>(input.size());
    readStringSyscall(state, istream);

    for (size_t i = 0; i < input.size(); i++)
        REQUIRE(state.memory.byteAt(strAddr + i) == input[i]);
}


TEST_CASE("Test Heap Allocation Syscall") {
    State state;

    state.registers[Register::A0] = 100;
    heapAllocSyscall(state);
    uint32_t address = state.registers[Register::V0];
    REQUIRE(address == HEAP_BASE);

    state.registers[Register::A0] = 50;
    heapAllocSyscall(state);
    address = state.registers[Register::V0];
    REQUIRE(address == HEAP_BASE + 100);

    state.registers[Register::A0] = 200;
    heapAllocSyscall(state);
    address = state.registers[Register::V0];
    REQUIRE(address == HEAP_BASE + 150);
}


TEST_CASE("Test Exit Syscall") { REQUIRE_THROWS_AS(exitSyscall(), ExecExit); }


TEST_CASE("Test Print Char Syscall") {
    constexpr char expected = 'A';
    State state;
    state.registers[Register::A0] = static_cast<int32_t>(expected);
    std::stringstream ostream;
    printCharSyscall(state, ostream);

    REQUIRE(expected == ostream.str()[0]);
}


TEST_CASE("Test Read Char Syscall") {
    const std::string input = "A";
    std::stringstream istream(input);
    State state;
    readCharSyscall(state, istream);

    REQUIRE(state.registers[Register::V0] == static_cast<int32_t>(input[0]));
}


TEST_CASE("Test Exit Value Syscall") {
    constexpr int32_t exitCode = 42;
    State state;
    state.registers[Register::A0] = exitCode;

    REQUIRE_THROWS_AS(exitValSyscall(state), ExecExit);
    try {
        exitValSyscall(state);
    } catch (const ExecExit& e) {
        REQUIRE(e.code() == exitCode);
    }
}


TEST_CASE("Test Time Syscall") {
    State state;
    timeSyscall(state);

    const auto duration = std::chrono::system_clock::now().time_since_epoch();
    const long milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    REQUIRE(state.registers[Register::A0] == static_cast<int32_t>(milliseconds & 0xFFFFFFFF));
    REQUIRE(state.registers[Register::A1] == static_cast<int32_t>(milliseconds >> 32 & 0xFFFFFFFF));
}


TEST_CASE("Test Sleep Syscall") {
    State state;
    state.registers[Register::A0] = 500;

    const auto startDuration = std::chrono::system_clock::now().time_since_epoch();
    const long startMillis =
            std::chrono::duration_cast<std::chrono::milliseconds>(startDuration).count();

    sleepSyscall(state);

    const auto endDuration = std::chrono::system_clock::now().time_since_epoch();
    const long endMillis =
            std::chrono::duration_cast<std::chrono::milliseconds>(endDuration).count();

    // Check that the program continues after the sleep syscall
    // Use a small tolerance to avoid errors
    REQUIRE(std::abs(endMillis - startMillis - state.registers[Register::A0]) < 10);
}


TEST_CASE("Test Negative Sleep Syscall") {
    State state;
    state.registers[Register::A0] = -500;

    REQUIRE_THROWS_AS(sleepSyscall(state), MasmRuntimeError);
}


TEST_CASE("Test Print Int Hex Syscall") {
    const std::string expected = "00000045";
    State state;
    state.registers[Register::A0] = 69;
    std::stringstream ostream;
    printIntHexSyscall(state, ostream);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print Int Bin Syscall") {
    const std::string expected = "00000000000000000000000001000101";
    State state;
    state.registers[Register::A0] = 69;
    std::stringstream ostream;
    printIntBinSyscall(state, ostream);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print UInt Syscall") {
    const std::string expected = "4294966876";
    State state;
    state.registers[Register::A0] = -420;
    std::stringstream ostream;
    printUIntSyscall(state, ostream);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Random Syscall with Set Seed") {
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 42;

    RandomGenerator rng(state.registers[Register::A1]);
    const int32_t expected = static_cast<int32_t>(rng.getRandomInt());

    setRandSeedSyscall(state);
    randIntSyscall(state);

    REQUIRE(state.registers[Register::A0] == expected);
}


TEST_CASE("Test Random Range Syscall with Set Seed") {
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 42; // Serves as both seed and max range

    RandomGenerator rng(state.registers[Register::A1]);
    const int32_t expected = static_cast<int32_t>(rng.getRandomInt(state.registers[Register::A1]));

    setRandSeedSyscall(state);
    randIntRangeSyscall(state);

    REQUIRE(state.registers[Register::A0] == expected);
}
