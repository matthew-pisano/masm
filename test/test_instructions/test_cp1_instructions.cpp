//
// Created by matthew on 6/6/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "../testing_utilities.h"
#include "debug/debug_interpreter.h"
#include "exceptions.h"
#include "interpreter/interpreter.h"
#include "interpreter/memory.h"
#include "tokenizer/postprocessor.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test FP Double Invalid Register Read") {
    const SourceFile rawFile = makeRawFile({"abs.d $f0, $f1"});
    const std::vector<LineTokens> tokens = Tokenizer::tokenizeFile({rawFile});

    Parser parser;
    const MemLayout actualLayout = parser.parse(tokens, true);

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    REQUIRE_THROWS_MATCHES(interpreter.interpret(actualLayout), MasmRuntimeError,
                           Catch::Matchers::Message("Runtime error at 0x00400000 (a.asm:1) -> "
                                                    "Invalid double precision register: f1"));
}


TEST_CASE("Test FP Double Invalid Register Write") {
    const SourceFile rawFile = makeRawFile({"abs.d $f1, $f2"});
    const std::vector<LineTokens> tokens = Tokenizer::tokenizeFile({rawFile});

    Parser parser;
    const MemLayout actualLayout = parser.parse(tokens, true);

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    REQUIRE_THROWS_MATCHES(interpreter.interpret(actualLayout), MasmRuntimeError,
                           Catch::Matchers::Message("Runtime error at 0x00400000 (a.asm:1) -> "
                                                    "Invalid double precision register: f1"));
}


TEST_CASE("Test FP Abs.s Instruction") {
    const SourceFile rawFile = makeRawFile({"abs.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "abs.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x00, 0x08, 0x05});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float32_t expected = 42.69;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "abs.d"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x20, 0x10, 0x05});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float64_t expected = 42e69;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "add.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x02, 0x08, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
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
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "add.d"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f4"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x24, 0x10, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setDouble(Coproc1Register::F2, 10);
    interpreter.getState().cp1.setDouble(Coproc1Register::F4, 20);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 30;
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Div.s Instruction") {
    const SourceFile rawFile = makeRawFile({"div.s $f0, $f1, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "div.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x02, 0x08, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, 10);
    interpreter.getState().cp1.setFloat(Coproc1Register::F2, 5);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 2;
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}

TEST_CASE("Test FP Mul.s Instruction") {
    const SourceFile rawFile = makeRawFile({"mul.s $f0, $f1, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "mul.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x02, 0x08, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, 10);
    interpreter.getState().cp1.setFloat(Coproc1Register::F2, 5);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 50;
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Neg.s Instruction") {
    const SourceFile rawFile = makeRawFile({"neg.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "neg.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x00, 0x08, 0x07});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float32_t expected = -10;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, -expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Sqrt.s Instruction") {
    const SourceFile rawFile = makeRawFile({"sqrt.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "sqrt.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x00, 0x08, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float32_t expected = 5;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, expected * expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP Sub.s Instruction") {
    const SourceFile rawFile = makeRawFile({"sub.s $f0, $f1, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "sub.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x02, 0x08, 0x01});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, 20);
    interpreter.getState().cp1.setFloat(Coproc1Register::F2, 10);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        constexpr float32_t expected = 10;
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP c.eq.s Instruction") {
    const SourceFile rawFile = makeRawFile({"c.eq.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "c.eq.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x01, 0x00, 0x32});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr float32_t testValue = 42.69;
    SECTION("Test Execute Equal") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue);
        interpreter.interpret(actualLayout);
        REQUIRE(interpreter.getState().cp1.getFlag(0));
    }

    SECTION("Test Execute Not Equal") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue + 0.01);
        interpreter.interpret(actualLayout);
        REQUIRE_FALSE(interpreter.getState().cp1.getFlag(0));
    }
}


TEST_CASE("Test FP c.lt.s Instruction") {
    const SourceFile rawFile = makeRawFile({"c.lt.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "c.lt.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x01, 0x00, 0x3c});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr float32_t testValue = 42.69;
    SECTION("Test Execute Less Than") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue * 2);
        interpreter.interpret(actualLayout);
        REQUIRE(interpreter.getState().cp1.getFlag(0));
    }

    SECTION("Test Execute Not Less Than") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue * 2);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue);
        interpreter.interpret(actualLayout);
        REQUIRE_FALSE(interpreter.getState().cp1.getFlag(0));
    }
}


TEST_CASE("Test FP c.le.s Instruction") {
    const SourceFile rawFile = makeRawFile({"c.le.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "c.le.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x01, 0x00, 0x3e});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr float32_t testValue = 42.69;
    SECTION("Test Execute Less Than or Equal") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue * 2);
        interpreter.interpret(actualLayout);
        REQUIRE(interpreter.getState().cp1.getFlag(0));
    }

    SECTION("Test Execute Equal") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue);
        interpreter.interpret(actualLayout);
        REQUIRE(interpreter.getState().cp1.getFlag(0));
    }

    SECTION("Test Execute Not Less Than or Equal") {
        interpreter.getState().cp1.setFloat(Coproc1Register::F0, testValue * 2);
        interpreter.getState().cp1.setFloat(Coproc1Register::F1, testValue);
        interpreter.interpret(actualLayout);
        REQUIRE_FALSE(interpreter.getState().cp1.getFlag(0));
    }
}


