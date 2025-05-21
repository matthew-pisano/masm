//
// Created by matthew on 4/26/25.
//

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <random>


#include "interpreter.h"


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
 * Executes the system call based on the value in the $v0 register
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 * @param ostream The output stream to print to
 */
void execSyscall(State& state, std::istream& istream, std::ostream& ostream);


/**
 * Prints the integer stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printIntSyscall(const State& state, std::ostream& ostream);

/**
 * Prints the null-terminated string stored in the memory at the address in $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printStringSyscall(State& state, std::ostream& ostream);

/**
 * Reads an integer from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readIntSyscall(State& state, std::istream& istream);

/**
 * Reads a string from the console and stores it in the memory at the address in $a0 up to the
 * length in $a1
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readStringSyscall(State& state, std::istream& istream);

/**
 * Allocates a block of memory of the size in $a0 and stores the address in $v0
 * @param state The current state of the interpreter
 */
void heapAllocSyscall(State& state);

/**
 * Exits the program with the exit code 0
 */
void exitSyscall();

/**
 * Prints the character stored in the register $a0 to the console
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printCharSyscall(const State& state, std::ostream& ostream);

/**
 * Reads a character from the console and stores it in the register $v0
 * @param state The current state of the interpreter
 * @param istream The input stream to read from
 */
void readCharSyscall(State& state, std::istream& istream);

/**
 * Exits the program with the exit code stored in $a0
 * @param state The current state of the interpreter
 */
void exitValSyscall(const State& state);

/**
 * Gets the current system time as a 64-bit integer with low bits in $a0 and high bits in $a1
 * @param state The current state of the interpreter
 */
void timeSyscall(State& state);

/**
 * Sleeps for the given number of milliseconds specified in $a0
 * @param state The current state of the interpreter
 */
void sleepSyscall(State& state);

/**
 * Prints the integer stored in the register $a0 as a hexadecimal value
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printIntHexSyscall(const State& state, std::ostream& ostream);

/**
 * Prints the integer stored in the register $a0 as a binary value
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printIntBinSyscall(const State& state, std::ostream& ostream);

/**
 * Prints the unsigned integer stored in the register $a0
 * @param state The current state of the interpreter
 * @param ostream The output stream to print to
 */
void printUIntSyscall(const State& state, std::ostream& ostream);

/**
 * Sets the random seed for the random number generator with the ID of the RNG in $a0 and the seed
 * in $a1
 * @param state The current state of the interpreter
 */
void setRandSeedSyscall(State& state);

/**
 * Generates a random integer from the random number generator with the ID in $a0 and stores it in
 * $a0
 * @param state The current state of the interpreter
 */
void randIntSyscall(State& state);

/**
 * Generates a random integer in the range [0, max] from the random number generator with the ID
 * in $a0, the max is in $a1, and stores it in $a0
 * @param state The current state of the interpreter
 */
void randIntRangeSyscall(State& state);

#endif // SYSCALLS_H
