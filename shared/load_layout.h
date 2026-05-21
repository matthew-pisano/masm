//
// Created by matthew on 7/29/25.
//

#ifndef LOAD_LAYOUT_H
#define LOAD_LAYOUT_H

#include <filesystem>

#include <masm/assembler/memory.h>
#include <masm/assembler/parser.h>
#include <masm/assembler/serialization.h>


/**
 * Gets the basename of a file path
 * @param path The file path to get the basename of
 * @return The basename of the file path
 */
inline std::string getFileBasename(const std::string& path) {
    // Find the last occurrence of the directory separator (slash or backslash)
    const size_t sepPos = path.find_last_of(std::filesystem::path::preferred_separator);
    const std::string fileName = sepPos == std::string::npos ? path : path.substr(sepPos + 1);
    return fileName;
}


/**
 * Loads a memory layout from source files, which are MIPS assembly files
 * @param inputFileNames A vector of file names to load the MIPS assembly source code from
 * @param parser The parser to use for parsing the source code
 * @return A memory layout object constructed from the source files
 */
inline MemLayout loadLayoutFromSource(const std::vector<std::string>& inputFileNames, Parser& parser) {
    std::vector<SourceFile> sourceFiles;
    sourceFiles.reserve(inputFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : inputFileNames)
        sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);
    const MemLayout layout = parser.parse(program);

    return layout;
}


/**
 * Loads a memory layout from a binary file, which is a compiled MIPS program
 * @param inputFileNames A vector of file names to load the binary data from
 * @return A memory layout object constructed from the binary data
 */
inline MemLayout loadLayoutFromBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.size() > 1)
        throw std::runtime_error("Only one binary file may be loaded in at a time");

    try {
        const std::vector<std::byte> binary = readFileBytes(inputFileNames[0]);
        const MemLayout layout = loadLayout(binary);
        return layout;
    } catch ([[maybe_unused]] const std::exception& e) {
        throw std::runtime_error("Failed to load binary file '" + inputFileNames[0] +
                                 "': check to make sure the file exists and is not malformed");
    }
}


/**
 * Checks if the first input file is a binary file (compiled MIPS program)
 * @param inputFileNames A vector of file names to check
 * @return True if the first file is a binary file, false otherwise
 */
inline bool isLoadingBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.empty())
        return false;

    const std::string& firstName = inputFileNames[0];
    const size_t firstSize = firstName.size();
    return firstSize > 2 && firstName.substr(firstSize - 2, 2) == ".o";
}

#endif // LOAD_LAYOUT_H
