//
// Created by matthew on 5/20/26.
//

#include "conversion.h"

#include <regex>
#include <sstream>
#include <stdexcept>


bool isSignedInteger(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+$");
    return std::regex_match(str, pattern);
}


bool isSignedFloat(const std::string& str) {
    const std::regex pattern("^[-]?[0-9]+(\\.[0-9]*)?$");
    return std::regex_match(str, pattern);
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


std::vector<std::byte> i32ToLEByte(const uint32_t i32) {
    // Break the instruction into 4 bytes (little-endian)
    std::vector<std::byte> bytes(4);
    bytes[0] = static_cast<std::byte>(i32 & 0xFF); // Least significant byte
    bytes[1] = static_cast<std::byte>(i32 >> 8 & 0xFF);
    bytes[2] = static_cast<std::byte>(i32 >> 16 & 0xFF);
    bytes[3] = static_cast<std::byte>(i32 >> 24 & 0xFF); // Most significant byte
    return bytes;
}


std::vector<std::byte> i16ToLEByte(const uint16_t i16) {
    // Break the instruction into 2 bytes (little-endian)
    std::vector<std::byte> bytes(2);
    bytes[0] = static_cast<std::byte>(i16 & 0xFF); // Least significant byte
    bytes[1] = static_cast<std::byte>(i16 >> 8 & 0xFF); // Most significant byte
    return bytes;
}


std::vector<std::byte> f32ToLEByte(float f32) {
    const uint32_t i32 = *reinterpret_cast<uint32_t*>(&f32);
    return i32ToLEByte(i32);
}


std::vector<std::byte> f64ToLEByte(double f64) {
    const uint64_t i64 = *reinterpret_cast<uint64_t*>(&f64);
    std::vector<std::byte> upperBytes = i32ToLEByte(i64 >> 32 & 0xFFFFFFFF);
    std::vector<std::byte> lowerBytes = i32ToLEByte(i64 & 0xFFFFFFFF);
    upperBytes.insert(upperBytes.end(), lowerBytes.begin(), lowerBytes.end());
    return upperBytes;
}


std::vector<std::byte> stringToBytes(const std::string& string, const bool nullTerminate) {
    std::vector<std::byte> bytes = {};
    for (const char c : string)
        bytes.push_back(static_cast<std::byte>(c));
    if (nullTerminate)
        bytes.push_back(static_cast<std::byte>(0));
    return bytes;
}


std::string hexToInt(std::string hex) {
    const std::regex pattern("^[-]?0x[0-9a-fA-F]+$");
    if (!std::regex_match(hex, pattern))
        throw std::runtime_error("Invalid hex integer " + hex);

    hex = hex.substr(2);
    uint32_t hexInt;
    try {
        hexInt = std::stoull(hex, nullptr, 16);
    } catch ([[maybe_unused]] const std::out_of_range& e) {
        throw std::runtime_error("Hex integer out of range: " + hex);
    } catch ([[maybe_unused]] const std::invalid_argument& e) {
        throw std::runtime_error("Invalid hex integer: " + hex);
    }
    return std::to_string(hexInt);
}


std::string hexToString(const uint32_t value) {
    std::stringstream ss;
    ss << "0x" << std::hex << value;
    return ss.str();
}


uint32_t stoui32(const std::string& str) {
    if (!isSignedInteger(str))
        throw std::runtime_error("Invalid integer " + str);

    try {
        const uint32_t value = std::stoul(str);
        if (value > UINT32_MAX)
            throw std::runtime_error("Unsigned integer out of range: " + str);

        return value;
    } catch ([[maybe_unused]] const std::out_of_range& e) {
        throw std::runtime_error("Unsigned integer out of range: " + str);
    } catch ([[maybe_unused]] const std::invalid_argument& e) {
        throw std::runtime_error("Invalid unsigned integer: " + str);
    }
}
