//
// Created by matthew on 4/15/25.
//

#include "utils.h"
#include <regex>

bool isSignedInteger(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+$");
    return std::regex_match(str, pattern);
}


std::vector<std::byte> stringToBytes(const std::string& string, const bool nullTerminate) {
    std::vector<std::byte> bytes = {};
    for (const char c : string)
        bytes.push_back(static_cast<std::byte>(c));
    if (nullTerminate)
        bytes.push_back(static_cast<std::byte>(0));
    return bytes;
}


std::vector<std::byte> intStringToBytes(const std::string& string) {
    if (!isSignedInteger(string))
        throw std::runtime_error("Invalid integer " + string);

    const uint32_t integer = std::stoi(string);
    return i32ToBEByte(integer);
}


std::vector<Token> filterTokenList(const std::vector<Token>& listTokens) {
    std::vector<Token> elements = {};

    for (size_t i = 0; i < listTokens.size(); i++) {
        if (i % 2 == 1 && listTokens[i].type != TokenType::SEPERATOR)
            throw std::runtime_error("Expected , after token " + listTokens[i - 1].value);
        if (i % 2 == 0 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected ,");
        if (i == listTokens.size() - 1 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected , after token " + listTokens[i - 1].value);

        if (listTokens[i].type == TokenType::SEPERATOR)
            continue;
        // Only push non seperator elements
        elements.push_back(listTokens[i]);
    }

    return elements;
}


bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens) {
    if (pattern.size() != tokens.size())
        return false;

    for (size_t i = 0; i < pattern.size(); i++)
        if (tokens[i].type != pattern[i])
            return false;

    return true;
}


std::vector<std::byte> i32ToBEByte(const uint32_t i32) {
    // Break the instruction into 4 bytes (big-endian)
    std::vector<std::byte> bytes(4);
    bytes[0] = static_cast<std::byte>(i32 >> 24 & 0xFF); // Most significant byte
    bytes[1] = static_cast<std::byte>(i32 >> 16 & 0xFF);
    bytes[2] = static_cast<std::byte>(i32 >> 8 & 0xFF);
    bytes[3] = static_cast<std::byte>(i32 & 0xFF); // Least significant byte
    return bytes;
}
