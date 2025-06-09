//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test slt Instruction") {
    const SourceFile rawFile = makeRawFile({"slt $t0, $t1, $t2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "slt"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x2a});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = -5;
    interpreter.getState().registers[Register::T2] = 3;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 1; // -5 < 3 is true
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sltu Instruction") {
    const SourceFile rawFile = makeRawFile({"sltu $t0, $t1, $t2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "sltu"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x29});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = -5; // Interpreted as large positive number
    interpreter.getState().registers[Register::T2] = 3;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0; // -5 < 3 unsigned is false
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test slti Instruction") {
    const SourceFile rawFile = makeRawFile({"slti $t0, $t1, 10"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "slti"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "10"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x29, 0x28, 0x00, 0x0a});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = 5;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 1; // 5 < 10 is true
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sltiu Instruction") {
    const SourceFile rawFile = makeRawFile({"sltiu $t0, $t1, 10"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "sltiu"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "10"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x2d, 0x28, 0x00, 0x0a});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = -5;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0; // -5 < 10 unsigned is false
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}
