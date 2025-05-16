//
// Created by matthew on 5/15/25.
//

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>


/**
 * A base class for all MASM exceptions
 */
class MasmException : public std::runtime_error {
    size_t lineno;

    explicit MasmException(const std::string& message, const size_t lineno) :
        std::runtime_error(message), lineno(lineno) {}

public:
    [[nodiscard]] const char* what() const noexcept override { return std::runtime_error::what(); }
};


/**
 * A class for syntax errors in MASM
 */
class MasmSyntaxError final : public MasmException {
public:
    explicit MasmSyntaxError(const std::string& message, const size_t lineno) :
        MasmException(message, lineno) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};


/**
 * A class for runtime errors in MASM
 */
class MasmRuntimeError final : public MasmException {
public:
    explicit MasmRuntimeError(const std::string& message, const size_t lineno) :
        MasmException(message, lineno) {}
    [[nodiscard]] const char* what() const noexcept override { return MasmException::what(); }
};

#endif // EXCEPTIONS_H
