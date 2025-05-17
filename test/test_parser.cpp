//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>

#include "directive.h"
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
    const std::vector<SourceLine> program = Tokenizer::tokenize(sourceLines);
    std::vector<std::vector<Token>> vecProgram;
    vecProgram.reserve(program.size());
    for (const SourceLine& line : program) {
        vecProgram.push_back(line.tokens);
    }

    Parser parser{};
    MemLayout actualMem = parser.parse(vecProgram);
    REQUIRE(expectedMem.size() == actualMem.size());
    for (const std::pair<const MemSection, std::vector<std::byte>>& pair : expectedMem) {
        REQUIRE(actualMem.contains(pair.first));

        REQUIRE(pair.second == actualMem[pair.first]);
    }
}


/**
 * Converts a vector of integers to a vector of bytes
 * @param intVec The vector of integers to convert
 * @return The vector of bytes
 */
std::vector<std::byte> intVec2ByteVec(const std::vector<int>& intVec) {
    std::vector<std::byte> byteVec(intVec.size());
    for (size_t i = 0; i < intVec.size(); ++i)
        byteVec[i] = static_cast<std::byte>(intVec[i]);
    return byteVec;
}


TEST_CASE("Test Directive Allocation") {
    SECTION("Test Align") {
        std::vector<std::byte> expected = {};
        std::vector<std::byte> actual = parseAllocDirective(
                1, Token{TokenType::ALLOC_DIRECTIVE, "align"}, {Token{TokenType::IMMEDIATE, "0"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0});
        actual = parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenType::IMMEDIATE, "1"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0, 0, 0});
        actual = parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenType::IMMEDIATE, "2"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0, 0, 0, 0, 0, 0, 0});
        actual = parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenType::IMMEDIATE, "3"}});
        REQUIRE(expected == actual);

        expected = {};
        actual = parseAllocDirective(8, Token{TokenType::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenType::IMMEDIATE, "3"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Ascii") {
        std::vector<std::byte> expected = {};
        std::vector<std::byte> actual = parseAllocDirective(
                1, Token{TokenType::ALLOC_DIRECTIVE, "ascii"}, {Token{TokenType::STRING, ""}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({'h', 'e', 'l', 'l', 'o'});
        actual = parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "ascii"},
                                     {Token{TokenType::STRING, "hello"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Asciiz") {
        std::vector<std::byte> expected = intVec2ByteVec({0});
        std::vector<std::byte> actual = parseAllocDirective(
                1, Token{TokenType::ALLOC_DIRECTIVE, "asciiz"}, {Token{TokenType::STRING, ""}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({'h', 'e', 'l', 'l', 'o', 0});
        actual = parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "asciiz"},
                                     {Token{TokenType::STRING, "hello"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Byte") {
        std::vector<std::byte> expected = intVec2ByteVec({69});
        std::vector<std::byte> actual = parseAllocDirective(
                1, Token{TokenType::ALLOC_DIRECTIVE, "byte"}, {Token{TokenType::IMMEDIATE, "69"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Double") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0xbf, 0xf8, 0xa3, 0xd7, 0x0a, 0x3d, 0x70, 0xa4});
        std::vector<std::byte> actual =
                parseAllocDirective(5, Token{TokenType::ALLOC_DIRECTIVE, "double"},
                                    {Token{TokenType::IMMEDIATE, "-1.54"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Float") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0xbf, 0xc5, 0x1e, 0xb8});
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenType::ALLOC_DIRECTIVE, "float"},
                                    {Token{TokenType::IMMEDIATE, "-1.54"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Half") {
        std::vector<std::byte> expected = intVec2ByteVec({0x00, 0x01, 0xa4});
        std::vector<std::byte> actual = parseAllocDirective(
                3, Token{TokenType::ALLOC_DIRECTIVE, "half"}, {Token{TokenType::IMMEDIATE, "420"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Space") {
        std::vector<std::byte> expected = intVec2ByteVec({0, 0, 0, 0, 0, 0, 0, 0, 0});
        std::vector<std::byte> actual = parseAllocDirective(
                3, Token{TokenType::ALLOC_DIRECTIVE, "space"}, {Token{TokenType::IMMEDIATE, "9"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Word") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0x00, 0xbc, 0x61, 0x4e});
        std::vector<std::byte> actual =
                parseAllocDirective(5, Token{TokenType::ALLOC_DIRECTIVE, "word"},
                                    {Token{TokenType::IMMEDIATE, "12345678"}});
        REQUIRE(expected == actual);
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
