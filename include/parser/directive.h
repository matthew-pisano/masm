//
// Created by matthew on 4/29/25.
//

#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include <cstdint>
#include <vector>

#include "tokenizer/tokenizer.h"


/**
 * Validates the arguments of a directive to ensure they match the expected pattern
 * @param dirToken The token for the directive
 * @param args Any argument tokens to pass to the directive
 * @throw runtime_error When the arguments for a directive are malformed
 */
void validateAllocDirective(const Token& dirToken, const std::vector<Token>& args);


/**
 * Parses a directive and its arguments into bytes that can be allocated to memory with padding
 * information preserved
 * @param loc The location in which the directive will be placed into memory
 * @param dirToken The token for the directive
 * @param args Any argument tokens to pass to the directive
 * @return A tuple containing the memory allocation associated with the directive and any padding
 * @throw runtime_error When the arguments for a directive are malformed
 */
std::tuple<std::vector<std::byte>, size_t>
parsePaddedAllocDirective(uint32_t loc, const Token& dirToken, const std::vector<Token>& args);

/**
 * Parses a directive and its arguments into bytes that can be allocated to memory
 * @param loc The location in which the directive will be placed into memory
 * @param dirToken The token for the directive
 * @param args Any argument tokens to pass to the directive
 * @return The memory allocation associated with the directive
 * @throw runtime_error When the arguments for a directive are malformed
 */
std::vector<std::byte> parseAllocDirective(uint32_t loc, const Token& dirToken,
                                           const std::vector<Token>& args);


/**
 * Allocates a block of memory of the given size, aligned to a multiple of the given value
 * @param loc The location in which the block will be placed into memory
 * @param blockSize The size of the block to allocate
 * @param blockAlign The alignment of the block
 * @return The memory allocation associated with the block
 */
std::vector<std::byte> parseAllocBlock(uint32_t loc, size_t blockSize, uint32_t blockAlign);


/**
 * Populates a given vector of bytes with the given string, optionally null terminating it
 * @param block The vector of bytes to populate
 * @param string The string to populate the vector with
 * @param nullTerminate Whether to null terminate the string
 */
void populateMemBlock(std::vector<std::byte>& block, const std::string& string, bool nullTerminate);


/**
 * Populates the given byte with the given byte
 * @param block The byte to populate
 * @param integer The integer to populate the byte with
 */
void populateMemBlock(std::byte& block, uint8_t integer);


/**
 * Populates a given vector of bytes with the given half word
 * @param block The vector of bytes to populate
 * @param integer The integer to populate the vector with
 */
void populateMemBlock(std::vector<std::byte>& block, uint16_t integer);


/**
 * Populates a given vector of bytes with the given word
 * @param block The vector of bytes to populate
 * @param integer The integer to populate the vector with
 * @throw runtime_error When the integer is out of range for the bytes within the vector
 */
void populateMemBlock(std::vector<std::byte>& block, uint32_t integer);


/**
 * Populates a given vector of bytes with the given single-precision floating point number
 * @param block The vector of bytes to populate
 * @param decimal The single-precision floating point number to populate the vector with
 */
void populateMemBlock(std::vector<std::byte>& block, float decimal);


/**
 * Populates a given vector of bytes with the given double-precision floating point number
 * @param block The vector of bytes to populate
 * @param decimal The double-precision floating point number to populate the vector with
 */
void populateMemBlock(std::vector<std::byte>& block, double decimal);


#endif // DIRECTIVE_H
