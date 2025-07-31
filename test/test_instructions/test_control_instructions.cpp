//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "debug/debug_interpreter.h"
#include "interpreter/interpreter.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"


TEST_CASE("Test j Instruction") {
    const SourceFile rawFile = makeRawFile({"j label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "j"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x08, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test jal Instruction") {
    const SourceFile rawFile = makeRawFile({"jal label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "jal"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x0c, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(interpreter.getState().registers[Register::RA] == 0x00400004);
    }
}


TEST_CASE("Test jr Instruction") {
    const SourceFile rawFile = makeRawFile({"jr $t0"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "jr"}, {TokenCategory::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x00, 0x08});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    interpreter.getState().registers[Register::T0] = 0x00400010;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test jalr Instruction") {
    const SourceFile rawFile = makeRawFile({"jalr $t0"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "jalr"}, {TokenCategory::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x00, 0x09});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreter(IOMode::SYSCALL, streamHandle);

    interpreter.getState().registers[Register::T0] = 0x00400010;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(interpreter.getState().registers[Register::RA] == 0x00400004);
    }
}


TEST_CASE("Test beq Instruction") {
    const SourceFile rawFile = makeRawFile({"beq $t0, $t1, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "beq"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x11, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 37;
    interpreterEq.getState().registers[Register::T1] = 37;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterNe(IOMode::SYSCALL, streamHandle);
    interpreterNe.getState().registers[Register::T0] = 37;
    interpreterNe.getState().registers[Register::T1] = 42;
    interpreterNe.interpret(actualLayout);
    SECTION("Test Execute Not Equal") {
        REQUIRE(interpreterNe.getState().registers[Register::PC] == 0x00400004);
    }
}


TEST_CASE("Test bne Instruction") {
    const SourceFile rawFile = makeRawFile({"bne $t0, $t1, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bne"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::REGISTER, "t1"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x15, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 37;
    interpreterEq.getState().registers[Register::T1] = 37;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400004);
    }

    DebugInterpreter interpreter2Ne(IOMode::SYSCALL, streamHandle);
    interpreter2Ne.getState().registers[Register::T0] = 37;
    interpreter2Ne.getState().registers[Register::T1] = 42;
    interpreter2Ne.interpret(actualLayout);
    SECTION("Test Execute Not Equal") {
        REQUIRE(interpreter2Ne.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test bgtz Instruction") {
    const SourceFile rawFile = makeRawFile({"bgtz $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bgtz"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                iV2bV({0x00, 0x08, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterGt(IOMode::SYSCALL, streamHandle);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterLt(IOMode::SYSCALL, streamHandle);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400008);
    }
}


TEST_CASE("Test bltz Instruction") {
    const SourceFile rawFile = makeRawFile({"bltz $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bltz"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                iV2bV({0x01, 0x00, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterGt(IOMode::SYSCALL, streamHandle);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterLt(IOMode::SYSCALL, streamHandle);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test bgez Instruction") {
    const SourceFile rawFile = makeRawFile({"bgez $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "bgez"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                iV2bV({0x01, 0x00, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterGt(IOMode::SYSCALL, streamHandle);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterLt(IOMode::SYSCALL, streamHandle);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400008);
    }
}


TEST_CASE("Test blez Instruction") {
    const SourceFile rawFile = makeRawFile({"blez $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "blez"},
                 {TokenCategory::REGISTER, "t0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                iV2bV({0x00, 0x08, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugInterpreter interpreterEq(IOMode::SYSCALL, streamHandle);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterGt(IOMode::SYSCALL, streamHandle);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterLt(IOMode::SYSCALL, streamHandle);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400010);
    }
}
