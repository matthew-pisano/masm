//
// Created by matthew on 4/15/25.
//

#include "utils.h"

#include <filesystem>
#include <regex>


std::string getFileBasename(const std::string& path) {
    // Find the last occurrence of the directory separator (slash or backslash)
    const size_t sepPos = path.find_last_of(std::filesystem::path::preferred_separator);
    const std::string fileName = sepPos == std::string::npos ? path : path.substr(sepPos + 1);
    return fileName;
}


bool isSignedInteger(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+$");
    return std::regex_match(str, pattern);
}


bool isSignedFloat(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+(\\.[0-9]*)?$");
    return std::regex_match(str, pattern);
}


std::string escapeString(const std::string& string) {
    std::string escapedString;
    bool toEscape = false;
    for (const char c : string) {
        if (!toEscape && c == '\\') {
            toEscape = true;
            continue;
        }

        if (toEscape) {
            if (c == 'n')
                escapedString += '\n';
            else if (c == 'r')
                escapedString += '\r';
            else if (c == 'b')
                escapedString += '\b';
            else if (c == 'f')
                escapedString += '\f';
            else if (c == 'a')
                escapedString += '\a';
            else if (c == 'v')
                escapedString += '\v';
            else if (c == 't')
                escapedString += '\t';
            else if (c == '\\' || c == '"')
                escapedString += c;
            else
                throw std::runtime_error("Invalid escape sequence \\" + std::string(1, c));
            toEscape = false;
            continue;
        }

        escapedString += c;
    }
    return escapedString;
}


std::vector<std::byte> stringToBytes(const std::string& string, const bool nullTerminate) {
    std::vector<std::byte> bytes = {};
    for (const char c : string)
        bytes.push_back(static_cast<std::byte>(c));
    if (nullTerminate)
        bytes.push_back(static_cast<std::byte>(0));
    return bytes;
}


std::vector<Token> filterTokenList(const std::vector<Token>& listTokens,
                                   const std::vector<TokenType>& validElems) {
    std::vector<Token> elements = {};

    for (size_t i = 0; i < listTokens.size(); i++) {
        if (i % 2 == 1 && listTokens[i].type != TokenType::SEPERATOR)
            throw std::runtime_error("Expected , after token " + listTokens[i - 1].value);
        if (i % 2 == 0 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected ','");
        if (i == listTokens.size() - 1 && listTokens[i].type == TokenType::SEPERATOR)
            throw std::runtime_error("Unexpected ',' after token '" + listTokens[i - 1].value +
                                     "'");

        if (listTokens[i].type == TokenType::SEPERATOR)
            continue;

        if (!validElems.empty() &&
            std::ranges::find(validElems, listTokens[i].type) == validElems.end())
            throw std::runtime_error("Invalid token '" + listTokens[i].value + "' of type '" +
                                     tokenTypeToString(listTokens[i].type) + "'");
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


std::vector<std::byte> i16ToBEByte(const uint16_t i16) {
    // Break the instruction into 2 bytes (big-endian)
    std::vector<std::byte> bytes(2);
    bytes[0] = static_cast<std::byte>(i16 >> 8 & 0xFF); // Most significant byte
    bytes[1] = static_cast<std::byte>(i16 & 0xFF); // Least significant byte
    return bytes;
}


std::vector<std::byte> f32ToBEByte(float f32) {
    const uint32_t i32 = *reinterpret_cast<uint32_t*>(&f32);
    return i32ToBEByte(i32);
}


std::vector<std::byte> f64ToBEByte(double f64) {
    const uint64_t i64 = *reinterpret_cast<uint64_t*>(&f64);
    std::vector<std::byte> upperBytes = i32ToBEByte(i64 >> 32 & 0xFFFFFFFF);
    std::vector<std::byte> lowerBytes = i32ToBEByte(i64 & 0xFFFFFFFF);
    upperBytes.insert(upperBytes.end(), lowerBytes.begin(), lowerBytes.end());
    return upperBytes;
}


std::string hexToInt(std::string hex) {
    const std::regex pattern("^[-]?0x[0-9a-fA-F]+$");
    if (!std::regex_match(hex, pattern))
        throw std::runtime_error("Invalid hex integer " + hex);

    hex = hex.substr(2);
    uint32_t hexInt;
    try {
        hexInt = std::stoull(hex, nullptr, 16);
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Hex integer out of range: " + hex);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid hex integer: " + hex);
    }
    return std::to_string(hexInt);
}


uint32_t stoui32(const std::string& str) {
    if (!isSignedInteger(str))
        throw std::runtime_error("Invalid integer " + str);

    try {
        const uint32_t value = std::stoul(str);
        if (value > UINT32_MAX)
            throw std::runtime_error("Unsigned integer out of range: " + str);

        return value;
    } catch (const std::out_of_range& e) {
        throw std::runtime_error("Unsigned integer out of range: " + str);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error("Invalid unsigned integer: " + str);
    }
}
