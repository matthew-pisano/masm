//
// Created by matthew on 5/23/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>

#include <masm/assembler/parser.hpp>
#include <masm/assembler/tokenizer.hpp>
#include <masm/simulator/simulator.hpp>

#include "mdb/debug_simulator.hpp"
#include "tests/testing_utilities.hpp"


TEST_CASE("Test j Instruction") {
    const SourceFile rawFile = makeRawFile({"j label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "j"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x08, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    simulator.simulate(actualLayout);
    SECTION("Test Execute") { REQUIRE(simulator.getState().registers[Register::PC] == 0x00400010); }
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
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x0c, 0x10, 0x00, 0x04});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    simulator.simulate(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(simulator.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(simulator.getState().registers[Register::RA] == 0x00400004);
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
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x00, 0x08});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    simulator.getState().registers[Register::T0] = 0x00400010;
    simulator.simulate(actualLayout);
    SECTION("Test Execute") { REQUIRE(simulator.getState().registers[Register::PC] == 0x00400010); }
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
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x00, 0x09});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    simulator.getState().registers[Register::T0] = 0x00400010;
    simulator.simulate(actualLayout);
    SECTION("Test Execute") {
        REQUIRE(simulator.getState().registers[Register::PC] == 0x00400010);
        REQUIRE(simulator.getState().registers[Register::RA] == 0x00400004);
    }
}


TEST_CASE("Test beq Instruction") {
    const SourceFile rawFile = makeRawFile({"beq $t0, $t1, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "beq"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x11, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 37;
    simulatorEq.getState().registers[Register::T1] = 37;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400010); }

    DebugSimulator simulatorNe(IOMode::SYSCALL, streamHandle);
    simulatorNe.getState().registers[Register::T0] = 37;
    simulatorNe.getState().registers[Register::T1] = 42;
    simulatorNe.simulate(actualLayout);
    SECTION("Test Execute Not Equal") { REQUIRE(simulatorNe.getState().registers[Register::PC] == 0x00400004); }
}


TEST_CASE("Test bne Instruction") {
    const SourceFile rawFile = makeRawFile({"bne $t0, $t1, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "bne"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::REGISTER, "t1"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x15, 0x09, 0x00, 0x03});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 37;
    simulatorEq.getState().registers[Register::T1] = 37;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400004); }

    DebugSimulator simulator2Ne(IOMode::SYSCALL, streamHandle);
    simulator2Ne.getState().registers[Register::T0] = 37;
    simulator2Ne.getState().registers[Register::T1] = 42;
    simulator2Ne.simulate(actualLayout);
    SECTION("Test Execute Not Equal") { REQUIRE(simulator2Ne.getState().registers[Register::PC] == 0x00400010); }
}


