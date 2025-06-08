//
// Created by matthew on 4/26/25.
//

#include "interpreter/syscalls.h"

#include <chrono>
#include <iostream>
#include <ostream>
#include <unistd.h>

#include "exceptions.h"
#include "interpreter/cpu.h"
#include "io/consoleio.h"


void SystemHandle::requiresSyscallMode(const IOMode ioMode, const std::string& syscallName) {
    if (ioMode != IOMode::SYSCALL)
        throw ExecExcept(syscallName + " syscall not supported in MMIO mode",
                         EXCEPT_CODE::SYSCALL_EXCEPTION);
}


void SystemHandle::execSyscall(const IOMode ioMode, State& state, StreamHandle& streamHandle) {
    int32_t syscallCode = state.registers[Register::V0];

    switch (static_cast<Syscall>(syscallCode)) {
        case Syscall::PRINT_INT:
            requiresSyscallMode(ioMode, "PRINT_INT");
            printIntSyscall(state, streamHandle);
            break;
        case Syscall::PRINT_STRING:
            requiresSyscallMode(ioMode, "PRINT_STRING");
            printStringSyscall(state, streamHandle);
            break;
        case Syscall::READ_INT:
            requiresSyscallMode(ioMode, "READ_INT");
            readIntSyscall(state, streamHandle);
            break;
        case Syscall::READ_STRING:
            requiresSyscallMode(ioMode, "READ_STRING");
            readStringSyscall(state, streamHandle);
            break;
        case Syscall::HEAP_ALLOC:
            heapAllocSyscall(state);
            break;
        case Syscall::EXIT:
            exitSyscall();
            break;
        case Syscall::PRINT_CHAR:
            requiresSyscallMode(ioMode, "PRINT_CHAR");
            printCharSyscall(state, streamHandle);
            break;
        case Syscall::READ_CHAR:
            requiresSyscallMode(ioMode, "READ_CHAR");
            readCharSyscall(state, streamHandle);
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
            printIntHexSyscall(state, streamHandle);
            break;
        case Syscall::PRINT_INT_BIN:
            requiresSyscallMode(ioMode, "PRINT_INT_BIN");
            printIntBinSyscall(state, streamHandle);
            break;
        case Syscall::PRINT_UINT:
            requiresSyscallMode(ioMode, "PRINT_UINT");
            printUIntSyscall(state, streamHandle);
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


void SystemHandle::printIntSyscall(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    streamHandle.putStr(std::to_string(value));
}


void SystemHandle::printStringSyscall(State& state, StreamHandle& streamHandle) {
    int32_t address = state.registers[Register::A0];
    while (true) {
        const unsigned char c = state.memory.byteAt(address);
        if (c == '\0')
            break;
        streamHandle.putChar(static_cast<char>(c));
        address++;
    }
}


void SystemHandle::readIntSyscall(State& state, StreamHandle& streamHandle) {
    std::string input;
    while (true) {
        const char c = streamHandle.getCharBlocking();
        if (c == '\n')
            break;

        if (c != '\b')
            input += c;
        else if (!input.empty())
            input.pop_back(); // Handle backspace
    }
    try {
        state.registers[Register::V0] = std::stoi(input);
    } catch (const std::invalid_argument&) {
        throw ExecExcept("Invalid input: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    } catch (const std::out_of_range&) {
        throw ExecExcept("Input out of range: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    }
}


void SystemHandle::readStringSyscall(State& state, StreamHandle& streamHandle) {
    const int32_t address = state.registers[Register::A0];
    const int32_t length = state.registers[Register::A1];
    int currLen = 0;
    while (currLen < length) {
        const char c = streamHandle.getCharBlocking();
        if (c == '\n')
            break;

        if (c != '\b') {
            state.memory.byteTo(address + currLen, c);
            currLen++;
        } else if (currLen > 0)
            currLen--;
    }
}


void SystemHandle::heapAllocSyscall(State& state) {
    const int32_t size = state.registers[Register::A0];
    const int32_t ptr = static_cast<int32_t>(state.heapAllocator.allocate(size));
    state.registers[Register::V0] = ptr;
}


void SystemHandle::exitSyscall() {
    throw ExecExit("Program exited with code " + std::to_string(0), 0);
}


void SystemHandle::printCharSyscall(const State& state, StreamHandle& streamHandle) {
    const char c = static_cast<char>(state.registers[Register::A0]);
    streamHandle.putChar(c);
}


void SystemHandle::readCharSyscall(State& state, StreamHandle& streamHandle) {
    const char c = streamHandle.getCharBlocking();
    state.registers[Register::V0] = 0xFF & static_cast<int32_t>(c);
}


void SystemHandle::exitValSyscall(const State& state) {
    const int32_t exitCode = state.registers[Register::A0];
    throw ExecExit("Program exited with code " + std::to_string(exitCode), exitCode);
}


void SystemHandle::timeSyscall(State& state) {
    // Get the current time in milliseconds since the epoch
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const int64_t milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Store high bits in $a1 and low bits in $a0
    state.registers[Register::A0] = static_cast<int32_t>(milliseconds & 0xFFFFFFFF);
    state.registers[Register::A1] = static_cast<int32_t>(milliseconds >> 32 & 0xFFFFFFFF);
}


void SystemHandle::sleepSyscall(State& state) {
    const int32_t milliseconds = state.registers[Register::A0];
    if (milliseconds < 0)
        throw ExecExcept("Negative sleep time: " + std::to_string(milliseconds),
                         EXCEPT_CODE::SYSCALL_EXCEPTION);

    usleep(milliseconds * 1000);
}


void SystemHandle::printIntHexSyscall(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << value;
    streamHandle.putStr(oss.str());
}

void SystemHandle::printIntBinSyscall(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    for (int i = 31; i >= 0; --i)
        streamHandle.putStr(std::to_string(value >> i & 1));
}

void SystemHandle::printUIntSyscall(const State& state, StreamHandle& streamHandle) {
    const uint32_t value = state.registers[Register::A0];
    streamHandle.putStr(std::to_string(value));
}

void SystemHandle::setRandSeedSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t seed = state.registers[Register::A1];
    rngMap[id] = RandomGenerator(seed);
}

void SystemHandle::randIntSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt());
}

void SystemHandle::randIntRangeSyscall(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t max = state.registers[Register::A1];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt(max));
}
