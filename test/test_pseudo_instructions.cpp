//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "interpreter.h"
#include "parser.h"
#include "testing_utilities.h"
#include "tokenizer.h"


// No execution tests are needed as the pseudo instructions are made of other, tested instructions


TEST_CASE("Test li Instruction") {
    const RawFile rawFile = makeRawFile({"li $t0, 100"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "100"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x24, 0x08, 0x00, 0x64});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test la Instruction") {
    const RawFile rawFile = makeRawFile({"la $t0, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "la"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    parser.getLabels().getLabelMap()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                intVec2ByteVec({0x3c, 0x01, 0x00, 0x40, 0x34, 0x28, 0x00, 0x10});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test move Instruction") {
    const RawFile rawFile = makeRawFile({"move $t0, $t1"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "move"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x09, 0x40, 0x21});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test nop Instruction") {
    const RawFile rawFile = makeRawFile({"nop"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "nop"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x00, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}
