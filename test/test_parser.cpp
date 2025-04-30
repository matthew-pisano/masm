//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>

#include "fileio.h"
#include "parser.h"


/**
 * Validates the memory layout of a parsed file against the loaded, expected memory
 * @param sourceFileNames The name sof the source files to parse
 * @param parsedFileName The name of the parsed file to compare against
 */
void validateMemLayout(const std::vector<std::string>& sourceFileNames,
                       const std::string& parsedFileName) {
    const std::vector<std::byte> parsedBytes = readFileBytes(parsedFileName);

    std::vector memSecBytes = {static_cast<std::byte>(MemSection::DATA),
                               static_cast<std::byte>(MemSection::TEXT)};

    MemLayout expectedMem;
    MemSection currSection;
    for (size_t i = 0; i < parsedBytes.size(); ++i) {
        constexpr std::byte groupSep{0x1d};
        if (i < parsedBytes.size() - 1 && parsedBytes[i] == groupSep &&
            std::ranges::find(memSecBytes, parsedBytes[i + 1]) != memSecBytes.end()) {
            currSection = static_cast<MemSection>(parsedBytes[i + 1]);
            i += 1;
            continue;
        }
        expectedMem[currSection].push_back(parsedBytes[i]);
    }

    std::vector<std::vector<std::string>> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& sourceFileName : sourceFileNames)
        sourceLines.push_back(readFileLines(sourceFileName));

    Tokenizer tokenizer{};
    const std::vector<std::vector<Token>> program = tokenizer.tokenize(sourceLines);
    Parser parser{};
    MemLayout actualMem = parser.parse(program);
    REQUIRE(expectedMem.size() == actualMem.size());
    for (const std::pair<const MemSection, std::vector<std::byte>>& pair : expectedMem) {
        REQUIRE(actualMem.contains(pair.first));

        REQUIRE(pair.second == actualMem[pair.first]);
    }
}


TEST_CASE("Test Parse Hello World") {
    const std::string test_case = "hello_world";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Parse Load Address") {
    const std::string test_case = "load_address";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Parse Arithmetic") {
    const std::string test_case = "arithmetic";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Parse Syscall") {
    const std::string test_case = "syscall";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}

TEST_CASE("Test Parse Loops") {
    const std::string test_case = "loops";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}

TEST_CASE("Test Parse Globals") {
    const std::string test_case = "globals";
    validateMemLayout({"test/fixtures/" + test_case + "/globalsOne.asm",
                       "test/fixtures/" + test_case + "/globalsTwo.asm"},
                      "test/fixtures/" + test_case + "/globalsOne.pse");
}
