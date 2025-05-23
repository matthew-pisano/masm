//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>

#include "interpreter.h"
#include "parser.h"
#include "testing_utilities.h"
#include "tokenizer.h"


TEST_CASE("Test j Instruction") {
    const RawFile rawFile = makeRawFile({"j label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x08, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test jal Instruction") {
    const RawFile rawFile = makeRawFile({"jal label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "jal"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x0c, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(interpreter.getState().registers[Register::RA] == 0x00400004);
    }
}


TEST_CASE("Test jr Instruction") {
    const RawFile rawFile = makeRawFile({"jr $t0"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x00, 0x00, 0x08});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T0] = 0x00400010;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test jalr Instruction") {
    const RawFile rawFile = makeRawFile({"jalr $t0"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "jalr"}, {TokenType::REGISTER, "t0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser{};
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x01, 0x00, 0x00, 0x09});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreter{std::cin, std::cout};
    interpreter.setUpdateMMIO(false);
    interpreter.getState().registers[Register::T0] = 0x00400010;
    interpreter.interpret(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(interpreter.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(interpreter.getState().registers[Register::RA] == 0x00400004);
    }
}


TEST_CASE("Test beq Instruction") {
    const RawFile rawFile = makeRawFile({"beq $t0, $t1, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "beq"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x11, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 37;
    interpreterEq.getState().registers[Register::T1] = 37;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterNe{std::cin, std::cout};
    interpreterNe.setUpdateMMIO(false);
    interpreterNe.getState().registers[Register::T0] = 37;
    interpreterNe.getState().registers[Register::T1] = 42;
    interpreterNe.interpret(actualLayout);
    SECTION("Test Execute Not Equal") {
        REQUIRE(interpreterNe.getState().registers[Register::PC] == 0x00400004);
    }
}


TEST_CASE("Test bne Instruction") {
    const RawFile rawFile = makeRawFile({"bne $t0, $t1, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "bne"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::REGISTER, "t1"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = intVec2ByteVec({0x15, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 37;
    interpreterEq.getState().registers[Register::T1] = 37;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400004);
    }

    DebugInterpreter interpreter2Ne{std::cin, std::cout};
    interpreter2Ne.setUpdateMMIO(false);
    interpreter2Ne.getState().registers[Register::T0] = 37;
    interpreter2Ne.getState().registers[Register::T1] = 42;
    interpreter2Ne.interpret(actualLayout);
    SECTION("Test Execute Not Equal") {
        REQUIRE(interpreter2Ne.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test bgtz Instruction") {
    const RawFile rawFile = makeRawFile({"bgtz $t0, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "bgtz"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                intVec2ByteVec({0x00, 0x08, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterGt{std::cin, std::cout};
    interpreterGt.setUpdateMMIO(false);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterLt{std::cin, std::cout};
    interpreterLt.setUpdateMMIO(false);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400008);
    }
}


TEST_CASE("Test bltz Instruction") {
    const RawFile rawFile = makeRawFile({"bltz $t0, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "bltz"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                intVec2ByteVec({0x01, 0x00, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterGt{std::cin, std::cout};
    interpreterGt.setUpdateMMIO(false);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterLt{std::cin, std::cout};
    interpreterLt.setUpdateMMIO(false);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400010);
    }
}


TEST_CASE("Test bgez Instruction") {
    const RawFile rawFile = makeRawFile({"bgez $t0, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "bgez"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                intVec2ByteVec({0x01, 0x00, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterGt{std::cin, std::cout};
    interpreterGt.setUpdateMMIO(false);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterLt{std::cin, std::cout};
    interpreterLt.setUpdateMMIO(false);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400008);
    }
}


TEST_CASE("Test blez Instruction") {
    const RawFile rawFile = makeRawFile({"blez $t0, label"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "blez"},
                                                                 {TokenType::REGISTER, "t0"},
                                                                 {TokenType::SEPERATOR, ","},
                                                                 {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    DebugParser parser{};
    parser.getLabels().labelMap["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes =
                intVec2ByteVec({0x00, 0x08, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    DebugInterpreter interpreterEq{std::cin, std::cout};
    interpreterEq.setUpdateMMIO(false);
    interpreterEq.getState().registers[Register::T0] = 0;
    interpreterEq.interpret(actualLayout);
    SECTION("Test Execute Equal") {
        REQUIRE(interpreterEq.getState().registers[Register::PC] == 0x00400010);
    }

    DebugInterpreter interpreterGt{std::cin, std::cout};
    interpreterGt.setUpdateMMIO(false);
    interpreterGt.getState().registers[Register::T0] = 69;
    interpreterGt.interpret(actualLayout);
    SECTION("Test Execute Greater Than") {
        REQUIRE(interpreterGt.getState().registers[Register::PC] == 0x00400008);
    }

    DebugInterpreter interpreterLt{std::cin, std::cout};
    interpreterLt.setUpdateMMIO(false);
    interpreterLt.getState().registers[Register::T0] = -420;
    interpreterLt.interpret(actualLayout);
    SECTION("Test Execute Less Than") {
        REQUIRE(interpreterLt.getState().registers[Register::PC] == 0x00400010);
    }
}
