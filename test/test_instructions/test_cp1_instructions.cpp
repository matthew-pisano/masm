//
// Created by matthew on 6/6/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "../testing_utilities.h"
#include "exceptions.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test FP Double Invalid Register Read") {
    const SourceFile rawFile = makeRawFile({"abs.d $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "abs.d"},
                                                                 {TokenCategory::REGISTER, "f0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    REQUIRE_THROWS_MATCHES(interpreter.interpret(actualLayout), MasmRuntimeError,
                           Catch::Matchers::Message("Runtime error at 0x00400000 (a.asm:1) -> "
                                                    "Invalid double precision register: f1"));
}


TEST_CASE("Test FP Double Invalid Register Write") {
    const SourceFile rawFile = makeRawFile({"abs.d $f1, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "abs.d"},
                                                                 {TokenCategory::REGISTER, "f1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    REQUIRE_THROWS_MATCHES(interpreter.interpret(actualLayout), MasmRuntimeError,
                           Catch::Matchers::Message("Runtime error at 0x00400000 (a.asm:1) -> "
                                                    "Invalid double precision register: f1"));
}


TEST_CASE("Test FP Abs.s Instruction") {
    const SourceFile rawFile = makeRawFile({"abs.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "abs.s"},
                                                                 {TokenCategory::REGISTER, "f0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f1"}}};
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
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "abs.d"},
                                                                 {TokenCategory::REGISTER, "f0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x46, 0x20, 0x10, 0x05});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float64_t expected = 42e69;
    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().cp1.setDouble(Coproc1Register::F2, expected * -1);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Add.s Instruction") {
    const SourceFile rawFile = makeRawFile({"add.s $f0, $f1, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "add.s"},
                                                                 {TokenCategory::REGISTER, "f0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x46, 0x02, 0x08, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, 10);
    interpreter.getState().cp1.setFloat(Coproc1Register::F2, 20);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 30;
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Add.d Instruction") {
    const SourceFile rawFile = makeRawFile({"add.d $f0, $f2, $f4"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "add.d"},
                                                                 {TokenCategory::REGISTER, "f0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f2"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "f4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x46, 0x24, 0x10, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{IOMode::SYSCALL, std::cin, std::cout};
    interpreter.getState().cp1.setDouble(Coproc1Register::F2, 10);
    interpreter.getState().cp1.setDouble(Coproc1Register::F4, 20);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 30;
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}
