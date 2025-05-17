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
     * @param lineno The line number of the error
     * @return The formatted message
     */
    static std::string constructMessage(const std::string& message, const size_t lineno) {
        if (lineno == -1ul)
            return message;
        return std::format("{} {}: {}", "Error on line", lineno, message);
    }

protected:
    explicit MasmException(const std::string& message, const size_t lineno = -1) :
        std::runtime_error(constructMessage(message, lineno)) {}

public:
    [[nodiscard]] const char* what() const noexcept override { return std::runtime_error::what(); }
};


/**
 * A class for syntax errors in MASM
 */
class MasmSyntaxError final : public MasmException {
public:
    explicit MasmSyntaxError(const std::string& message, const size_t lineno = -1) :
        MasmException(message, lineno) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * A class for runtime errors in MASM
 */
class MasmRuntimeError final : public MasmException {
public:
    explicit MasmRuntimeError(const std::string& message, const size_t lineno = -1) :
        MasmException(message, lineno) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};

#endif // EXCEPTIONS_H