TEST_CASE("Test FP bc1f Instruction") {
    const SourceFile rawFile = makeRawFile({"bc1f label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bc1f"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x45, 0x00, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().cp1.setFlag(0, true);
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Flag True") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400004);
    }

    DebugInterpreter interpreterNe(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().cp1.setFlag(0, false);
    interpreterNe.interpret(actualLayout);
    SECTION("Test Execute Flag False") {
        REQUIRE(interpreterNe.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test FP bc1t Instruction") {
    const SourceFile rawFile = makeRawFile({"bc1t label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bc1t"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x45, 0x01, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().cp1.setFlag(0, true);
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Flag True") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterNe(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().cp1.setFlag(0, false);
    interpreterNe.interpret(actualLayout);
    SECTION("Test Execute Flag False") {
        REQUIRE(interpreterNe.getState().registers[Register::PC] == 0x00400004);
    }
}


TEST_CASE("Test FP cvt.d.s Instruction") {
    const SourceFile rawFile = makeRawFile({"cvt.d.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "cvt.d.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x00, 0x08, 0x21});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float32_t expected = 55.5;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP cvt.s.d Instruction") {
    const SourceFile rawFile = makeRawFile({"cvt.s.d $f0, $f2"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "cvt.s.d"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f2"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x20, 0x10, 0x20});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    constexpr float64_t expected = 55.5;
    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    interpreter.getState().cp1.setDouble(Coproc1Register::F2, expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP ldc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"ldc1 $f0, 0($t0)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "ldc1"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0xd5, 0x00, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    uint32_t address = memSectionOffset(MemSection::DATA);

    constexpr float64_t expected = 75.5;
    const int64_t intRepr = *reinterpret_cast<const int64_t*>(&expected);
    interpreter.getState().memory.wordTo(address, static_cast<int32_t>(intRepr & 0xFFFFFFFF));
    interpreter.getState().memory.wordTo(address + 4, static_cast<int32_t>(intRepr >> 32));
    interpreter.getState().registers[Register::T0] = static_cast<int32_t>(address);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getDouble(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP lwc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"lwc1 $f0, 0($t0)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "lwc1"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0xc5, 0x00, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    uint32_t address = memSectionOffset(MemSection::DATA);

    constexpr float32_t expected = 75.5;
    const int32_t intRepr = *reinterpret_cast<const int32_t*>(&expected);
    interpreter.getState().memory.wordTo(address, intRepr);
    interpreter.getState().registers[Register::T0] = static_cast<int32_t>(address);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}


TEST_CASE("Test FP sdc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"sdc1 $f0, 0($t0)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "sdc1"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0xf5, 0x00, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    uint32_t address = memSectionOffset(MemSection::DATA);

    constexpr float64_t expected = 75.5;
    const int64_t intRepr = *reinterpret_cast<const int64_t*>(&expected);
    const int32_t lower = static_cast<int32_t>(intRepr & 0xFFFFFFFF);
    const int32_t upper = static_cast<int32_t>(intRepr >> 32);
    interpreter.getState().registers[Register::T0] = static_cast<int32_t>(address);
    interpreter.getState().cp1.setDouble(Coproc1Register::F0, expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().memory.wordAt(address) == lower);
        REQUIRE(interpreter.getState().memory.wordAt(address + 4) == upper);
    }
}


TEST_CASE("Test FP swc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"swc1 $f0, 0($t0)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    Postprocessor::processBaseAddressing(actualTokens);
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "swc1"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0xe5, 0x00, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);
    uint32_t address = memSectionOffset(MemSection::DATA);

    constexpr float32_t floatRepr = 75.5;
    const int32_t expected = *reinterpret_cast<const int32_t*>(&floatRepr);
    interpreter.getState().registers[Register::T0] = static_cast<int32_t>(address);
    interpreter.getState().cp1.setFloat(Coproc1Register::F0, 75.5);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") { REQUIRE(expected == interpreter.getState().memory.wordAt(address)); }
}


TEST_CASE("Test FP mfc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"mfc1 $t0, $f0"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "mfc1"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x44, 0x08, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr int32_t expected = 0x012345678;
    interpreter.getState().cp1[Coproc1Register::F0] = expected;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") { REQUIRE(expected == interpreter.getState().registers[Register::T0]); }
}


TEST_CASE("Test FP mtc1 Instruction") {
    const SourceFile rawFile = makeRawFile({"mtc1 $t0, $f0"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "mtc1"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x44, 0x88, 0x00, 0x00});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr int32_t expected = 0x012345678;
    interpreter.getState().registers[Register::T0] = expected;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1[Coproc1Register::F0]);
    }
}


TEST_CASE("Test FP mov.s Instruction") {
    const SourceFile rawFile = makeRawFile({"mov.s $f0, $f1"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "mov.s"},
                 {TokenCategory::REGISTER, "f0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "f1"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x46, 0x00, 0x08, 0x06});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    constexpr float32_t expected = 42.69;
    interpreter.getState().cp1.setFloat(Coproc1Register::F1, expected);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(expected == interpreter.getState().cp1.getFloat(Coproc1Register::F0));
    }
}
