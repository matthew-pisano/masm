//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "../testing_utilities.h"
#include "exceptions.h"
#include "io/fileio.h"
#include "parser/directive.h"
#include "parser/parser.h"
#include "utils.h"


/**
 * Validates the memory layout of a parsed file against the loaded, expected memory
 * @param sourceFileNames The name sof the source files to parse
 * @param parsedFileName The name of the parsed file to compare against
 * @param useLittleEndian Whether to use little-endian byte order
 */
void validateMemLayout(const std::vector<std::string>& sourceFileNames,
                       const std::string& parsedFileName, const bool useLittleEndian = false) {
    const std::vector<std::byte> parsedBytes = readFileBytes(parsedFileName);

    std::vector memSecBytes = {
            static_cast<std::byte>(MemSection::DATA), static_cast<std::byte>(MemSection::TEXT),
            static_cast<std::byte>(MemSection::KDATA), static_cast<std::byte>(MemSection::KTEXT)};

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
        expectedMem.data[currSection].push_back(parsedBytes[i]);
    }

    std::vector<SourceFile> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : sourceFileNames)
        sourceLines.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> program = Tokenizer::tokenize(sourceLines);

    Parser parser(useLittleEndian);
    MemLayout actualMem = parser.parse(program);
    REQUIRE(expectedMem.data.size() == actualMem.data.size());
    for (const std::pair<const MemSection, std::vector<std::byte>>& pair : expectedMem.data) {
        REQUIRE(actualMem.data.contains(pair.first));
        REQUIRE(pair.second == actualMem.data[pair.first]);
    }
}


