//
// Created by matthew on 5/18/25.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>

#include <masm/exceptions.hpp>
#include <masm/simulator/syscalls.hpp>


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
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printInt(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print Float Syscall") {
    const std::string expected = "3.14";
    State state;
    state.cp1.setFloat(Coproc1Register::F12, std::stof(expected));
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printFloat(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print Double Syscall") {
    const std::string expected = "2.71828";
    State state;
    state.cp1.setDouble(Coproc1Register::F12, std::stod(expected));
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printDouble(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print String Syscall") {
    const std::string expected = "Hello, world!";
    State state;
    const uint32_t strAddr = writeStringToMem(state, expected);
    state.registers[Register::A0] = static_cast<int32_t>(strAddr);
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printString(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Read Int Syscall") {
    State state;

    SECTION("Valid Input") {
        const std::string input = "42\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readInt(state, streamHandle);

        REQUIRE(state.registers[Register::V0] == 42);
    }

    SECTION("Invalid Input") {
        const std::string input = "invalid\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readInt(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Invalid input: invalid"));
    }

    SECTION("Out of Range Input") {
        const std::string input = "99999999999999999999\n"; // Too large for int32_t
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readInt(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Input out of range: 99999999999999999999"));
    }

    SECTION("Edited Input") {
        const std::string input = "42\b4\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readInt(state, streamHandle);

        REQUIRE(state.registers[Register::V0] == 44);
    }
}


TEST_CASE("Test Read Float Syscall") {
    State state;

    SECTION("Valid Input") {
        const std::string input = "3.14\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readFloat(state, streamHandle);

        REQUIRE(state.cp1.getFloat(Coproc1Register::F0) == 3.14f);
    }

    SECTION("Invalid Input") {
        const std::string input = "invalid\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readFloat(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Invalid float input: invalid"));
    }

    SECTION("Out of Range Input") {
        const std::string input = "1e40\n"; // Too large for float
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readFloat(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Float input out of range: 1e40"));
    }
}


TEST_CASE("Test Read Double Syscall") {
    State state;

    SECTION("Valid Input") {
        const std::string input = "2.71828\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readDouble(state, streamHandle);

        REQUIRE(state.cp1.getDouble(Coproc1Register::F0) == 2.71828);
    }

    SECTION("Invalid Input") {
        const std::string input = "invalid\n";
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readDouble(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Invalid double input: invalid"));
    }

    SECTION("Out of Range Input") {
        const std::string input = "1e400\n"; // Too large for double
        std::istringstream istream(input);
        StreamHandle streamHandle{istream, std::cout};

        REQUIRE_THROWS_MATCHES(SystemHandle::readDouble(state, streamHandle), ExecExcept,
                               Catch::Matchers::Message("Double input out of range: 1e400"));
    }
}


TEST_CASE("Test Read String Syscall") {
    SECTION("Read Simple String") {
        const std::string input = "Hello, world!";
        std::stringstream istream(input);
        State state;
        const uint32_t strAddr = memSectionOffset(MemSection::DATA);
        state.registers[Register::A0] = static_cast<int32_t>(strAddr);
        state.registers[Register::A1] = static_cast<int32_t>(input.size());
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readString(state, streamHandle);

        for (size_t i = 0; i < input.size(); i++)
            REQUIRE(state.memory.byteAt(strAddr + i) == input[i]);
    }

    SECTION("Read Edited String") {
        const std::string input = "Hello, world\b\b\b\b\bthere!";
        const std::string expected = "Hello, there!";
        std::stringstream istream(input);
        State state;
        const uint32_t strAddr = memSectionOffset(MemSection::DATA);
        state.registers[Register::A0] = static_cast<int32_t>(strAddr);
        state.registers[Register::A1] = static_cast<int32_t>(expected.size());
        StreamHandle streamHandle{istream, std::cout};
        SystemHandle::readString(state, streamHandle);

        for (size_t i = 0; i < expected.size(); i++)
            REQUIRE(state.memory.byteAt(strAddr + i) == expected[i]);
    }
}


TEST_CASE("Test Heap Allocation Syscall") {
    State state;
    state.registers[Register::SP] = static_cast<int32_t>(memSectionOffset(MemSection::STACK));

    SECTION("Test Heap Allocation with Valid Size") {
        state.registers[Register::A0] = 100;
        SystemHandle::heapAlloc(state);
        uint32_t address = state.registers[Register::V0];
        REQUIRE(address == HEAP_BASE_ADDR);
        REQUIRE(state.heapAllocator.allocated() == HEAP_BLOCK_SIZE);
        REQUIRE(state.heapAllocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE);

        state.registers[Register::A0] = 50;
        SystemHandle::heapAlloc(state);
        address = state.registers[Register::V0];
        REQUIRE(address == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE);
        REQUIRE(state.heapAllocator.allocated() == HEAP_BLOCK_SIZE * 2);
        REQUIRE(state.heapAllocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 2);

        state.registers[Register::A0] = 200;
        SystemHandle::heapAlloc(state);
        address = state.registers[Register::V0];
        REQUIRE(address == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 2);
        REQUIRE(state.heapAllocator.allocated() == HEAP_BLOCK_SIZE * 3);
        REQUIRE(state.heapAllocator.top() == HEAP_BASE_ADDR + HEAP_BLOCK_SIZE * 3);
    }

    SECTION("Test Heap Allocation with Zero Size") {
        state.registers[Register::A0] = 0;

        REQUIRE_THROWS_MATCHES(SystemHandle::heapAlloc(state), ExecExcept,
                               Catch::Matchers::Message("Cannot allocate zero bytes"));
    }
}


TEST_CASE("Test Exit Syscall") {
    REQUIRE_THROWS_MATCHES(SystemHandle::exit(), ExecExit,
                           Catch::Matchers::Predicate<ExecExit>(
                                   [](const ExecExit& e) { return e.code() == 0 && std::string(e.what()).empty(); },
                                   "code == 0 and what() is empty"));
}


TEST_CASE("Test Print Char Syscall") {
    constexpr char expected = 'A';
    State state;
    state.registers[Register::A0] = static_cast<int32_t>(expected);
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printChar(state, streamHandle);

    REQUIRE(expected == ostream.str()[0]);
}


TEST_CASE("Test Read Char Syscall") {
    const std::string input = "A";
    std::stringstream istream(input);
    State state;
    StreamHandle streamHandle{istream, std::cout};
    SystemHandle::readChar(state, streamHandle);

    REQUIRE(state.registers[Register::V0] == static_cast<int32_t>(input[0]));
}


TEST_CASE("Test Exit Value Syscall") {
    constexpr int32_t exitCode = 42;
    State state;
    state.registers[Register::A0] = exitCode;

    REQUIRE_THROWS_MATCHES(SystemHandle::exitVal(state), ExecExit,
                           Catch::Matchers::Predicate<ExecExit>(
                                   [](const ExecExit& e) { return e.code() == 42 && std::string(e.what()).empty(); },
                                   "code == 42 and what() is empty"));
}


TEST_CASE("Test Time Syscall") {
    State state;
    SystemHandle::time(state);

    const auto duration = std::chrono::system_clock::now().time_since_epoch();
    const int64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    REQUIRE(state.registers[Register::A0] == static_cast<int32_t>(milliseconds & 0xFFFFFFFF));
    REQUIRE(state.registers[Register::A1] == static_cast<int32_t>(milliseconds >> 32 & 0xFFFFFFFF));
}


TEST_CASE("Test Sleep Syscall") {
    State state;

    SECTION("Test Sleep with Valid Duration") {
        state.registers[Register::A0] = 500;

        const auto startDuration = std::chrono::system_clock::now().time_since_epoch();
        const long startMillis = std::chrono::duration_cast<std::chrono::milliseconds>(startDuration).count();

        SystemHandle::sleep(state);

        const auto endDuration = std::chrono::system_clock::now().time_since_epoch();
        const long endMillis = std::chrono::duration_cast<std::chrono::milliseconds>(endDuration).count();

        // Check that the program continues after the sleep syscall
        // Use a small tolerance to avoid errors
        REQUIRE(std::abs(endMillis - startMillis - state.registers[Register::A0]) < 10);
    }


    SECTION("Test Sleep with Negative Duration") {
        state.registers[Register::A0] = -500;

        REQUIRE_THROWS_MATCHES(SystemHandle::sleep(state), ExecExcept,
                               Catch::Matchers::Message("Negative sleep time: -500"));
    }
}


TEST_CASE("Test Print Int Hex Syscall") {
    const std::string expected = "00000045";
    State state;
    state.registers[Register::A0] = 69;
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printIntHex(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print Int Bin Syscall") {
    const std::string expected = "00000000000000000000000001000101";
    State state;
    state.registers[Register::A0] = 69;
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printIntBin(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Print UInt Syscall") {
    const std::string expected = "4294966876";
    State state;
    state.registers[Register::A0] = -420;
    std::stringstream ostream;
    StreamHandle streamHandle{std::cin, ostream};
    SystemHandle::printUInt(state, streamHandle);

    REQUIRE(expected == ostream.str());
}


TEST_CASE("Test Random Syscall with Set Seed") {
    SystemHandle sysHandle;
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 42;

    RandomGenerator rng(state.registers[Register::A1]);
    const int32_t expected = static_cast<int32_t>(rng.getRandomInt());

    SystemHandle::setRandSeed(state, sysHandle.getRngMap());
    SystemHandle::randInt(state, sysHandle.getRngMap());

    REQUIRE(state.registers[Register::A0] == expected);
}


TEST_CASE("Test Random Range Syscall with Set Seed") {
    SystemHandle sysHandle;
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 42; // Serves as both seed and max range

    RandomGenerator rng(state.registers[Register::A1]);
    const int32_t expected = static_cast<int32_t>(rng.getRandomInt(state.registers[Register::A1]));

    SystemHandle::setRandSeed(state, sysHandle.getRngMap());
    SystemHandle::randIntRange(state, sysHandle.getRngMap());

    REQUIRE(state.registers[Register::A0] == expected);
}


TEST_CASE("Test Random Float Syscall") {
    SystemHandle sysHandle;
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 1444;

    RandomGenerator rng(state.registers[Register::A1]);
    const float expected = rng.getRandomFloat();

    SystemHandle::setRandSeed(state, sysHandle.getRngMap());
    SystemHandle::randFloat(state, sysHandle.getRngMap());

    REQUIRE(state.cp1.getFloat(Coproc1Register::F0) == expected);
}


TEST_CASE("Test Random Double Syscall") {
    SystemHandle sysHandle;
    State state;
    state.registers[Register::A0] = 1;
    state.registers[Register::A1] = 1444;

    RandomGenerator rng(state.registers[Register::A1]);
    const double expected = rng.getRandomDouble();

    SystemHandle::setRandSeed(state, sysHandle.getRngMap());
    SystemHandle::randDouble(state, sysHandle.getRngMap());

    REQUIRE(state.cp1.getDouble(Coproc1Register::F0) == expected);
}
