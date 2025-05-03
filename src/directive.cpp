//
// Created by matthew on 4/29/25.
//

#include "directive.h"

#include <stdexcept>

#include "utils.h"


std::vector<std::byte> parseAllocDirective(const uint32_t loc, const Token& dirToken,
                                           const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    std::vector<std::byte> bytes = {};

    // Insert a string
    if (dirName == "asciiz" || dirName == "ascii") {
        if (args.size() != 1)
            throw std::runtime_error(dirName + " directive expects exactly one argument");
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(dirName + " directive expects a string argument");

        const std::string escapedString = escapeString(args[0].value);
        bytes = parseAllocBlock(loc, escapedString.length(), 1);
        populateMemBlock(bytes, escapedString, dirName == "asciiz");
    }
    // Insert a word
    else if (dirName == "word") {
        for (const Token& arg : args) {
            if (arg.type != TokenType::IMMEDIATE || !isSignedInteger(arg.value))
                throw std::runtime_error(dirName + " directive expects integers as arguments");

            std::vector<std::byte> wordBytes = parseAllocBlock(loc, 4, 4);
            populateMemBlock(wordBytes, std::stoi(arg.value));
            bytes.insert(bytes.end(), wordBytes.begin(), wordBytes.end());
        }
    }
    // Insert a space of the specified length
    else if (dirName == "space") {
        if (args.size() != 1)
            throw std::runtime_error("space directive expects exactly one argument");
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(args[0].value) ||
            std::stoi(args[0].value) <= 0)
            throw std::runtime_error(dirName + " directive expects a positive integer argument");

        bytes = parseAllocBlock(loc, std::stoi(args[0].value), 1);
    }
    // Insert padding to align next allocation
    else if (dirName == "align") {
        if (args.size() != 1)
            throw std::runtime_error("align directive expects exactly one argument");
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(args[0].value))
            throw std::runtime_error(dirName + " directive expects a positive integer argument");

        const int alignSize = std::stoi(args[0].value);
        if (alignSize < 0 || alignSize > 3)
            throw std::runtime_error("Alignment size must be between 0 and 3");
        bytes = parseAllocBlock(loc, 0, 2 << alignSize);
    } else
        throw std::runtime_error("Unsupported directive " + dirName);

    return bytes;
}


std::vector<std::byte> parseAllocBlock(const uint32_t loc, const int blockSize,
                                       const int blockAlign) {
    if (blockSize < 0)
        throw std::runtime_error("Block size must be positive");

    const uint32_t padding = blockAlign - (loc % blockAlign); // Pad to nearest multiple
    std::vector bytes(padding + blockSize, std::byte{0});
    return bytes;
}


void populateMemBlock(std::vector<std::byte>& block, const std::string& string,
                      const bool nullTerminate) {
    const std::vector<std::byte> bytes = stringToBytes(string, nullTerminate);
    if (block.size() < bytes.size())
        throw std::runtime_error("Block size is too small to hold string");
    block = bytes;
}


void populateMemBlock(std::vector<std::byte>& block, const int integer) {
    const std::vector<std::byte> bytes = i32ToBEByte(integer);
    if (block.size() < bytes.size())
        throw std::runtime_error("Block size is too small to hold integer");
    block = bytes;
}


void populateMemBlock(std::vector<std::byte>& block, const float decimal) {
    const std::vector<std::byte> bytes = f32ToBEByte(decimal);
    if (block.size() < bytes.size())
        throw std::runtime_error("Block size is too small to hold float");
    block = bytes;
}


void populateMemBlock(std::vector<std::byte>& block, const double decimal) {
    const std::vector<std::byte> bytes = f64ToBEByte(decimal);
    if (block.size() < bytes.size())
        throw std::runtime_error("Block size is too small to hold double");
    block = bytes;
}
