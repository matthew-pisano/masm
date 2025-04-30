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
 * Checks if a string is a value signed integer
 * @param str The string to check
 * @return True if the string is a signed integer, false otherwise
 */
bool isSignedInteger(const std::string& str);


/**
 * Converts a string to a vector of bytes, where each byte is the ASCII value of the character
 * @param string The string to convert
 * @param nullTerminate Whether to null terminate the string
 * @param escape Whether to escape the string
 * @return The vector of bytes representing the string
 */
std::vector<std::byte> stringToBytes(const std::string& string, bool nullTerminate, bool escape);


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
 * @return The filtered list of tokens
 * @throw runtime_error When the list is malformed
 */
std::vector<Token> filterTokenList(const std::vector<Token>& listTokens);


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

#endif // UTILS_H
