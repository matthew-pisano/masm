//
// Created by matthew on 5/22/25.
//


#include <catch2/catch_test_macros.hpp>

#include "instruction.h"
#include "interpreter.h"
#include "parser.h"
#include "testing_utilities.h"
#include "tokenizer.h"


TEST_CASE("Test add Instruction") {
    const RawFile rawFile = makeRawFile({"add $t0, $t1, $t2"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});

    SECTION("Test Tokenize add") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "add"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);

    SECTION("Test Parse add") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x2a, 0x40, 0x20});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    Interpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T1] = 1;
    interpreter.getState().registers[Register::T2] = 2;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute add") {
        constexpr int32_t expectedResult = 3;
        const int32_t actualResult = interpreter.getState().registers[Register::T0];
        REQUIRE(expectedResult == actualResult);
    }
}
