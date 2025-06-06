//
// Created by matthew on 6/6/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test FP Abs.s Instruction") {
    const SourceFile rawFile = makeRawFile({"abs.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "abs.s"},
                                                                 {TokenType::REGISTER, "f0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x46, 0x00, 0x08, 0x05});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float32_t expected = 42.69;
    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, expected * -1);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Abs.d Instruction") {
    const SourceFile rawFile = makeRawFile({"abs.d $f0, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "abs.d"},
                                                                 {TokenType::REGISTER, "f0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x46, 0x20, 0x10, 0x05});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float64_t expected = 42.69;
    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().cp1.setDouble(Coproc1Register::F2, expected * -1);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}
