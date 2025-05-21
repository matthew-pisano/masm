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
 * Wraps a series of lines into a raw files
 * @param lines The lines to wrap
 * @return A raw file containing the lines
 */
RawFile wrapLines(const std::vector<std::string>& lines) { return {"a.asm", lines}; }


TEST_CASE("Test Tokenize Single Lines") {
    SECTION("Test Directive") {
        const RawFile rawFile = wrapLines({".asciiz"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(rawFile);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::ALLOC_DIRECTIVE, "asciiz"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Memory Directive") {
        const RawFile rawFile = wrapLines({".data"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEC_DIRECTIVE, "data"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Label Declaration") {
        const RawFile rawFile = wrapLines({"label:"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL_DEF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Label Reference") {
        const RawFile rawFile = wrapLines({"j label"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABEL_REF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Instruction") {
        const RawFile rawFile = wrapLines({"addi"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Register") {
        const RawFile rawFile = wrapLines({"$v0"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::REGISTER, "v0"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Immediate") {
        RawFile rawFile = wrapLines({"42"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::IMMEDIATE, "42"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

        rawFile = wrapLines({"-42"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "-42"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

        rawFile = wrapLines({"-42.0"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "-42.0"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

        rawFile = wrapLines({"0x1a"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "26"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test Seperator") {
        const RawFile rawFile = wrapLines({"j ,"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::SEPERATOR, ","}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }
    SECTION("Test String") {
        const RawFile rawFile = wrapLines({R"("'ello \n\"There\"")"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::STRING, R"('ello \n\"There\")"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Globl") {
        const RawFile rawFile = wrapLines({".globl label"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::META_DIRECTIVE, "globl"}, {TokenType::LABEL_REF, "label"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Eqv") {
        const RawFile rawFile = wrapLines({".eqv exit li $v0, 10", "exit"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
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
        const RawFile rawFile = wrapLines({".macro foobar(%foo, %bar)"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
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
        const RawFile rawFile = wrapLines({R"(unterminated: .asciiz "incomplet)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), std::runtime_error);
    }
}


TEST_CASE("Test Base Addressing") {
    RawFile rawFile = wrapLines({"lw $t1, 8($t0)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                       {TokenType::REGISTER, "t1"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::REGISTER, "t0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "8"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);

    rawFile = wrapLines({"lw $t1, ($t0)"});
    actualTokens = Tokenizer::tokenize({rawFile});
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
    const RawFile rawFile = wrapLines({".eqv exit li $v0, 10", "exit"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                       {TokenType::REGISTER, "v0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "10"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
}


TEST_CASE("Test Tokenize Macro") {
    SECTION("Test Macro without Parameters") {
        const RawFile rawFile =
                wrapLines({".macro done", "li $v0, 10", "syscall", ".end_macro", "done"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::INSTRUCTION, "syscall"}}};
        for (size_t i = 0; i < expectedTokens.size(); ++i)
            REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
    }

    SECTION("Test Macro with Parameters") {
        const RawFile rawFile =
                wrapLines({".macro terminate(%termination_value)", "li $a0, %termination_value",
                           "li $v0, 17", "syscall", ".end_macro", "terminate(1)"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
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


TEST_CASE("Test Tokenize Include") {
    const std::vector<RawFile> rawFile = {{"a.asm", {"jr $t0", R"(.include "b.asm")", "jr $t2"}},
                                          {"b.asm", {"label:", "jr $t1"}}};
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize(rawFile);
    std::vector<std::vector<Token>> expectedTokens = {
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t0"}},
            {{TokenType::LABEL_DEF, "label@masm_mangle_file_a.asm"}},
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t1"}},
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t2"}}};
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        REQUIRE(expectedTokens[i] == actualTokens[i].tokens);
}


TEST_CASE("Test Tokenizer Syntax Errors") {
    SECTION("Test Misplaced Quote") {
        const RawFile rawFile = wrapLines({R"(g"hello")"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Unclosed Quote") {
        const RawFile rawFile = wrapLines({R"("hello)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Unexpected First Token") {
        RawFile rawFile = wrapLines({","});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({"("});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({")"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({":"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Undeclared Global Label") {
        const RawFile rawFile = wrapLines({".globl invalid"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Global Label") {
        RawFile rawFile = wrapLines({".globl"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".globl 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Eqv") {
        RawFile rawFile = wrapLines({".eqv"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".eqv hello"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".eqv 1 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Base Addressing") {
        RawFile rawFile = wrapLines({"($t0)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({"lw $s1 2($t0"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({"lw $s1 2()"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Params") {
        RawFile rawFile = wrapLines({".macro macro %arg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".macro macro (%arg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Call") {
        RawFile rawFile = wrapLines(
                {".macro macro(%arg)", "addi $t0, $zero, %arg", ".end_macro", "macro(0, 1)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines(
                {".macro macro(%arg)", "addi $t0, $zero, %bargg", ".end_macro", "macro(0)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Declaration") {
        RawFile rawFile = wrapLines({".macro"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".macro 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".macro macro(%arg)", "addi $t0, $zero, %bargg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Include") {
        RawFile rawFile = wrapLines({".include"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = wrapLines({".include 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
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
