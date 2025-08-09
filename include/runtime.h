//
// Created by matthew on 7/29/25.
//

#ifndef RUNTIME_H
#define RUNTIME_H
#include "interpreter/memory.h"
#include "parser/parser.h"

/**
 * Loads a memory layout from source files, which are MIPS assembly files
 * @param inputFileNames A vector of file names to load the MIPS assembly source code from
 * @param parser The parser to use for parsing the source code
 * @return A memory layout object constructed from the source files
 */
MemLayout loadLayoutFromSource(const std::vector<std::string>& inputFileNames, Parser& parser);


/**
 * Loads a memory layout from a binary file, which is a compiled MIPS program
 * @param inputFileNames A vector of file names to load the binary data from
 * @return A memory layout object constructed from the binary data
 */
MemLayout loadLayoutFromBinary(const std::vector<std::string>& inputFileNames);


/**
 * Checks if the first input file is a binary file (compiled MIPS program)
 * @param inputFileNames A vector of file names to check
 * @return True if the first file is a binary file, false otherwise
 */
bool isLoadingBinary(const std::vector<std::string>& inputFileNames);

#endif // RUNTIME_H