TEST_CASE("Test Directive Allocation") {
    SECTION("Test Align") {
        std::vector<std::byte> expected = {};
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "align"},
                                    {Token{TokenCategory::IMMEDIATE, "0"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0});
        actual = parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenCategory::IMMEDIATE, "1"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0, 0, 0});
        actual = parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenCategory::IMMEDIATE, "2"}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({0, 0, 0, 0, 0, 0, 0});
        actual = parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenCategory::IMMEDIATE, "3"}});
        REQUIRE(expected == actual);

        expected = {};
        actual = parseAllocDirective(8, Token{TokenCategory::ALLOC_DIRECTIVE, "align"},
                                     {Token{TokenCategory::IMMEDIATE, "3"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Ascii") {
        std::vector<std::byte> expected = {};
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "ascii"},
                                    {Token{TokenCategory::STRING, ""}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({'h', 'e', 'l', 'l', 'o'});
        actual = parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "ascii"},
                                     {Token{TokenCategory::STRING, "hello"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Asciiz") {
        std::vector<std::byte> expected = intVec2ByteVec({0});
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "asciiz"},
                                    {Token{TokenCategory::STRING, ""}});
        REQUIRE(expected == actual);

        expected = intVec2ByteVec({'h', 'e', 'l', 'l', 'o', 0});
        actual = parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "asciiz"},
                                     {Token{TokenCategory::STRING, "hello"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Byte") {
        std::vector<std::byte> expected = intVec2ByteVec({69});
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "byte"},
                                    {Token{TokenCategory::IMMEDIATE, "69"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Double") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0xbf, 0xf8, 0xa3, 0xd7, 0x0a, 0x3d, 0x70, 0xa4});
        std::vector<std::byte> actual =
                parseAllocDirective(5, Token{TokenCategory::ALLOC_DIRECTIVE, "double"},
                                    {Token{TokenCategory::IMMEDIATE, "-1.54"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Float") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0xbf, 0xc5, 0x1e, 0xb8});
        std::vector<std::byte> actual =
                parseAllocDirective(1, Token{TokenCategory::ALLOC_DIRECTIVE, "float"},
                                    {Token{TokenCategory::IMMEDIATE, "-1.54"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Half") {
        std::vector<std::byte> expected = intVec2ByteVec({0x00, 0x01, 0xa4});
        std::vector<std::byte> actual =
                parseAllocDirective(3, Token{TokenCategory::ALLOC_DIRECTIVE, "half"},
                                    {Token{TokenCategory::IMMEDIATE, "420"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Space") {
        std::vector<std::byte> expected = intVec2ByteVec({0, 0, 0, 0, 0, 0, 0, 0, 0});
        std::vector<std::byte> actual =
                parseAllocDirective(3, Token{TokenCategory::ALLOC_DIRECTIVE, "space"},
                                    {Token{TokenCategory::IMMEDIATE, "9"}});
        REQUIRE(expected == actual);
    }

    SECTION("Test Word") {
        std::vector<std::byte> expected =
                intVec2ByteVec({0x00, 0x00, 0x00, 0x00, 0xbc, 0x61, 0x4e});
        std::vector<std::byte> actual =
                parseAllocDirective(5, Token{TokenCategory::ALLOC_DIRECTIVE, "word"},
                                    {Token{TokenCategory::IMMEDIATE, "12345678"}});
        REQUIRE(expected == actual);
    }
}


TEST_CASE("Test Parser Syntax Errors") {
    SECTION("Test Unknown Label Reference") {
        Parser parser{};
        const std::vector<LineTokens> program = {
                {"test.asm",
                 1,
                 {{TokenCategory::INSTRUCTION, "j"}, {TokenCategory::LABEL_REF, "ref"}}}};
        REQUIRE_THROWS_MATCHES(
                parser.parse(program), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at test.asm:1 -> Unknown label 'ref'"));
    }

    SECTION("Test Unknown Duplicate Label") {
        Parser parser{};
        const std::vector<LineTokens> program = {
                {"test.asm", 1, {{TokenCategory::LABEL_DEF, "label"}}},
                {"test.asm", 1, {{TokenCategory::LABEL_DEF, "label"}}}};
        REQUIRE_THROWS_MATCHES(
                parser.parse(program), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at test.asm:1 -> Duplicate label 'label'"));
    }

    SECTION("Test Empty Allocation Directive") {
        Parser parser{};
        const std::vector<LineTokens> program = {
                {"test.asm", 1, {{TokenCategory::ALLOC_DIRECTIVE, "word"}}}};
        REQUIRE_THROWS_MATCHES(parser.parse(program), MasmSyntaxError,
                               Catch::Matchers::Message("Syntax error at test.asm:1 -> Directive "
                                                        "'word' expects at least one argument"));
    }

    SECTION("Test Unsupported Directive") {
        Parser parser{};
        const std::vector<LineTokens> program = {
                {"test.asm",
                 1,
                 {{TokenCategory::ALLOC_DIRECTIVE, "eeby"}, {TokenCategory::IMMEDIATE, "1"}}}};
        REQUIRE_THROWS_MATCHES(
                parser.parse(program), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at test.asm:1 -> Unsupported directive 'eeby'"));
    }
}


TEST_CASE("Test Word Allocation from Label") {
    Parser parser{};
    const std::vector<LineTokens> program = {
            {"test.asm", 1, {{TokenCategory::SEC_DIRECTIVE, "data"}}},
            {"test.asm", 2, {{TokenCategory::LABEL_DEF, "label"}}},
            {"test.asm",
             2,
             {{TokenCategory::ALLOC_DIRECTIVE, "word"}, {TokenCategory::IMMEDIATE, "0"}}},
            {"test.asm",
             3,
             {{TokenCategory::ALLOC_DIRECTIVE, "word"}, {TokenCategory::LABEL_REF, "label"}}}};

    MemLayout layout = parser.parse(program);
    REQUIRE(layout.data[MemSection::DATA].size() == 8);
    std::vector<std::byte> expected =
            intVec2ByteVec({0x00, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x00});
    REQUIRE(expected == layout.data[MemSection::DATA]);
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


TEST_CASE("Test Parse Syscall Input Output") {
    const std::string test_case = "input_output";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Parse MMIO Input Output") {
    const std::string test_case = "mmio";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}


TEST_CASE("Test Parse MMIO Input Output Little Endian") {
    const std::string test_case = "mmio_le";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse", true);
}


TEST_CASE("Test Parse Echo Interrupt") {
    const std::string test_case = "echointer";
    validateMemLayout({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                      "test/fixtures/" + test_case + "/" + test_case + ".pse");
}
