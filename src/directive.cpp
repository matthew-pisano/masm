//
// Created by matthew on 4/29/25.
//

#include "directive.h"

#include <stdexcept>

#include "utils.h"


std::vector<std::byte> parseAllocDirective(uint32_t loc, const Token& dirToken,
                                           const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    std::vector<std::byte> bytes = {};

    // Return a byte vector containing each character of the string
    if (dirName == "asciiz" || dirName == "ascii") {
        if (args.size() != 1)
            throw std::runtime_error(dirName + " directive expects exactly one argument");
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(dirName + " directive expects a string argument");
        return stringToBytes(args[0].value, dirName == "asciiz", true);
    }
    // Return a word for each argument given
    if (dirName == "word") {
        for (const Token& arg : args) {
            if (arg.type != TokenType::IMMEDIATE)
                throw std::runtime_error("word directive expects immediate values as arguments");
            std::vector<std::byte> immediateBytes = intStringToBytes(arg.value);
            bytes.insert(bytes.end(), immediateBytes.begin(), immediateBytes.end());
        }
        return bytes;
    }

    if (dirName == "space") {
        if (args.size() != 1)
            throw std::runtime_error("space directive expects exactly one argument");
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(args[0].value) ||
            std::stoi(args[0].value) <= 0)
            throw std::runtime_error(dirName + " directive expects a positive integer argument");

        for (int i = 0; i < std::stoi(args[0].value); ++i)
            bytes.push_back(static_cast<std::byte>(0));
        return bytes;
    }

    throw std::runtime_error("Unsupported directive " + dirName);
}
