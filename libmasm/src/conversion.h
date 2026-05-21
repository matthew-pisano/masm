//
// Created by matthew on 5/20/26.
//

#ifndef MASM_CONVERSIONS_H
#define MASM_CONVERSIONS_H
#include <cstdint>
#include <string>
#include <vector>


/**
 * Checks if a string is a value signed integer
 * @param str The string to check
 * @return True if the string is a signed integer, false otherwise
 */
bool isSignedInteger(const std::string& str);


/**
 * Checks if a string is a value signed float
 * @param str The string to check
 * @return True if the string is a signed float, false otherwise
 */
bool isSignedFloat(const std::string& str);


/**
 * Converts a 32-bit integer to a vector of bytes in big-endian order
 * @param i32 The 32-bit integer to convert
 * @return A vector of bytes representing the integer in big-endian order
 */
std::vector<std::byte> i32ToBEByte(uint32_t i32);


/**
 * Converts a 16-bit integer to a vector of bytes in big-endian order
 * @param i16 The 16-bit integer to convert
 * @return A vector of bytes representing the integer in big-endian order
 */
std::vector<std::byte> i16ToBEByte(uint16_t i16);

/**
 * Converts a 32-bit float to a vector of bytes in big-endian order
 * @param f32 The 32-bit float to convert
 * @return A vector of bytes representing the float in big-endian order
 */
std::vector<std::byte> f32ToBEByte(float f32);


/**
 * Converts a 64-bit float to a vector of bytes in big-endian order
 * @param f64 The 64-bit float to convert
 * @return A vector of bytes representing the float in big-endian order
 */
std::vector<std::byte> f64ToBEByte(double f64);


/**
 * Converts a 32-bit integer to a vector of bytes in little-endian order
 * @param i32 The 32-bit integer to convert
 * @return A vector of bytes representing the integer in little-endian order
 */
std::vector<std::byte> i32ToLEByte(uint32_t i32);


/**
 * Converts a 16-bit integer to a vector of bytes in little-endian order
 * @param i16 The 16-bit integer to convert
 * @return A vector of bytes representing the integer in little-endian order
 */
std::vector<std::byte> i16ToLEByte(uint16_t i16);

/**
 * Converts a 32-bit float to a vector of bytes in little-endian order
 * @param f32 The 32-bit float to convert
 * @return A vector of bytes representing the float in little-endian order
 */
std::vector<std::byte> f32ToLEByte(float f32);


/**
 * Converts a 64-bit float to a vector of bytes in little-endian order
 * @param f64 The 64-bit float to convert
 * @return A vector of bytes representing the float in little-endian order
 */
std::vector<std::byte> f64ToLEByte(double f64);


/**
 * Converts an integer to a hexadecimal string
 * @param value The integer value to convert
 * @return The hexadecimal string representation of the integer value
 */
std::string i32ToHexString(uint32_t value);


/**
 * Converts a string to an unsigned 32-bit integer wth bounds and error handling
 * @param str The string to convert
 * @return The unsigned 32-bit integer value of the string
 */
uint32_t stringToi32(const std::string& str);


/**
 * Converts a string to a vector of bytes, where each byte is the ASCII value of the character
 * @param string The string to convert
 * @param nullTerminate Whether to null terminate the string
 * @return The vector of bytes representing the string
 */
std::vector<std::byte> stringToBytes(const std::string& string, bool nullTerminate);

#endif // MASM_CONVERSIONS_H
