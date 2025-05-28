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
    static std::string constructMessage(const std::string& message, const size_t addr,
                                        const std::string& filename, const size_t lineno) {
        std::string hexAddr = std::format("0x{:08X}", addr);
        return std::format("Runtime error at {} ({}:{}) -> {}", hexAddr, filename, lineno, message);
    }

public:
    explicit MasmRuntimeError(const std::string& message, const size_t addr,
                              const std::string& filename, const size_t lineno) :
        MasmException(constructMessage(message, addr, filename, lineno)) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * Execution to indicate that the program has terminated successfully eith the given code
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

#endif // EXCEPTIONS_H
