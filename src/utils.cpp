//
// Created by matthew on 4/15/25.
//

#include "utils.h"
#include <regex>

bool isSignedInteger(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+$");
    return std::regex_match(str, pattern);
}


std::vector<std::byte> stringToBytes(const std::string& string) {
    std::vector<std::byte> bytes = {};
    for (const char c : string)
        bytes.push_back(static_cast<std::byte>(c));
    return bytes;
}


std::vector<std::byte> intStringToBytes(const std::string& string) {
    if (!isSignedInteger(string))
        throw std::runtime_error("Invalid integer " + string);

    const int integer = std::stoi(string);

    std::vector<std::byte> bytes = {};
    // Using big endian
    bytes.push_back(static_cast<std::byte>(integer >> 24 & 0xFF));
    bytes.push_back(static_cast<std::byte>(integer >> 16 & 0xFF));
    bytes.push_back(static_cast<std::byte>(integer >> 8 & 0xFF));
    bytes.push_back(static_cast<std::byte>(integer & 0xFF));
    return bytes;
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
