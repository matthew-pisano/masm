//
// Created by matthew on 6/3/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "debug/debug_interpreter.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test Eret Instruction") {
    const SourceFile rawFile = makeRawFile({"eret"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "eret"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x42, 0x00, 0x00, 0x18});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr int32_t expectedPC = 0x00400004;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp0[Coproc0Register::EPC] = expectedPC;
    interpreter.getState().cp0[Coproc0Register::CAUSE] = 1;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expectedPC == interpreter.getState().registers[Register::PC]);
        REQUIRE(0 == interpreter.getState().cp0[Coproc0Register::EPC]);
        REQUIRE(0 == interpreter.getState().cp0[Coproc0Register::CAUSE]);
    }
}


TEST_CASE("Test Mtc0 Instruction") {
    const SourceFile rawFile = makeRawFile({"mtc0 $t1, $8"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "mtc0"},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "8"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x40, 0x89, 0x40, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().registers[Register::T1] = 1444;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") { REQUIRE(1444 == interpreter.getState().cp0[Coproc0Register::VADDR]); }
}


TEST_CASE("Test Mfc0 Instruction") {
    const SourceFile rawFile = makeRawFile({"mfc0 $t1, $8"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "mfc0"},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "8"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x40, 0x09, 0x40, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp0[Coproc0Register::VADDR] = 1444;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") { REQUIRE(1444 == interpreter.getState().registers[Register::T1]); }
}
