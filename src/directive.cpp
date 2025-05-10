//
// Created by matthew on 4/29/25.
//

#include "directive.h"

#include <algorithm>
#include <stdexcept>

#include "utils.h"


void validateAllocDirective(const Token& dirToken, const std::vector<Token>& args) {
    const std::string dirName = dirToken.value;
    if (args.empty())
        throw std::runtime_error("Directive " + dirName + " expects at least one argument");

    const std::vector<std::string> singleArgDirectives = {"asciiz", "ascii", "space", "align"};
    if (std::ranges::find(singleArgDirectives, dirName) != singleArgDirectives.end() &&
        args.size() != 1)
        throw std::runtime_error(dirName + " directive expects exactly one argument");


    if (dirName == "align") {
        const std::string val = args[0].value;
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(val) || std::stoi(val) < 0 ||
            std::stoi(val) > 3)
            throw std::runtime_error(dirName +
                                     " directive expects an integer argument between 0 and 3");
    } else if (dirName == "asciiz" || dirName == "ascii") {
        if (args[0].type != TokenType::STRING)
            throw std::runtime_error(dirName + " directive expects a string argument");
    } else if (dirName == "byte" || dirName == "half" || dirName == "word") {
        for (const Token& arg : args)
            if (arg.type != TokenType::IMMEDIATE || !isSignedInteger(arg.value))
                throw std::runtime_error(dirName + " directive expects integers as arguments");
    } else if (dirName == "double" || dirName == "float") {
        for (const Token& arg : args)
            if (arg.type != TokenType::IMMEDIATE || !isSignedFloat(arg.value))
                throw std::runtime_error(dirName + " directive expects floats as arguments");
    } else if (dirName == "space") {
        const std::string val = args[0].value;
        if (args[0].type != TokenType::IMMEDIATE || !isSignedInteger(val) || std::stoi(val) <= 0)
            throw std::runtime_error(dirName + " directive expects a positive integer argument");
    } else
        throw std::runtime_error("Unsupported directive " + dirName);
}


std::vector<std::byte> parseAllocDirective(const uint32_t loc, const Token& dirToken,
                                           const std::vector<Token>& args) {

    // Throw error if pattern for directive is invalid
    validateAllocDirective(dirToken, args);

    const std::string dirName = dirToken.value;
    std::vector<std::byte> bytes = {};

    // Insert padding to align next allocation
    if (dirName == "align") {
        bytes = parseAllocBlock(loc, 0, 1 << std::stoi(args[0].value));
    }
    // Insert a string
    else if (dirName == "asciiz" || dirName == "ascii") {
        const std::string escapedString = escapeString(args[0].value);
        // Add extra byte if null terminating
        const size_t blockSize = escapedString.length() + (dirName == "asciiz" ? 1 : 0);
        bytes = parseAllocBlock(loc, blockSize, 1);
        populateMemBlock(bytes, escapedString, dirName == "asciiz");
    }
    // Insert a byte
    else if (dirName == "byte") {
        for (const Token& arg : args) {
            std::byte byte = parseAllocBlock(loc, 1, 1)[0];
            populateMemBlock(byte, std::stoi(arg.value));
            bytes.push_back(byte);
        }
    }
    // Insert a double
    else if (dirName == "double") {
        for (const Token& arg : args) {
            std::vector<std::byte> doubleBytes = parseAllocBlock(loc, 8, 8);
            populateMemBlock(doubleBytes, std::stod(arg.value));
            bytes.insert(bytes.end(), doubleBytes.begin(), doubleBytes.end());
        }
    }
    // Insert a float
    else if (dirName == "float") {
        for (const Token& arg : args) {
            std::vector<std::byte> doubleBytes = parseAllocBlock(loc, 4, 4);
            populateMemBlock(doubleBytes, std::stof(arg.value));
            bytes.insert(bytes.end(), doubleBytes.begin(), doubleBytes.end());
        }
    }
    // Insert a half word
    else if (dirName == "half") {
        for (const Token& arg : args) {
            std::vector<std::byte> halfBytes = parseAllocBlock(loc, 2, 2);
            populateMemBlock(halfBytes, static_cast<uint16_t>(std::stoi(arg.value)));
            bytes.insert(bytes.end(), halfBytes.begin(), halfBytes.end());
        }
    }
    // Insert a space of the specified length
    else if (dirName == "space") {
        bytes = parseAllocBlock(loc, std::stoi(args[0].value), 1);
    }
    // Insert a word
    else if (dirName == "word") {
        for (const Token& arg : args) {
            std::vector<std::byte> wordBytes = parseAllocBlock(loc, 4, 4);
            populateMemBlock(wordBytes, static_cast<uint32_t>(std::stoi(arg.value)));
            bytes.insert(bytes.end(), wordBytes.begin(), wordBytes.end());
        }
    } else
        throw std::runtime_error("Unsupported directive " + dirName);

    return bytes;
}


std::vector<std::byte> parseAllocBlock(const uint32_t loc, const size_t blockSize,
                                       const uint32_t blockAlign) {
    if (blockAlign == 0)
        throw std::runtime_error("Block alignment cannot be zero");

    uint32_t padding = blockAlign - (loc % blockAlign); // Pad to nearest multiple
    if (padding == blockAlign)
        padding = 0;
    std::vector bytes(padding + blockSize, std::byte{0});
    return bytes;
}


void populateMemBlock(std::vector<std::byte>& block, const std::string& string,
                      const bool nullTerminate) {
    const std::vector<std::byte> bytes = stringToBytes(string, nullTerminate);
    const long offset = static_cast<long>(block.size() - bytes.size());
    std::ranges::copy(bytes, block.begin() + offset);
}


void populateMemBlock(std::byte& block, const uint8_t integer) {
    block = static_cast<std::byte>(integer);
}


void populateMemBlock(std::vector<std::byte>& block, const uint16_t integer) {
    const std::vector<std::byte> bytes = i16ToBEByte(integer);
    const long offset = static_cast<long>(block.size() - bytes.size());
    std::ranges::copy(bytes, block.begin() + offset);
}


void populateMemBlock(std::vector<std::byte>& block, const uint32_t integer) {
    const std::vector<std::byte> bytes = i32ToBEByte(integer);
    const long offset = static_cast<long>(block.size() - bytes.size());
    std::ranges::copy(bytes, block.begin() + offset);
}


void populateMemBlock(std::vector<std::byte>& block, const float decimal) {
    const std::vector<std::byte> bytes = f32ToBEByte(decimal);
    const long offset = static_cast<long>(block.size() - bytes.size());
    std::ranges::copy(bytes, block.begin() + offset);
}


void populateMemBlock(std::vector<std::byte>& block, const double decimal) {
    const std::vector<std::byte> bytes = f64ToBEByte(decimal);
    const long offset = static_cast<long>(block.size() - bytes.size());
    std::ranges::copy(bytes, block.begin() + offset);
}
