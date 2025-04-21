//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>

#include "fileio.h"
#include "parser.h"


/**
 * Validates the memory layout of a parsed file against the loaded, expected memory
 * @param sourceFileName The name of the source file to parse
 * @param parsedFileName The name of the parsed file to compare against
 */
void validateMemLayout(const std::string& sourceFileName, const std::string& parsedFileName) {
    const std::vector<unsigned char> parsedBytes = readFileBytes(parsedFileName);

    MemLayout expectedMem;
    MemSection currSection;
    for (size_t i = 0; i < parsedBytes.size(); ++i) {
        constexpr unsigned char groupSep = 0x1d;
        if (i < parsedBytes.size() - 1 && parsedBytes[i] == groupSep &&
            (parsedBytes[i + 1] == 0 || parsedBytes[i + 1] == 1)) {
            currSection = static_cast<MemSection>(parsedBytes[i + 1]);
            i += 1;
            continue;
        }
        expectedMem[currSection].push_back(parsedBytes[i]);
    }

    const std::vector<std::string> sourceLines = readFileLines(sourceFileName);
    const std::vector<std::vector<Token>> program = Tokenizer::tokenize(sourceLines);
    Parser parser{};
    MemLayout actualMem = parser.parse(program);
    REQUIRE(expectedMem.size() == actualMem.size());
    for (const std::pair<const MemSection, std::vector<unsigned char>>& pair : expectedMem) {
        REQUIRE(actualMem.contains(pair.first));

        REQUIRE(pair.second == actualMem[pair.first]);
    }
}


TEST_CASE("Test Hello World") {
    const std::string test_case = "hello_world";
    validateMemLayout("test/fixtures/" + test_case + "/" + test_case + ".asm",
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Load Address") {
    const std::string test_case = "load_address";
    validateMemLayout("test/fixtures/" + test_case + "/" + test_case + ".asm",
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Arithmetic") {
    const std::string test_case = "arithmetic";
    validateMemLayout("test/fixtures/" + test_case + "/" + test_case + ".asm",
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Syscall") {
    const std::string test_case = "syscall";
    validateMemLayout("test/fixtures/" + test_case + "/" + test_case + ".asm",
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}

TEST_CASE("Test Loops") {
    const std::string test_case = "loops";
    validateMemLayout("test/fixtures/" + test_case + "/" + test_case + ".asm",
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}
