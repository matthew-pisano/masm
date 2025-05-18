//
// Created by matthew on 4/15/25.
//

#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <tokenizer.h>
#include <vector>


/**
 * Gets the basename of a file path
 * @param path The file path to get the basename of
 * @return The basename of the file path
 */
std::string getFileBasename(const std::string& path);


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
 * Escapes a string by replacing escape sequences with their corresponding characters
 * @param string The string to escape
 * @return The escaped string
 */
std::string escapeString(const std::string& string);


/**
 * Converts a string to a vector of bytes, where each byte is the ASCII value of the character
 * @param string The string to convert
 * @param nullTerminate Whether to null terminate the string
 * @return The vector of bytes representing the string
 */
std::vector<std::byte> stringToBytes(const std::string& string, bool nullTerminate);


/**
 * Converts a string representing an integer into a 32-bit big endian representation in bytes
 * @param string The string to convert
 * @return The vector of bytes representing the integer
 * @throw runtime_error When the string is not a valid signed integer
 */
std::vector<std::byte> intStringToBytes(const std::string& string);


/**
 * Validates a comma seperated list of tokens, returning the list with commas stripped out
 * @param listTokens The list of tokens to filter
 * @param validElems The only valid token types to include in the filtered list
 * @return The filtered list of tokens
 * @throw runtime_error When the list is malformed or contains invalid tokens
 */
std::vector<Token> filterTokenList(const std::vector<Token>& listTokens,
                                   const std::vector<TokenType>& validElems = {});


/**
 * Checks to see if a given vector of tokens matches a token type pattern
 * @param pattern The pattern to match against
 * @param tokens The tokens to check
 * @return True if the tokens match the pattern, false otherwise
 */
bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens);


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

#endif // UTILS_H
