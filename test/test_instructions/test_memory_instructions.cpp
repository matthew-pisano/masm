//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "tokenizer/postprocessor.h"
#include "tokenizer/tokenizer.h"
#include "../testing_utilities.h"


TEST_CASE("Test lb Instruction") {
    const RawFile rawFile = makeRawFile({"lb $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lb"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x81, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().memory.byteTo(0x10010004, 0x82348687);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xffffff87;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test lbu Instruction") {
    const RawFile rawFile = makeRawFile({"lbu $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lbu"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x91, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().memory.byteTo(0x10010004, 0x82348687);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00000087;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test lh Instruction") {
    const RawFile rawFile = makeRawFile({"lh $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lh"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x85, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().memory.halfTo(0x10010004, 0x82348687);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xffff8687;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test lhu Instruction") {
    const RawFile rawFile = makeRawFile({"lhu $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lhu"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x95, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().memory.halfTo(0x10010004, 0x82348687);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00008687;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test lw Instruction") {
    const RawFile rawFile = makeRawFile({"lw $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x8d, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().memory.wordTo(0x10010004, 0x82348687);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x82348687;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test lui Instruction") {
    const RawFile rawFile = makeRawFile({"lui $t0, 100"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lui"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "100"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x3c, 0x08, 0x00, 0x64});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00640000;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sb Instruction") {
    const RawFile rawFile = makeRawFile({"sb $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sb"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xa1, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.getState().registers[Register::T0] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x78;
        const int32_t actualResult = interpreter.getState().memory.byteAt(0x10010004);
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sh Instruction") {
    const RawFile rawFile = makeRawFile({"sh $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sh"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xa5, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.getState().registers[Register::T0] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x5678;
        const int32_t actualResult = interpreter.getState().memory.halfAt(0x10010004);
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sw Instruction") {
    const RawFile rawFile = makeRawFile({"sw $t0, 4($t1)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sw"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xad, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.getState().registers[Register::T0] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x12345678;
        const int32_t actualResult = interpreter.getState().memory.wordAt(0x10010004);
        REQUIRE(expectedResult == actualResult);
    }
}
