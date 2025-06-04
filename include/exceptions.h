//
// Created by matthew on 5/15/25.
//

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <format>
#include <stdexcept>


/**
 * A base class for all MASM exceptions
 */
class MasmException : public std::runtime_error {
protected:
    explicit MasmException(const std::string& message) : std::runtime_error(message) {}

public:
    [[nodiscard]] const char* what() const noexcept override { return std::runtime_error::what(); }
};


/**
 * A class for syntax errors in MASM
 */
class MasmSyntaxError final : public MasmException {
    static std::string constructMessage(const std::string& message, const std::string& filename,
                                        const size_t lineno) {
        return std::format("Syntax error at {}:{} -> {}", filename, lineno, message);
    }

public:
    explicit MasmSyntaxError(const std::string& message, const std::string& filename,
                             const size_t lineno) :
        MasmException(constructMessage(message, filename, lineno)) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * A class for runtime errors in MASM
 */
class MasmRuntimeError final : public MasmException {
    /**
     * Constructs a message for the runtime exception
     * @param message The original error message to display
     * @param addr The address of the error
     * @param lineno The line number of the source code that produced the error
     * @param filename The name of the source file that produced the error
     * @return The formatted message
     */
    static std::string constructMessage(const std::string& message, const uint32_t addr,
                                        const std::string& filename, const size_t lineno) {
        std::string hexAddr = std::format("0x{:08X}", addr);
        return std::format("Runtime error at {} ({}:{}) -> {}", hexAddr, filename, lineno, message);
    }

public:
    explicit MasmRuntimeError(const std::string& message, const uint32_t addr,
                              const std::string& filename, const size_t lineno) :
        MasmException(constructMessage(message, addr, filename, lineno)) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * Execution to indicate that the program has terminated successfully with the given code
 */
class ExecExit final : public std::runtime_error {
    int errorCode;

public:
    explicit ExecExit(const std::string& message, const int code) :
        std::runtime_error(message), errorCode(code) {}

    /**
     * Get the error code of the exception
     * @return The error code
     */
    [[nodiscard]] int code() const { return errorCode; }
};


/**
 * The possible exception codes thrown by the interpreter (stored in bits [2-6] of cause register)
 */
enum class EXCEPT_CODE {
    ADDRESS_EXCEPTION_LOAD = 0x0010,
    ADDRESS_EXCEPTION_STORE = 0x0014,
    SYSCALL_EXCEPTION = 0x0020,
    BREAKPOINT_EXCEPTION = 0x0024,
    RESERVED_INSTRUCTION_EXCEPTION = 0x0028,
    ARITHMETIC_OVERFLOW_EXCEPTION = 0x0030,
    TRAP_EXCEPTION = 0x0034,
    DIVIDE_BY_ZERO_EXCEPTION = 0x003c,
    FLOATING_POINT_OVERFLOW = 0x0040,
    FLOATING_POINT_UNDERFLOW = 0x0044
};


/**
 * Execution exception to indicate that an error has occurred during execution
 */
class ExecExcept final : public std::runtime_error {
    EXCEPT_CODE causeCode;

public:
    explicit ExecExcept(const std::string& message, const EXCEPT_CODE code) :
        std::runtime_error(message), causeCode(code) {}

    /**
     * Get the cause value of the exception
     * @return The cause value
     */
    [[nodiscard]] EXCEPT_CODE cause() const { return causeCode; }
};

#endif // EXCEPTIONS_H
