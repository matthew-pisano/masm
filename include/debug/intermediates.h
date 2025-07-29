//
// Created by matthew on 7/28/25.
//

#ifndef INTERMEDIATES_H
#define INTERMEDIATES_H
#include <string>

#include "interpreter/memory.h"
#include "parser/labels.h"


/**
 * Converts a memory layout to a human-readable string representation (preprocessed assembly)
 * @param layout The memory layout to convert
 * @param labelMap The label map to use for resolving labels
 * @return A string representation of the memory layout
 */
std::string layoutAsString(const MemLayout& layout, const LabelMap& labelMap);


/**
 * Converts a memory layout to a binary representation, which is a vector of bytes
 * @param layout The memory layout to convert
 * @return A vector of bytes representing the memory layout in binary form
 */
std::vector<std::byte> layoutAsBinary(const MemLayout& layout);


#endif // INTERMEDIATES_H
