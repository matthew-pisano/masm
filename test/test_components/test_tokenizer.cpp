//
// Created by matthew on 4/14/25.
//


#include <catch2/catch_test_macros.hpp>

#include "../testing_utilities.h"
#include "exceptions.h"
#include "io/fileio.h"
#include "tokenizer/tokenizer.h"
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
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Single Lines") {
    SECTION("Test Directive") {
        const RawFile rawFile = makeRawFile({".asciiz"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile(rawFile);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::ALLOC_DIRECTIVE, "asciiz"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Memory Directive") {
        const RawFile rawFile = makeRawFile({".data"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEC_DIRECTIVE, "data"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Label Declaration") {
        const RawFile rawFile = makeRawFile({"label:"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL_DEF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Label Reference") {
        const RawFile rawFile = makeRawFile({"j label"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Instruction") {
        const RawFile rawFile = makeRawFile({"addi"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Register") {
        const RawFile rawFile = makeRawFile({"$v0"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::REGISTER, "v0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Immediate") {
        RawFile rawFile = makeRawFile({"42"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::IMMEDIATE, "42"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"-42"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "-42"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"-42.0"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "-42.0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"0x1a"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenType::IMMEDIATE, "26"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Seperator") {
        const RawFile rawFile = makeRawFile({"j ,"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::SEPERATOR, ","}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test String") {
        const RawFile rawFile = makeRawFile({R"("'ello \n\"There\"")"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::STRING, R"('ello \n\"There\")"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Globl") {
        const RawFile rawFile = makeRawFile({".globl label"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::META_DIRECTIVE, "globl"}, {TokenType::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Eqv") {
        const RawFile rawFile = makeRawFile({".eqv exit li $v0, 10", "exit"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "eqv"},
                                                           {TokenType::LABEL_REF, "exit"},
                                                           {TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "10"}},
                                                          {{TokenType::LABEL_REF, "exit"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Macro Parameters") {
        const RawFile rawFile = makeRawFile({".macro foobar(%foo, %bar)"});
        std::vector<SourceLine> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::META_DIRECTIVE, "macro"},
                                                           {TokenType::LABEL_REF, "foobar"},
                                                           {TokenType::OPEN_PAREN, "("},
                                                           {TokenType::MACRO_PARAM, "foo"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::MACRO_PARAM, "bar"},
                                                           {TokenType::CLOSE_PAREN, ")"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
}


TEST_CASE("Test Tokenize Invalid Syntax") {
    SECTION("Test Unexpected EOL") {
        const RawFile rawFile = makeRawFile({R"(unterminated: .asciiz "incomplet)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), std::runtime_error);
    }
}


TEST_CASE("Test Base Addressing") {
    RawFile rawFile = makeRawFile({"lw $t1, 8($t0)"});
    std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                       {TokenType::REGISTER, "t1"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::REGISTER, "t0"},
                                                       {TokenType::SEPERATOR, ","},
                                                       {TokenType::IMMEDIATE, "8"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

    rawFile = makeRawFile({"lw $t1, ($t0)"});
    actualTokens = Tokenizer::tokenize({rawFile});
    expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                       {TokenType::REGISTER, "t1"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::REGISTER, "t0"},
                       {TokenType::SEPERATOR, ","},
                       {TokenType::IMMEDIATE, "0"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Eqv") {
    const RawFile rawFile = makeRawFile({".eqv exit li $v0, 10", "exit"});
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
    const std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                             {TokenType::REGISTER, "v0"},
                                                             {TokenType::SEPERATOR, ","},
                                                             {TokenType::IMMEDIATE, "10"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Macro") {
    SECTION("Test Macro without Parameters") {
        const RawFile rawFile =
                makeRawFile({".macro done", "li $v0, 10", "syscall", ".end_macro", "done"});
        const std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "li"},
                 {TokenType::REGISTER, "v0"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::IMMEDIATE, "10"}},
                {{TokenType::INSTRUCTION, "syscall"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Macro with Parameters") {
        const RawFile rawFile =
                makeRawFile({".macro terminate(%termination_value)", "li $a0, %termination_value",
                             "li $v0, 17", "syscall", ".end_macro", "terminate(1)"});
        const std::vector<SourceLine> actualTokens = Tokenizer::tokenize({rawFile});
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "li"},
                 {TokenType::REGISTER, "a0"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::IMMEDIATE, "1"}},
                {{TokenType::INSTRUCTION, "li"},
                 {TokenType::REGISTER, "v0"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::IMMEDIATE, "17"}},
                {{TokenType::INSTRUCTION, "syscall"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
}


TEST_CASE("Test Tokenize Include") {
    const std::vector<RawFile> rawFile = {{"a.asm", {"jr $t0", R"(.include "b.asm")", "jr $t2"}},
                                          {"b.asm", {"label:", "jr $t1"}}};
    const std::vector<SourceLine> actualTokens = Tokenizer::tokenize(rawFile);
    const std::vector<std::vector<Token>> expectedTokens = {
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t0"}},
            {{TokenType::LABEL_DEF, "label@masm_mangle_file_a.asm"}},
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t1"}},
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t2"}},
            {{TokenType::LABEL_DEF, "label@masm_mangle_file_b.asm"}},
            {{TokenType::INSTRUCTION, "jr"}, {TokenType::REGISTER, "t1"}},
    };
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenizer Syntax Errors") {
    SECTION("Test Misplaced Quote") {
        const RawFile rawFile = makeRawFile({R"(g"hello")"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Unclosed Quote") {
        const RawFile rawFile = makeRawFile({R"("hello)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Unexpected First Token") {
        RawFile rawFile = makeRawFile({","});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({"("});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({")"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({":"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Undeclared Global Label") {
        const RawFile rawFile = makeRawFile({".globl invalid"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Global Label") {
        RawFile rawFile = makeRawFile({".globl"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".globl 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Eqv") {
        RawFile rawFile = makeRawFile({".eqv"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".eqv hello"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".eqv 1 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Base Addressing") {
        RawFile rawFile = makeRawFile({"($t0)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({"lw $s1 2($t0"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({"lw $s1 2()"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Params") {
        RawFile rawFile = makeRawFile({".macro macro %arg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".macro macro (%arg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Call") {
        RawFile rawFile = makeRawFile(
                {".macro macro(%arg)", "addi $t0, $zero, %arg", ".end_macro", "macro(0, 1)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile(
                {".macro macro(%arg)", "addi $t0, $zero, %bargg", ".end_macro", "macro(0)"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Malformed Macro Declaration") {
        RawFile rawFile = makeRawFile({".macro"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".macro 1"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".macro macro(%arg)", "addi $t0, $zero, %bargg"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);
    }

    SECTION("Test Invalid Include") {
        RawFile rawFile = makeRawFile({".include"});
        REQUIRE_THROWS_AS(Tokenizer::tokenize({rawFile}), MasmSyntaxError);

        rawFile = makeRawFile({".include 1"});
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


TEST_CASE("Test Tokenize Syscall Input Output") {
    const std::string test_case = "input_output";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}


TEST_CASE("Test Tokenize MMIO Input Output") {
    const std::string test_case = "mmio";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}
