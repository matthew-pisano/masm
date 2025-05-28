//
// Created by matthew on 5/22/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "interpreter/interpreter.h"
#include "parser/instruction.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test add Instruction") {
    const RawFile rawFile = makeRawFile({"add $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "add"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x20});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = -1;
    interpreter.getState().registers[Register::T2] = 2;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 1;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test addu Instruction") {
    const RawFile rawFile = makeRawFile({"addu $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addu"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x21});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = -1;
    interpreter.getState().registers[Register::T2] = 2;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 1;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test addi Instruction") {
    const RawFile rawFile = makeRawFile({"addi $t0, $t1, -2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "-2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x21, 0x28, 0xff, 0xfe});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = -1;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = -3;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test addiu Instruction") {
    const RawFile rawFile = makeRawFile({"addiu $t0, $t1, -2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addiu"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "-2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x25, 0x28, 0xff, 0xfe});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = -1;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = -3;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test and Instruction") {
    const RawFile rawFile = makeRawFile({"and $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "and"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x24});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x0F0F0F0F;
    interpreter.getState().registers[Register::T2] = 0xF0F0F0F0;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00000000;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test andi Instruction") {
    const RawFile rawFile = makeRawFile({"andi $t0, $t1, 0x00FF"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "andi"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "255"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x31, 0x28, 0x00, 0xff});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00000078;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test div Instruction") {
    const RawFile rawFile = makeRawFile({"div $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "div"},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x00, 0x1a});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 17;
    interpreter.getState().registers[Register::T2] = 5;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        // 17 / 5 = 3 remainder 2
        constexpr int32_t expectedLo = 3; // Quotient
        constexpr int32_t expectedHi = 2; // Remainder
        const int32_t actualLo = interpreter.getState().registers[Register::LO];
        const int32_t actualHi = interpreter.getState().registers[Register::HI];
        REQUIRE(expectedLo == actualLo);
        REQUIRE(expectedHi == actualHi);
    }
}


TEST_CASE("Test divu Instruction") {
    const RawFile rawFile = makeRawFile({"divu $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "divu"},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x00, 0x1b});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0xFFFFFFFF; // Large unsigned
    interpreter.getState().registers[Register::T2] = 3;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        // 0xFFFFFFFF / 3 = 0x55555555, remainder 0
        constexpr int32_t expectedLo = 0x55555555; // Quotient
        constexpr int32_t expectedHi = 0; // Remainder
        const int32_t actualLo = interpreter.getState().registers[Register::LO];
        const int32_t actualHi = interpreter.getState().registers[Register::HI];
        REQUIRE(expectedLo == actualLo);
        REQUIRE(expectedHi == actualHi);
    }
}


TEST_CASE("Test mfhi Instruction") {
    const RawFile rawFile = makeRawFile({"mfhi $t0"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "mfhi"}, {TokenType::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x00, 0x40, 0x10});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::HI] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x12345678;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test mflo Instruction") {
    const RawFile rawFile = makeRawFile({"mflo $t0"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "mflo"}, {TokenType::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x00, 0x40, 0x12});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::LO] = 0x87654321;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x87654321;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test mthi Instruction") {
    const RawFile rawFile = makeRawFile({"mthi $t1"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "mthi"}, {TokenType::REGISTER, "t1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x20, 0x00, 0x11});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x12345678;
        const int32_t actualResult = interpreter.getState().registers[Register::HI];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test mtlo Instruction") {
    const RawFile rawFile = makeRawFile({"mtlo $t1"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "mtlo"}, {TokenType::REGISTER, "t1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x20, 0x00, 0x13});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x87654321;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x87654321;
        const int32_t actualResult = interpreter.getState().registers[Register::LO];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test mult Instruction") {
    const RawFile rawFile = makeRawFile({"mult $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "mult"},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x00, 0x18});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.getState().registers[Register::T2] = 2;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        // Result should be in HI:LO registers
        // 0x12345678 * 2 = 0x2468ACF0 (fits in 32 bits, so HI should be 0)
        constexpr int32_t expectedLo = 0x2468ACF0;
        constexpr int32_t expectedHi = 0;
        const int32_t actualLo = interpreter.getState().registers[Register::LO];
        const int32_t actualHi = interpreter.getState().registers[Register::HI];
        REQUIRE(expectedLo == actualLo);
        REQUIRE(expectedHi == actualHi);
    }
}


TEST_CASE("Test multu Instruction") {
    const RawFile rawFile = makeRawFile({"multu $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "multu"},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x00, 0x19});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0xFFFFFFFF; // Large unsigned
    interpreter.getState().registers[Register::T2] = 2;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        // 0xFFFFFFFF * 2 = 0x1FFFFFFFE (requires both HI and LO)
        constexpr int32_t expectedLo = 0xFFFFFFFE;
        constexpr int32_t expectedHi = 1;
        const int32_t actualLo = interpreter.getState().registers[Register::LO];
        const int32_t actualHi = interpreter.getState().registers[Register::HI];
        REQUIRE(expectedLo == actualLo);
        REQUIRE(expectedHi == actualHi);
    }
}


TEST_CASE("Test nor Instruction") {
    const RawFile rawFile = makeRawFile({"nor $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "nor"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x27});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x0F0F0F0F;
    interpreter.getState().registers[Register::T2] = 0xF0F0F0F0;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x00000000;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test or Instruction") {
    const RawFile rawFile = makeRawFile({"or $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "or"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x25});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x0F0F0F0F;
    interpreter.getState().registers[Register::T2] = 0xF0F0F0F0;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xFFFFFFFF;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test ori Instruction") {
    const RawFile rawFile = makeRawFile({"ori $t0, $t1, 0x00FF"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "ori"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "255"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x35, 0x28, 0x00, 0xff});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345600;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x123456FF;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sll Instruction") {
    const RawFile rawFile = makeRawFile({"sll $t0, $t1, 4"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sll"},
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
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x09, 0x41, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x23456780;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test srl Instruction") {
    const RawFile rawFile = makeRawFile({"srl $t0, $t1, 4"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "srl"},
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
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x09, 0x41, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x01234567;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sra Instruction") {
    const RawFile rawFile = makeRawFile({"sra $t0, $t1, 4"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sra"},
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
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x00, 0x09, 0x41, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x80000000; // Negative number
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xF8000000; // Sign extended
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sllv Instruction") {
    const RawFile rawFile = makeRawFile({"sllv $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sllv"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x49, 0x40, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.getState().registers[Register::T2] = 4;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x23456780;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test srlv Instruction") {
    const RawFile rawFile = makeRawFile({"srlv $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "srlv"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x49, 0x40, 0x06});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12345678;
    interpreter.getState().registers[Register::T2] = 4;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x01234567;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test srav Instruction") {
    const RawFile rawFile = makeRawFile({"srav $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "srav"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x49, 0x40, 0x07});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x80000000; // Negative number
    interpreter.getState().registers[Register::T2] = 4;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xF8000000; // Sign extended
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test sub Instruction") {
    const RawFile rawFile = makeRawFile({"sub $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "sub"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x22});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 5;
    interpreter.getState().registers[Register::T2] = 3;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 2;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test subu Instruction") {
    const RawFile rawFile = makeRawFile({"subu $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "subu"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x23});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 5;
    interpreter.getState().registers[Register::T2] = 3;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 2;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test xor Instruction") {
    const RawFile rawFile = makeRawFile({"xor $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "xor"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x26});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0xAAAAAAAA;
    interpreter.getState().registers[Register::T2] = 0x55555555;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0xFFFFFFFF;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}


TEST_CASE("Test xori Instruction") {
    const RawFile rawFile = makeRawFile({"xori $t0, $t1, 0xFFFF"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "xori"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::IMMEDIATE, "65535"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x39, 0x28, 0xff, 0xff});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().registers[Register::T1] = 0x12340000;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr int32_t expectedResult = 0x1234FFFF;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}
