//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "debug/debug_interpreter.h"
#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "tokenizer/postprocessor.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test lb Instruction") {
    const SourceFile rawFile = makeRawFile({"lb $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lb"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x81, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"lbu $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lbu"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x91, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"lh $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lh"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x85, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"lhu $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lhu"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x95, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"lw $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x8d, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"lui $t0, 100"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lui"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "100"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x3c, 0x08, 0x00, 0x64});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00640000;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sb Instruction") {
    const SourceFile rawFile = makeRawFile({"sb $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "sb"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xa1, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"sh $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "sh"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xa5, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
    const SourceFile rawFile = makeRawFile({"sw $t0, 4($t1)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "sw"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::IMMEDIATE, "4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0xad, 0x28, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = 0x10010000;
    interpreter.getState().registers[Register::T0] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x12345678;
        const int32_t actualResult = interpreter.getState().memory.wordAt(0x10010004);
        REQUIRE(expectedResult == actualResult);
    }
}
