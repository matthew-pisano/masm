//
// Created by matthew on 4/26/25.
//

#include "interpreter/syscalls.h"

#include <chrono>
#include <unistd.h>

#include "exceptions.h"
#include "interpreter/cpu.h"
#include "io/consoleio.h"


void SystemHandle::requiresSyscallMode(const IOMode ioMode, const std::string& syscallName) {
    if (ioMode != IOMode::SYSCALL)
        throw ExecExcept(syscallName + " syscall not supported in MMIO mode",
                         EXCEPT_CODE::SYSCALL_EXCEPTION);
}

void SystemHandle::exec(const IOMode ioMode, State& state, StreamHandle& streamHandle) {
    int32_t syscallCode = state.registers[Register::V0];

    switch (static_cast<Syscall>(syscallCode)) {
        case Syscall::PRINT_INT:
            requiresSyscallMode(ioMode, "PRINT_INT");
            printInt(state, streamHandle);
            break;
        case Syscall::PRINT_STRING:
            requiresSyscallMode(ioMode, "PRINT_STRING");
            printString(state, streamHandle);
            break;
        case Syscall::READ_INT:
            requiresSyscallMode(ioMode, "READ_INT");
            readInt(state, streamHandle);
            break;
        case Syscall::READ_STRING:
            requiresSyscallMode(ioMode, "READ_STRING");
            readString(state, streamHandle);
            break;
        case Syscall::HEAP_ALLOC:
            heapAlloc(state);
            break;
        case Syscall::EXIT:
            exit();
            break;
        case Syscall::PRINT_CHAR:
            requiresSyscallMode(ioMode, "PRINT_CHAR");
            printChar(state, streamHandle);
            break;
        case Syscall::READ_CHAR:
            requiresSyscallMode(ioMode, "READ_CHAR");
            readChar(state, streamHandle);
            break;
        case Syscall::EXIT_VAL:
            exitVal(state);
            break;
        case Syscall::TIME:
            time(state);
            break;
        case Syscall::SLEEP:
            sleep(state);
            break;
        case Syscall::PRINT_INT_HEX:
            requiresSyscallMode(ioMode, "PRINT_INT_HEX");
            printIntHex(state, streamHandle);
            break;
        case Syscall::PRINT_INT_BIN:
            requiresSyscallMode(ioMode, "PRINT_INT_BIN");
            printIntBin(state, streamHandle);
            break;
        case Syscall::PRINT_UINT:
            requiresSyscallMode(ioMode, "PRINT_UINT");
            printUInt(state, streamHandle);
            break;
        case Syscall::SET_SEED:
            setRandSeed(state);
            break;
        case Syscall::RAND_INT:
            randInt(state);
            break;
        case Syscall::RAND_INT_RANGE:
            randIntRange(state);
            break;
        default:
            throw std::runtime_error("Unknown syscall " + std::to_string(syscallCode));
    }
}

void SystemHandle::printInt(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    streamHandle.putStr(std::to_string(value));
}

void SystemHandle::printFloat(const State& state, StreamHandle& streamHandle) {
    const float32_t value = state.cp1.getFloat(Coproc1Register::F12);
    std::ostringstream oss;
    oss << std::setprecision(6) << value; // Set precision for float output
    streamHandle.putStr(oss.str());
}

void SystemHandle::printDouble(const State& state, StreamHandle& streamHandle) {
    const float64_t value = state.cp1.getDouble(Coproc1Register::F12);
    std::ostringstream oss;
    oss << std::setprecision(6) << value; // Set precision for double output
    streamHandle.putStr(oss.str());
}

void SystemHandle::printString(State& state, StreamHandle& streamHandle) {
    int32_t address = state.registers[Register::A0];
    while (true) {
        const unsigned char c = state.memory.byteAt(address);
        if (c == '\0')
            break;
        streamHandle.putChar(static_cast<char>(c));
        address++;
    }
}

