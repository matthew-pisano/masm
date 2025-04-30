//
// Created by matthew on 4/29/25.
//

#ifndef DIRECTIVE_H
#define DIRECTIVE_H

#include <cstdint>
#include <vector>

#include "tokenizer.h"

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

#endif // DIRECTIVE_H
