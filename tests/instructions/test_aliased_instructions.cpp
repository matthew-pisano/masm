//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include <masm/assembler/parser.hpp>
#include <masm/assembler/tokenizer.hpp>
#include <masm/interpreter/interpreter.hpp>

#include "tests/testing_utilities.hpp"


// No execution tests are needed as the aliased instructions are made of other, tested instructions


TEST_CASE("Test lw Instruction Alias") {
    const SourceFile rawFile = makeRawFile({"lw $t0, 0xffff0000"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4294901760"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x3c, 0x01, 0xff, 0xff, 0x8c, 0x28, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test lw Instruction Alias Labeled") {
    const SourceFile rawFile = makeRawFile({"lw $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0xffff0000;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x3c, 0x01, 0xff, 0xff, 0x8c, 0x28, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test sw Instruction Alias") {
    const SourceFile rawFile = makeRawFile({"sw $t0, 0xffff0000"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "sw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4294901760"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x3c, 0x01, 0xff, 0xff, 0xac, 0x28, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test sw Instruction Alias Labeled") {
    const SourceFile rawFile = makeRawFile({"sw $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "sw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0xffff0000;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x3c, 0x01, 0xff, 0xff, 0xac, 0x28, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test div Instruction Three Registers") {
    const SourceFile rawFile = makeRawFile({"div $t0, $t1, $t2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{
                {TokenCategory::INSTRUCTION, "div"},
                {TokenCategory::REGISTER, "t0"},
                {TokenCategory::SEPERATOR, ","},
                {TokenCategory::REGISTER, "t1"},
                {TokenCategory::SEPERATOR, ","},
                {TokenCategory::REGISTER, "t2"},
        }};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x2a, 0x00, 0x1a, 0x00, 0x00, 0x40, 0x12});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test divu Instruction Three Registers") {
    const SourceFile rawFile = makeRawFile({"divu $t0, $t1, $t2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{
                {TokenCategory::INSTRUCTION, "divu"},
                {TokenCategory::REGISTER, "t0"},
                {TokenCategory::SEPERATOR, ","},
                {TokenCategory::REGISTER, "t1"},
                {TokenCategory::SEPERATOR, ","},
                {TokenCategory::REGISTER, "t2"},
        }};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x2a, 0x00, 0x1b, 0x00, 0x00, 0x40, 0x12});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }
}