void SystemHandle::readInt(State& state, StreamHandle& streamHandle) {
    const std::string input = readSeq(streamHandle);
    try {
        state.registers[Register::V0] = std::stoi(input);
    } catch (const std::invalid_argument&) {
        throw ExecExcept("Invalid input: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    } catch (const std::out_of_range&) {
        throw ExecExcept("Input out of range: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    }
}

void SystemHandle::readFloat(State& state, StreamHandle& streamHandle) {
    const std::string input = readSeq(streamHandle);
    try {
        state.cp1.setFloat(Coproc1Register::F0, std::stof(input));
    } catch (const std::invalid_argument&) {
        throw ExecExcept("Invalid float input: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    } catch (const std::out_of_range&) {
        throw ExecExcept("Float input out of range: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    }
}

void SystemHandle::readDouble(State& state, StreamHandle& streamHandle) {
    const std::string input = readSeq(streamHandle);
    try {
        state.cp1.setDouble(Coproc1Register::F0, std::stod(input));
    } catch (const std::invalid_argument&) {
        throw ExecExcept("Invalid double input: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    } catch (const std::out_of_range&) {
        throw ExecExcept("Double input out of range: " + input, EXCEPT_CODE::SYSCALL_EXCEPTION);
    }
}


void SystemHandle::readString(State& state, StreamHandle& streamHandle) {
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

void SystemHandle::heapAlloc(State& state) {
    const int32_t size = state.registers[Register::A0];
    const int32_t ptr = static_cast<int32_t>(state.heapAllocator.allocate(size));
    state.registers[Register::V0] = ptr;
}

void SystemHandle::exit() { throw ExecExit("Program exited with code " + std::to_string(0), 0); }

void SystemHandle::printChar(const State& state, StreamHandle& streamHandle) {
    const char c = static_cast<char>(state.registers[Register::A0]);
    streamHandle.putChar(c);
}

void SystemHandle::readChar(State& state, StreamHandle& streamHandle) {
    const char c = streamHandle.getCharBlocking();
    state.registers[Register::V0] = 0xFF & static_cast<int32_t>(c);
}

void SystemHandle::exitVal(const State& state) {
    const int32_t exitCode = state.registers[Register::A0];
    throw ExecExit("Program exited with code " + std::to_string(exitCode), exitCode);
}

void SystemHandle::time(State& state) {
    // Get the current time in milliseconds since the epoch
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const int64_t milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Store high bits in $a1 and low bits in $a0
    state.registers[Register::A0] = static_cast<int32_t>(milliseconds & 0xFFFFFFFF);
    state.registers[Register::A1] = static_cast<int32_t>(milliseconds >> 32 & 0xFFFFFFFF);
}

void SystemHandle::sleep(State& state) {
    const int32_t milliseconds = state.registers[Register::A0];
    if (milliseconds < 0)
        throw ExecExcept("Negative sleep time: " + std::to_string(milliseconds),
                         EXCEPT_CODE::SYSCALL_EXCEPTION);

    usleep(milliseconds * 1000);
}

void SystemHandle::printIntHex(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << value;
    streamHandle.putStr(oss.str());
}

void SystemHandle::printIntBin(const State& state, StreamHandle& streamHandle) {
    const int32_t value = state.registers[Register::A0];
    for (int i = 31; i >= 0; --i)
        streamHandle.putStr(std::to_string(value >> i & 1));
}

void SystemHandle::printUInt(const State& state, StreamHandle& streamHandle) {
    const uint32_t value = state.registers[Register::A0];
    streamHandle.putStr(std::to_string(value));
}

void SystemHandle::setRandSeed(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t seed = state.registers[Register::A1];
    rngMap[id] = RandomGenerator(seed);
}

void SystemHandle::randInt(State& state) {
    const int32_t id = state.registers[Register::A0];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt());
}

void SystemHandle::randIntRange(State& state) {
    const int32_t id = state.registers[Register::A0];
    const int32_t max = state.registers[Register::A1];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.registers[Register::A0] = static_cast<int32_t>(rngMap[id].getRandomInt(max));
}

void SystemHandle::randFloat(State& state) {
    const int32_t id = state.registers[Register::A0];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.cp1.setFloat(Coproc1Register::F0, rngMap[id].getRandomFloat());
}

void SystemHandle::randDouble(State& state) {
    const int32_t id = state.registers[Register::A0];
    if (!rngMap.contains(id))
        rngMap[id] = RandomGenerator();
    state.cp1.setDouble(Coproc1Register::F0, rngMap[id].getRandomDouble());
}
