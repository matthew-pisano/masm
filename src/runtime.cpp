//
// Created by matthew on 7/29/25.
//


#include "runtime.h"

#include "debug/intermediates.h"
#include "io/fileio.h"
#include "utils.h"


MemLayout loadLayoutFromSource(const std::vector<std::string>& inputFileNames, Parser& parser) {
    std::vector<SourceFile> sourceFiles;
    sourceFiles.reserve(inputFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : inputFileNames)
        sourceFiles.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceFiles);
    const MemLayout layout = parser.parse(program);

    return layout;
}


MemLayout loadLayoutFromBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.size() > 1)
        throw std::runtime_error("Only one binary file may be loaded in at a time");

    try {
        const std::vector<std::byte> binary = readFileBytes(inputFileNames[0]);
        const MemLayout layout = loadLayout(binary);
        return layout;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Failed to load binary file '" + inputFileNames[0] +
                                 "': " + e.what());
    }
}


bool isLoadingBinary(const std::vector<std::string>& inputFileNames) {
    if (inputFileNames.empty())
        return false;

    const std::string& firstName = inputFileNames[0];
    const size_t firstSize = firstName.size();
    return firstSize > 2 && firstName.substr(firstSize - 2, 2) == ".o";
}
