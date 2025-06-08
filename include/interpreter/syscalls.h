//
// Created by matthew on 4/26/25.
//

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <ctime>
#include <map>
#include <random>

#include "io/consoleio.h"
#include "state.h"


/**
 * Random number generator class that uses the Mersenne Twister algorithm
 */
class RandomGenerator {
    std::mt19937 rng; // Mersenne Twister engine

public:
    explicit RandomGenerator(const unsigned int seed) { rng.seed(seed); }
    RandomGenerator() { rng.seed(static_cast<unsigned int>(std::time(nullptr))); }

    /**
     * Generates a random integer in the given range
     * @param max The maximum value (inclusive)
     * @return A random integer in the range [0, max]
     */
    uint32_t getRandomInt(const int max) {
        std::uniform_int_distribution dist(0, max);
        return dist(rng);
    }

    /**
     * Generates a random integer in the range [INT32_MIN, INT32_MAX]
     * @return A random integer in the range [INT32_MIN, INT32_MAX]
     */
    uint32_t getRandomInt() { return getRandomInt(INT32_MAX); }
};


/**
 * Enumeration of the system calls available in the MIPS architecture
 */
enum class Syscall {
    // Keyboard/Display Syscalls
    PRINT_INT = 1,
    PRINT_STRING = 4,
    READ_INT = 5,
    READ_STRING = 8,
    HEAP_ALLOC = 9,
    EXIT = 10,
    PRINT_CHAR = 11,
    READ_CHAR = 12,
    EXIT_VAL = 17,

    // MARS Extended Syscalls
    TIME = 30,
    SLEEP = 32,
    PRINT_INT_HEX = 34,
    PRINT_INT_BIN = 35,
    PRINT_UINT = 36,
    SET_SEED = 40,
    RAND_INT = 41,
    RAND_INT_RANGE = 42
};


/**
 * SystemHandle class that handles the execution of system calls in the MIPS interpreter
 */
class SystemHandle {

    /**
     * A map of random number generator instances indexed by their IDs
     */
    std::map<size_t, RandomGenerator> rngMap = {};

    /**
     * Checks if the current I/O mode is SYSCALL mode, and throws an exception if it is not.
     * @param ioMode The current I/O mode of the interpreter
     * @param syscallName The name of the system call that requires SYSCALL mode
     * @throw ExecExcept if the I/O mode is not SYSCALL mode
     */
    static void requiresSyscallMode(IOMode ioMode, const std::string& syscallName);

public:
    /**
     * Executes the system call based on the value in the $v0 register
     * @param ioMode The I/O mode of the interpreter (some syscalls will fail if not in SYSCALL
     * mode)
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for input/output operations
     */
    void exec(IOMode ioMode, State& state, StreamHandle& streamHandle);

    /**
     * Prints the integer stored in the register $a0 to the console
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printInt(const State& state, StreamHandle& streamHandle);

    /**
     * Prints the null-terminated string stored in the memory at the address in $a0 to the console
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printString(State& state, StreamHandle& streamHandle);

    /**
     * Reads an integer from the console and stores it in the register $v0
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for reading
     */
    void readInt(State& state, StreamHandle& streamHandle);

    /**
     * Reads a string from the console and stores it in the memory at the address in $a0 up to the
     * length in $a1
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for reading
     */
    void readString(State& state, StreamHandle& streamHandle);

    /**
     * Allocates a block of memory of the size in $a0 and stores the address in $v0
     * @param state The current state of the interpreter
     */
    void heapAlloc(State& state);

    /**
     * Exits the program with the exit code 0
     */
    void exit();

    /**
     * Prints the character stored in the register $a0 to the console
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printChar(const State& state, StreamHandle& streamHandle);

    /**
     * Reads a character from the console and stores it in the register $v0
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for reading
     */
    void readChar(State& state, StreamHandle& streamHandle);

    /**
     * Exits the program with the exit code stored in $a0
     * @param state The current state of the interpreter
     */
    void exitVal(const State& state);

    /**
     * Gets the current system time as a 64-bit integer with low bits in $a0 and high bits in $a1
     * @param state The current state of the interpreter
     */
    void time(State& state);

    /**
     * Sleeps for the given number of milliseconds specified in $a0
     * @param state The current state of the interpreter
     */
    void sleep(State& state);

    /**
     * Prints the integer stored in the register $a0 as a hexadecimal value
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printIntHex(const State& state, StreamHandle& streamHandle);

    /**
     * Prints the integer stored in the register $a0 as a binary value
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printIntBin(const State& state, StreamHandle& streamHandle);

    /**
     * Prints the unsigned integer stored in the register $a0
     * @param state The current state of the interpreter
     * @param streamHandle The stream handle for printing
     */
    void printUInt(const State& state, StreamHandle& streamHandle);

    /**
     * Sets the random seed for the random number generator with the ID of the RNG in $a0 and the
     * seed in $a1
     * @param state The current state of the interpreter
     */
    void setRandSeed(State& state);

    /**
     * Generates a random integer from the random number generator with the ID in $a0 and stores it
     * in $a0
     * @param state The current state of the interpreter
     */
    void randInt(State& state);

    /**
     * Generates a random integer in the range [0, max] from the random number generator with the ID
     * in $a0, the max is in $a1, and stores it in $a0
     * @param state The current state of the interpreter
     */
    void randIntRange(State& state);
};

#endif // SYSCALLS_H
