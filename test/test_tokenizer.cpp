//
// Created by matthew on 4/14/25.
//


#include <catch2/catch_test_macros.hpp>

#include "exceptions.h"
#include "fileio.h"
#include "tokenizer.h"
#include "utils.h"


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

    std::vector<RawFile> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : sourceFileNames)
        sourceLines.push_back({getFileBasename(fileName), readFileLines(fileName)});

    const std::vector<SourceLine> actualTokens = Tokenizer::tokenize(sourceLines);
    REQUIRE(expectedTokens.size() == actualTokens.size());
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
}


/**
 * Wraps a series of lines into a singleton vector of raw files
 * @param lines The lines to wrap
 * @return A vector of raw files containing the lines
 */
std::vector<RawFile> wrapLines(const std::vector<std::string>& lines) { return {{"a.asm", lines}}; }


TEST_CASE("Test Tokenize Single Lines") {
    SECTION("Test Directive") {
        const std::vector<std::string> lines = {".asciiz"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::ALLOC_DIRECTIVE, "asciiz"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Memory Directive") {
        const std::vector<std::string> lines = {".data"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEC_DIRECTIVE, "data"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Label Declaration") {
        const std::vector<std::string> lines = {"label:"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL_DEF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Label Reference") {
        const std::vector<std::string> lines = {"j label"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABEL_REF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Instruction") {
        const std::vector<std::string> lines = {"addi"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Register") {
        const std::vector<std::string> lines = {"$v0"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::REGISTER, "v0"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Immediate") {
        std::vector<std::string> lines = {"42"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::IMMEDIATE, "42"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

        lines = {"-42"};
        actualTokens = Tokenizer::tokenizeFile(lines);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

        lines = {"-42.0"};
        actualTokens = Tokenizer::tokenizeFile(lines);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42.0"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Seperator") {
        const std::vector<std::string> lines = {"j ,"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::SEPERATOR, ","}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test String") {
        const std::vector<std::string> lines = {R"("'ello \n\"There\"")"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::STRING, R"('ello \n\"There\")"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Globl") {
        std::vector<std::string> lines = {".globl label"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::META_DIRECTIVE, "globl"}, {TokenType::LABEL_REF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Eqv") {
        std::vector<std::string> lines = {".eqv exit li $v0, 10", "exit"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "eqv"},
                                                           {TokenType::LABEL_REF, "exit"},
                                                           {TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::LABEL_REF, "exit"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Macro Parameters") {
        const std::vector<std::string> lines = {".macro foobar(%foo, %bar)"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(lines);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "macro"},
                                                           {TokenType::LABEL_REF, "foobar"},
                                                           {TokenType::OPEN_PAREN, "("},
                                                           {TokenType::MACRO_PARAM, "foo"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::MACRO_PARAM, "bar"},
                                                           {TokenType::CLOSE_PAREN, ")"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
}


TEST_CASE("Test Tokenize Invalid Syntax") {
    SECTION("Test Unexpected EOL") {
        const std::vector<std::string> lines = {R"(unterminated: .asciiz "incomplet)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), std::runtime_error);
    }
}


TEST_CASE("Test Base Addressing") {
    std::vector<std::string> lines = {"lw $t1, 8($t0)"};
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize(wrapLines({lines}));
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                       {TokenType::REGISTER, "t1"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::REGISTER, "t0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "8"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

    lines = {"lw $t1, ($t0)"};
    actualTokens = Tokenizer::tokenize(wrapLines({lines}));
    expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                       {TokenType::REGISTER, "t1"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::REGISTER, "t0"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::IMMEDIATE, "0"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
}


TEST_CASE("Test Tokenize Eqv") {
    const std::vector<std::string> lines = {".eqv exit li $v0, 10", "exit"};
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize(wrapLines({lines}));
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                       {TokenType::REGISTER, "v0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "10"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
}


TEST_CASE("Test Tokenize Macro") {
    SECTION("Test Macro without Parameters") {
        const std::vector<std::string> lines = {".macro done", "li $v0, 10", "syscall",
                                                ".end_macro", "done"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenize(wrapLines({lines}));
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::INSTRUCTION, "syscall"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Macro with Parameters") {
        const std::vector<std::string> lines = {".macro terminate(%termination_value)",
                                                "li $a0, %termination_value",
                                                "li $v0, 17",
                                                "syscall",
                                                ".end_macro",
                                                "terminate(1)"};
        std::vector<SourceLine> actualTokens = Tokenizer::tokenize(wrapLines({lines}));
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "a0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "1"}},
                                                          {{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "17"}},
                                                          {{TokenType::INSTRUCTION, "syscall"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
}


TEST_CASE("Test Tokenizer Syntax Errors") {
    SECTION("Test Misplaced Quote") {
        const std::vector<std::string> lines = {R"(g"hello")"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Unclosed Quote") {
        const std::vector<std::string> lines = {R"("hello)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Unexpected First Token") {
        std::vector<std::string> lines = {R"(,)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(()"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"())"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(:)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Undeclared Global Label") {
        const std::vector<std::string> lines = {R"(.globl invalid)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Invalid Global Label") {
        std::vector<std::string> lines = {R"(.globl)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(.globl 1)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Invalid Eqv") {
        std::vector<std::string> lines = {R"(.eqv)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(.eqv hello)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(.eqv 1 1)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Base Addressing") {
        std::vector<std::string> lines = {R"(($t0))"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"("lw $s1 2($t0)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"("lw $s1 2())"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Params") {
        std::vector<std::string> lines = {R"(.macro macro %arg)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {R"(".macro macro (%arg)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Call") {
        std::vector<std::string> lines = {".macro macro(%arg)", "addi $t0, $zero, %arg",
                                          ".end_macro", "macro(0, 1)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {".macro macro(%arg)", "addi $t0, $zero, %bargg", ".end_macro", "macro(0)"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Declaration") {
        std::vector<std::string> lines = {".macro"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {".macro 1"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);

        lines = {".macro macro(%arg)", "addi $t0, $zero, %bargg"};
        REQUIRE_THROWS_AS(Tokenizer::tokenize(wrapLines({lines})), MasmSyntaxError);
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
