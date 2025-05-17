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

    /**
     * Constructs a message for the exception
     * @param message The message to display
     * @param useAddr Whether to use the address or line number
     * @param loc The line number or address of the error
     * @return The formatted message
     */
    static std::string constructMessage(const std::string& message, const bool useAddr,
                                        const size_t loc) {
        if (loc == 0ul)
            return message;
        const std::string msg = useAddr ? "Error at address" : "Error on line";
        return std::format("{} {}: {}", msg, loc, message);
    }

protected:
    explicit MasmException(const std::string& message, const bool useAddr,
                           const size_t lineno = 0) :
        std::runtime_error(constructMessage(message, useAddr, lineno)) {}

public:
    [[nodiscard]] const char* what() const noexcept override { return std::runtime_error::what(); }
};


/**
 * A class for syntax errors in MASM
 */
class MasmSyntaxError final : public MasmException {
public:
    explicit MasmSyntaxError(const std::string& message, const size_t lineno = 0) :
        MasmException(message, false, lineno) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * A class for runtime errors in MASM
 */
class MasmRuntimeError final : public MasmException {
public:
    explicit MasmRuntimeError(const std::string& message, const size_t addr = -1) :
        MasmException(message, true, addr) {}
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