TEST_CASE("Test bgtz Instruction") {
    const SourceFile rawFile = makeRawFile({"bgtz $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "bgtz"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x00, 0x08, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 0;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400008); }

    DebugSimulator simulatorGt(IOMode::SYSCALL, streamHandle);
    simulatorGt.getState().registers[Register::T0] = 69;
    simulatorGt.simulate(actualLayout);
    SECTION("Test Execute Greater Than") { REQUIRE(simulatorGt.getState().registers[Register::PC] == 0x00400010); }

    DebugSimulator simulatorLt(IOMode::SYSCALL, streamHandle);
    simulatorLt.getState().registers[Register::T0] = -420;
    simulatorLt.simulate(actualLayout);
    SECTION("Test Execute Less Than") { REQUIRE(simulatorLt.getState().registers[Register::PC] == 0x00400008); }
}


TEST_CASE("Test bltz Instruction") {
    const SourceFile rawFile = makeRawFile({"bltz $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "bltz"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x08, 0x2a, 0x14, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 0;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400008); }

    DebugSimulator simulatorGt(IOMode::SYSCALL, streamHandle);
    simulatorGt.getState().registers[Register::T0] = 69;
    simulatorGt.simulate(actualLayout);
    SECTION("Test Execute Greater Than") { REQUIRE(simulatorGt.getState().registers[Register::PC] == 0x00400008); }

    DebugSimulator simulatorLt(IOMode::SYSCALL, streamHandle);
    simulatorLt.getState().registers[Register::T0] = -420;
    simulatorLt.simulate(actualLayout);
    SECTION("Test Execute Less Than") { REQUIRE(simulatorLt.getState().registers[Register::PC] == 0x00400010); }
}


TEST_CASE("Test bgez Instruction") {
    const SourceFile rawFile = makeRawFile({"bgez $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "bgez"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x01, 0x00, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 0;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400010); }

    DebugSimulator simulatorGt(IOMode::SYSCALL, streamHandle);
    simulatorGt.getState().registers[Register::T0] = 69;
    simulatorGt.simulate(actualLayout);
    SECTION("Test Execute Greater Than") { REQUIRE(simulatorGt.getState().registers[Register::PC] == 0x00400010); }

    DebugSimulator simulatorLt(IOMode::SYSCALL, streamHandle);
    simulatorLt.getState().registers[Register::T0] = -420;
    simulatorLt.simulate(actualLayout);
    SECTION("Test Execute Less Than") { REQUIRE(simulatorLt.getState().registers[Register::PC] == 0x00400008); }
}


TEST_CASE("Test blez Instruction") {
    const SourceFile rawFile = makeRawFile({"blez $t0, label"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "blez"},
                                                                 {TokenCategory::REGISTER, "t0"},
                                                                 {TokenCategory::SEPERATOR, ","},
                                                                 {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    parser.getLabels()["label"] = 0x00400010;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x00, 0x08, 0x08, 0x2a, 0x10, 0x20, 0x00, 0x02});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    StreamHandle streamHandle(std::cin, std::cout);
    DebugSimulator simulatorEq(IOMode::SYSCALL, streamHandle);
    simulatorEq.getState().registers[Register::T0] = 0;
    simulatorEq.simulate(actualLayout);
    SECTION("Test Execute Equal") { REQUIRE(simulatorEq.getState().registers[Register::PC] == 0x00400010); }

    DebugSimulator simulatorGt(IOMode::SYSCALL, streamHandle);
    simulatorGt.getState().registers[Register::T0] = 69;
    simulatorGt.simulate(actualLayout);
    SECTION("Test Execute Greater Than") { REQUIRE(simulatorGt.getState().registers[Register::PC] == 0x00400008); }

    DebugSimulator simulatorLt(IOMode::SYSCALL, streamHandle);
    simulatorLt.getState().registers[Register::T0] = -420;
    simulatorLt.simulate(actualLayout);
    SECTION("Test Execute Less Than") { REQUIRE(simulatorLt.getState().registers[Register::PC] == 0x00400010); }
}


TEST_CASE("Test break Instruction") {
    const SourceFile rawFile = makeRawFile({"break"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "break"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x00, 0x00, 0x00, 0x0d});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    std::ostringstream oss;
    StreamHandle streamHandle(std::cin, oss);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    SECTION("Test Execute") {
        int exitCode = simulator.simulate(actualLayout);
        const std::string expectedOutput = "Break instruction executed";
        REQUIRE(expectedOutput == oss.str());
        REQUIRE(exitCode == 0);
    }

    std::string inputString = "c\nq\n";
    std::istringstream iss(inputString);
    oss.str("");
    oss.clear();

    StreamHandle interactiveStreamHandle(iss, oss);
    DebugSimulator interactiveSimulator(IOMode::SYSCALL, interactiveStreamHandle);
    interactiveSimulator.setInteractive(true);
    SECTION("Test Debugger") {
        interactiveSimulator.simulate(actualLayout);

        const std::string expectedOutput = "\b\n(mdb) Break instruction executed\n"
                                           "There is no program running.  Use 'run' to "
                                           "restart\b\n(mdb) ";
        REQUIRE(expectedOutput == oss.str());
    }
}


TEST_CASE("Test break Instruction with Code") {
    const SourceFile rawFile = makeRawFile({"break 42"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
    SECTION("Test Tokenize") {
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "break"}, {TokenCategory::IMMEDIATE, "42"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    Parser parser;
    const MemLayout actualLayout = parser.parse(actualTokens, true);
    SECTION("Test Parse") {
        const std::vector<std::byte> expectedBytes = iV2bV({0x00, 0x00, 0x0a, 0x8d});
        const std::vector<std::byte> actualBytes = actualLayout.data.at(MemSection::TEXT);
        REQUIRE(expectedBytes == actualBytes);
    }

    std::ostringstream oss;
    StreamHandle streamHandle(std::cin, oss);
    DebugSimulator simulator(IOMode::SYSCALL, streamHandle);

    SECTION("Test Execute") {
        int exitCode = simulator.simulate(actualLayout);
        const std::string expectedOutput = "Break instruction executed";
        REQUIRE(expectedOutput == oss.str());
        REQUIRE(exitCode == 42);
    }

    std::string inputString = "c\nq\n";
    std::istringstream iss(inputString);
    oss.str("");
    oss.clear();

    StreamHandle interactiveStreamHandle(iss, oss);
    DebugSimulator interactiveSimulator(IOMode::SYSCALL, interactiveStreamHandle);
    interactiveSimulator.setInteractive(true);
    SECTION("Test Debugger") {
        interactiveSimulator.simulate(actualLayout);
        const std::string expectedOutput = "\b\n(mdb) Break instruction executed\n"
                                           "There is no program running.  Use 'run' to "
                                           "restart\b\n(mdb) ";
        REQUIRE(expectedOutput == oss.str());
    }
}
