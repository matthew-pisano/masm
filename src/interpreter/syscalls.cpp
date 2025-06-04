//
// Created by matthew on 4/26/25.
//

#include "interpreter/syscalls.h"

#include <chrono>
#include <iostream>
#include <ostream>
#include <unistd.h>

#include "exceptions.h"
#include "io/consoleio.h"


std::map<size_t, RandomGenerator> rngMap = {};


/**
 * Reads (blocking) a character from the input stream, handling both console and file streams.
 * @param istream The input stream to read from, can be std::cin or any other input stream
 * @return The character read from the input stream
 */
char getStreamChar(std::istream& istream) {
    char c;
    if (&istream != &std::cin)
        istream.get(c);
    else {
        while (!consoleHasChar())
            usleep(1000); // Sleep for 1 ms
        c = consoleGetChar();
    }
    return c;
}


void requiresSyscallMode(const IOMode ioMode, const std::string& syscallName) {
    if (ioMode != IOMode::SYSCALL)
        throw ExecExcept(syscallName + " syscall not supported in MMIO mode",
                         EXCEPT_CODE::SYSCALL_EXCEPTION);
}


void execSyscall(const IOMode ioMode, State& state, std::istream& istream, std::ostream& ostream) {
    int32_t syscallCode = state.registers[Register::V0];

    switch (static_cast<Syscall>(syscallCode)) {
        case Syscall::PRINT_INT:
            requiresSyscallMode(ioMode, "PRINT_INT");
            printIntSyscall(state, ostream);
            break;
        case Syscall::PRINT_STRING:
            requiresSyscallMode(ioMode, "PRINT_STRING");
            printStringSyscall(state, ostream);
            break;
        case Syscall::READ_INT:
            requiresSyscallMode(ioMode, "READ_INT");
            readIntSyscall(state, istream);
            break;
        case Syscall::READ_STRING:
            requiresSyscallMode(ioMode, "READ_STRING");
            readStringSyscall(state, istream);
            break;
        case Syscall::HEAP_ALLOC:
            heapAllocSyscall(state);
            break;
        case Syscall::EXIT:
            exitSyscall();
            break;
        case Syscall::PRINT_CHAR:
            requiresSyscallMode(ioMode, "PRINT_CHAR");
            printCharSyscall(state, ostream);
            break;
        case Syscall::READ_CHAR:
            requiresSyscallMode(ioMode, "READ_CHAR");
            readCharSyscall(state, istream);
            break;
        case Syscall::EXIT_VAL:
            exitValSyscall(state);
            break;
        case Syscall::TIME:
            timeSyscall(state);
            break;
        case Syscall::SLEEP:
            sleepSyscall(state);
            break;
        case Syscall::PRINT_INT_HEX:
            requiresSyscallMode(ioMode, "PRINT_INT_HEX");
            printIntHexSyscall(state, ostream);
            break;
        case Syscall::PRINT_INT_BIN:
            requiresSyscallMode(ioMode, "PRINT_INT_BIN");
            printIntBinSyscall(state, ostream);
            break;
        case Syscall::PRINT_UINT:
            requiresSyscallMode(ioMode, "PRINT_UINT");
            printUIntSyscall(state, ostream);
            break;
        case Syscall::SET_SEED:
            setRandSeedSyscall(state);
            break;
        case Syscall::RAND_INT:
            randIntSyscall(state);
            break;
        case Syscall::RAND_INT_RANGE:
            randIntRangeSyscall(state);
            break;
        default:
            throw std::runtime_error("Unknown syscall " + std::to_string(syscallCode));
    }
}


void printIntSyscall(const State& state, std::ostream& ostream) {
    const int32_t value = state.registers[Register::A0];
    ostream << value;
}


void printStringSyscall(State& state, std::ostream& ostream) {
    int32_t address = state.registers[Register::A0];
    while (true) {
        const unsigned char c = state.memory.byteAt(address);
        if (c == '\0')
            break;
        ostream << c;
        address++;
    }
    ostream.flush();
}


void readIntSyscall(State& state, std::istream& istream) {
    std::string input;
    while (true) {
        const char c = getStreamChar(istream);
        if (c == '\n')
            break;
        input += c;
    }
    try {
        state.registers[Register::V0] = std::stoi(input);
    } catch (const std::invalid_argument&) {
        throw ExecExcept("Invalid input: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    } catch (const std::out_of_range&) {
        throw ExecExcept("Input out of range: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    }
}


void readStringSyscall(State& state, std::istream& istream) {
    const int32_t address = state.registers[Register::A0];
    const int32_t length = state.registers[Register::A1];
    int currLen = 0;
    while (currLen < length) {
        const char c = getStreamChar(istream);
        if (c == '\n')
            break;
        state.memory.byteTo(address + currLen, c);
        currLen++;
    }
}


void heapAllocSyscall(State& state) {
    const int32_t size = state.registers[Register::A0];
    const int32_t ptr = static_cast<int32_t>(state.heapAllocator.allocate(size));
    state.registers[Register::V0] = ptr;
}


void exitSyscall() { throw ExecExit("Program exited with code " + std::to_string(0), 0); }


void printCharSyscall(const State& state, std::ostream& ostream) {
    const char c = static_cast<char>(state.registers[Register::A0]);
    ostream << c;
}


void readCharSyscall(State& state, std::istream& istream) {
    const char c = getStreamChar(istream);
    state.registers[Register::V0] = 0xFF & static_cast<int32_t>(c);
}


void exitValSyscall(const State& state) {
    const int32_t exitCode = state.registers[Register::A0];
    throw ExecExit("Program exited with code " + std::to_string(exitCode), exitCode);
}


void timeSyscall(State& state) {
    // Get the current time in milliseconds since the epoch
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const long milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Store high bits in $a1 and low bits in $a0
    state.registers[Register::A0] = static_cast<int32_t>(milliseconds & 0xFFFFFFFF);
    state.registers[Register::A1] = static_cast<int32_t>(milliseconds >> 32 & 0xFFFFFFFF);
}


void sleepSyscall(State& state) {
    const int32_t milliseconds = state.registers[Register::A0];
    if (milliseconds < 0)
        throw ExecExcept("Negative sleep time: " + std::to_string(milliseconds),
                         EXCEPT_CODE::SYSCALL_EXCEPTION);

    usleep(milliseconds * 1000);
}


void printIntHexSyscall(const State& state, std::ostream& ostream) {
    const int32_t value = state.registers[Register::A0];
    ostream << std::hex << std::setw(8) << std::setfill('0') << value;
    ostream.flush();
}

void printIntBinSyscall(const State& state, std::ostream& ostream) {
    const int32_t value = state.registers[Register::A0];
    for (int i = 31; i >= 0; --i)
        ostream << (value >> i & 1);
    ostream.flush();
}

void printUIntSyscall(const State& state, std::ostream& ostream) {
    const uint32_t value = state.registers[Register::A0];
    ostream << value;
    ostream.flush();
}

void setRandSeedSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t seed = state.registers[Register::A1];
    rngMap[id] = RandomGenerator(seed);
}

void randIntSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt());
}

void randIntRangeSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t max = state.registers[Register::A1];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt(max));
}
