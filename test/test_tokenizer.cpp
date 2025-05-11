//
// Created by matthew on 4/14/25.
//


#include <catch2/catch_test_macros.hpp>

#include "fileio.h"
#include "tokenizer.h"


/**
 * Validates the tokenization of a file against the loaded, expected tokens
 * @param sourceFileNames The names of the source files to tokenize
 * @param tokensFileName The name of the tokenized file to compare against
 */
void validateTokens(const std::vector<std::string>& sourceFileNames,
                    const std::string& tokensFileName) {
    const std::vector<std::string> tokenizedLines = readFileLines(tokensFileName);

    constexpr char groupSep = 0x1d;
    std::vector<std::vector<Token>> expectedTokens = {};
    for (const std::string& line : tokenizedLines) {
        if (line.empty())
            continue;
        expectedTokens.emplace_back();
        std::vector<Token>& lastLine = expectedTokens[expectedTokens.size() - 1];

        std::string token;
        size_t lastToken = -1;
        std::string tokenType;
        std::string tokenValue;
        for (size_t i = 0; i < line.length(); i++) {
            if (line[i] == groupSep) {
                lastToken = i;
                lastLine.push_back({});
                lastLine[lastLine.size() - 1].type = static_cast<TokenType>(std::stoi(tokenType));
                lastLine[lastLine.size() - 1].value = tokenValue;
                tokenType.clear();
                tokenValue.clear();
                continue;
            }
            if (i - lastToken <= 2) {
                tokenType += line[i];
                continue;
            }

            tokenValue += line[i];
        }
    }

    std::vector<std::vector<std::string>> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& sourceFileName : sourceFileNames)
        sourceLines.push_back(readFileLines(sourceFileName));
    const std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenize(sourceLines);
    REQUIRE(expectedTokens.size() == actualTokens.size());
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i]);
}


TEST_CASE("Test Tokenize Single Lines") {
    SECTION("Test Directive") {
        const std::vector<std::string> lines = {".asciiz"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::ALLOC_DIRECTIVE, "asciiz"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Memory Directive") {
        const std::vector<std::string> lines = {".data"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEC_DIRECTIVE, "data"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Label Declaration") {
        const std::vector<std::string> lines = {"label:"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL_DEF, "label"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Label Reference") {
        const std::vector<std::string> lines = {"j label"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Instruction") {
        const std::vector<std::string> lines = {"addi"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Register") {
        const std::vector<std::string> lines = {"$v0"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::REGISTER, "v0"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Immediate") {
        std::vector<std::string> lines = {"42"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::IMMEDIATE, "42"}}};
        REQUIRE(expectedTokens == actualTokens);

        lines = {"-42"};
        actualTokens = Tokenizer::tokenizeFile(lines);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42"}}};
        REQUIRE(expectedTokens == actualTokens);

        lines = {"-42.0"};
        actualTokens = Tokenizer::tokenizeFile(lines);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42.0"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Seperator") {
        const std::vector<std::string> lines = {","};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEPERATOR, ","}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test String") {
        const std::vector<std::string> lines = {R"("'ello \n\"There\"")"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::STRING, R"('ello \n\"There\")"}}};
        REQUIRE(expectedTokens == actualTokens);
    }

    SECTION("Test Globl") {
        std::vector<std::string> lines = {".globl label"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::META_DIRECTIVE, "globl"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE(expectedTokens == actualTokens);
    }

    SECTION("Test Eqv") {
        std::vector<std::string> lines = {".eqv exit li $v0, 10", "exit"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "eqv"},
                                                           {TokenType::LABEL_REF, "exit"},
                                                           {TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::LABEL_REF, "exit"}}};
        REQUIRE(expectedTokens == actualTokens);
    }

    SECTION("Test Macro Parameters") {
        const std::vector<std::string> lines = {".macro foobar(%foo, %bar)"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "macro"},
                                                           {TokenType::LABEL_REF, "foobar"},
                                                           {TokenType::OPEN_PAREN, "("},
                                                           {TokenType::MACRO_PARAM, "foo"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::MACRO_PARAM, "bar"},
                                                           {TokenType::CLOSE_PAREN, ")"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
}


TEST_CASE("Test Tokenize Invalid Syntax") {
    SECTION("Test Unexpected EOL") {
        const std::vector<std::string> lines = {R"(unterminated: .asciiz "incomplet)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize({lines}), std::runtime_error);
    }
}


TEST_CASE("Test Base Addressing") {
    std::vector<std::string> lines = {"lw $t1, 8($t0)"};
    std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenize({lines});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                       {TokenType::REGISTER, "t1"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::REGISTER, "t0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "8"}}};
    REQUIRE(expectedTokens == actualTokens);

    lines = {"lw $t1, ($t0)"};
    actualTokens = Tokenizer::tokenize({lines});
    expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                       {TokenType::REGISTER, "t1"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::REGISTER, "t0"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::IMMEDIATE, "0"}}};
    REQUIRE(expectedTokens == actualTokens);
}


TEST_CASE("Test Tokenize Eqv") {
    const std::vector<std::string> lines = {".eqv exit li $v0, 10", "exit"};
    std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenize({lines});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                       {TokenType::REGISTER, "v0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "10"}}};
    REQUIRE(expectedTokens == actualTokens);
}


TEST_CASE("Test Tokenize Macro") {
    SECTION("Test Macro without Parameters") {
        const std::vector<std::string> lines = {".macro done", "li $v0, 10", "syscall",
                                                ".end_macro", "done"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenize({lines});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::INSTRUCTION, "syscall"}}};
        REQUIRE(expectedTokens == actualTokens);
    }

    SECTION("Test Macro with Parameters") {
        const std::vector<std::string> lines = {".macro terminate(%termination_value)",
                                                "li $a0, %termination_value",
                                                "li $v0, 17",
                                                "syscall",
                                                ".end_macro",
                                                "terminate(1)"};
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenize({lines});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "a0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "1"}},
                                                          {{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "17"}},
                                                          {{TokenType::INSTRUCTION, "syscall"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
}


TEST_CASE("Test Tokenize Hello World") {
    const std::string test_case = "hello_world";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}


TEST_CASE("Test Tokenize Load Address") {
    const std::string test_case = "load_address";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}


TEST_CASE("Test Tokenize Arithmetic") {
    const std::string test_case = "arithmetic";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}


TEST_CASE("Test Tokenize Syscall") {
    const std::string test_case = "syscall";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}

TEST_CASE("Test Tokenize Loops") {
    const std::string test_case = "loops";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}


TEST_CASE("Test Tokenize Globals") {
    const std::string test_case = "globals";
    validateTokens({"test/fixtures/" + test_case + "/globalsOne.asm",
                    "test/fixtures/" + test_case + "/globalsTwo.asm"},
                   "test/fixtures/" + test_case + "/globalsOne.tkn");
}
