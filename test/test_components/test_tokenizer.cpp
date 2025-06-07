//
// Created by matthew on 4/14/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

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
    std::string tokensSource = readFile(tokensFileName);
    std::vector<std::string> tokenizedLines;
    std::stringstream ss(tokensSource);
    std::string line;
    while (std::getline(ss, line))
        tokenizedLines.push_back(line);

    constexpr char groupSep = 0x1d;
    std::vector<std::vector<Token>> expectedTokens = {};
    for (const std::string& line : tokenizedLines) {
        if (line.empty())
            continue;
        expectedTokens.emplace_back();
        std::vector<Token>& lastLine = expectedTokens[expectedTokens.size() - 1];

        std::string token;
        size_t lastToken = -1;
        std::string TokenCategory;
        std::string tokenValue;
        for (size_t i = 0; i < line.length(); i++) {
            if (line[i] == groupSep) {
                lastToken = i;
                lastLine.push_back({});
                lastLine[lastLine.size() - 1].type = static_cast<TokenCategory>(std::stoi(TokenCategory));
                lastLine[lastLine.size() - 1].value = tokenValue;
                TokenCategory.clear();
                tokenValue.clear();
                continue;
            }
            if (i - lastToken <= 2) {
                TokenCategory += line[i];
                continue;
            }

            tokenValue += line[i];
        }
    }

    std::vector<SourceFile> sourceLines;
    sourceLines.reserve(sourceFileNames.size()); // Preallocate memory for performance
    for (const std::string& fileName : sourceFileNames)
        sourceLines.push_back({getFileBasename(fileName), readFile(fileName)});

    const std::vector<LineTokens> actualTokens = Tokenizer::tokenize(sourceLines);
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Single Lines") {
    SECTION("Test Directive") {
        const SourceFile rawFile = makeRawFile({".asciiz"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile(rawFile);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::ALLOC_DIRECTIVE, "asciiz"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Memory Directive") {
        const SourceFile rawFile = makeRawFile({".data"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::SEC_DIRECTIVE, "data"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Label Declaration") {
        const SourceFile rawFile = makeRawFile({"label:"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::LABEL_DEF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Label Reference") {
        const SourceFile rawFile = makeRawFile({"j label"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "j"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Instruction") {
        const SourceFile rawFile = makeRawFile({"addi"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "addi"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Register") {
        const SourceFile rawFile = makeRawFile({"$v0"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::REGISTER, "v0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Immediate") {
        SourceFile rawFile = makeRawFile({"42"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::IMMEDIATE, "42"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"-42"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenCategory::IMMEDIATE, "-42"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"-42.0"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenCategory::IMMEDIATE, "-42.0"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

        rawFile = makeRawFile({"0x1a"});
        actualTokens = Tokenizer::tokenizeFile({rawFile});
        expectedTokens = {{{TokenCategory::IMMEDIATE, "26"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test Seperator") {
        const SourceFile rawFile = makeRawFile({"j ,"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "j"}, {TokenCategory::SEPERATOR, ","}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
    SECTION("Test String") {
        const SourceFile rawFile = makeRawFile({R"("'ello \n\"There\"")"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::STRING, R"('ello \n\"There\")"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Globl") {
        const SourceFile rawFile = makeRawFile({".globl label"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::META_DIRECTIVE, "globl"}, {TokenCategory::LABEL_REF, "label"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Eqv") {
        const SourceFile rawFile = makeRawFile({".eqv exit li $v0, 10", "exit"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::META_DIRECTIVE, "eqv"},
                                                           {TokenCategory::LABEL_REF, "exit"},
                                                           {TokenCategory::INSTRUCTION, "li"},
                                                           {TokenCategory::REGISTER, "v0"},
                                                           {TokenCategory::SEPERATOR, ","},
                                                           {TokenCategory::IMMEDIATE, "10"}},
                                                          {{TokenCategory::LABEL_REF, "exit"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Macro Parameters") {
        const SourceFile rawFile = makeRawFile({".macro foobar(%foo, %bar)"});
        std::vector<LineTokens> actualTokens = Tokenizer::tokenizeFile({rawFile});
        std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::META_DIRECTIVE, "macro"},
                                                           {TokenCategory::LABEL_REF, "foobar"},
                                                           {TokenCategory::OPEN_PAREN, "("},
                                                           {TokenCategory::MACRO_PARAM, "foo"},
                                                           {TokenCategory::SEPERATOR, ","},
                                                           {TokenCategory::MACRO_PARAM, "bar"},
                                                           {TokenCategory::CLOSE_PAREN, ")"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
}


TEST_CASE("Test Base Addressing") {
    SourceFile rawFile = makeRawFile({"lw $t1, 8($t0)"});
    std::vector<LineTokens> actualTokens = Tokenizer::tokenize({rawFile});
    std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "lw"},
                                                       {TokenCategory::REGISTER, "t1"},
                                                       {TokenCategory::SEPERATOR, ","},
                                                       {TokenCategory::REGISTER, "t0"},
                                                       {TokenCategory::SEPERATOR, ","},
                                                       {TokenCategory::IMMEDIATE, "8"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));

    rawFile = makeRawFile({"lw $t1, ($t0)"});
    actualTokens = Tokenizer::tokenize({rawFile});
    expectedTokens = {{{TokenCategory::INSTRUCTION, "lw"},
                       {TokenCategory::REGISTER, "t1"},
                       {TokenCategory::SEPERATOR, ","},
                       {TokenCategory::REGISTER, "t0"},
                       {TokenCategory::SEPERATOR, ","},
                       {TokenCategory::IMMEDIATE, "0"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Eqv") {
    const SourceFile rawFile = makeRawFile({".eqv exit li $v0, 10", "exit"});
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenize({rawFile});
    const std::vector<std::vector<Token>> expectedTokens = {{{TokenCategory::INSTRUCTION, "li"},
                                                             {TokenCategory::REGISTER, "v0"},
                                                             {TokenCategory::SEPERATOR, ","},
                                                             {TokenCategory::IMMEDIATE, "10"}}};
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenize Macro") {
    SECTION("Test Macro without Parameters") {
        const SourceFile rawFile =
                makeRawFile({".macro done", "li $v0, 10", "syscall", ".end_macro", "done"});
        const std::vector<LineTokens> actualTokens = Tokenizer::tokenize({rawFile});
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "li"},
                 {TokenCategory::REGISTER, "v0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "10"}},
                {{TokenCategory::INSTRUCTION, "syscall"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }

    SECTION("Test Macro with Parameters") {
        const SourceFile rawFile =
                makeRawFile({".macro terminate(%termination_value)", "li $a0, %termination_value",
                             "li $v0, 17", "syscall", ".end_macro", "terminate(1)"});
        const std::vector<LineTokens> actualTokens = Tokenizer::tokenize({rawFile});
        const std::vector<std::vector<Token>> expectedTokens = {
                {{TokenCategory::INSTRUCTION, "li"},
                 {TokenCategory::REGISTER, "a0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "1"}},
                {{TokenCategory::INSTRUCTION, "li"},
                 {TokenCategory::REGISTER, "v0"},
                 {TokenCategory::SEPERATOR, ","},
                 {TokenCategory::IMMEDIATE, "17"}},
                {{TokenCategory::INSTRUCTION, "syscall"}}};
        REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
    }
}


TEST_CASE("Test Tokenize Include") {
    const std::vector<SourceFile> rawFile = {{"a.asm", "jr $t0\n.include \"b.asm\"\n jr $t2"},
                                             {"b.asm", "label:\njr $t1"}};
    const std::vector<LineTokens> actualTokens = Tokenizer::tokenize(rawFile);
    const std::vector<std::vector<Token>> expectedTokens = {
            {{TokenCategory::INSTRUCTION, "jr"}, {TokenCategory::REGISTER, "t0"}},
            {{TokenCategory::LABEL_DEF, "label@masm_mangle_file_a.asm"}},
            {{TokenCategory::INSTRUCTION, "jr"}, {TokenCategory::REGISTER, "t1"}},
            {{TokenCategory::INSTRUCTION, "jr"}, {TokenCategory::REGISTER, "t2"}},
            {{TokenCategory::LABEL_DEF, "label@masm_mangle_file_b.asm"}},
            {{TokenCategory::INSTRUCTION, "jr"}, {TokenCategory::REGISTER, "t1"}},
    };
    REQUIRE_NOTHROW(validateTokenLines(expectedTokens, actualTokens));
}


TEST_CASE("Test Tokenizer Syntax Errors") {
    SECTION("Test Misplaced Quote") {
        const SourceFile rawFile = makeRawFile({R"(g"hello")"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token 'g'"));
    }

    SECTION("Test Unclosed Quote") {
        const SourceFile rawFile = makeRawFile({R"("hello)"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Unexpected EOL while parsing token 'hello '"));
    }

    SECTION("Test Unexpected First Token") {
        SourceFile rawFile = makeRawFile({","});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token ','"));

        rawFile = makeRawFile({"("});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token '('"));

        rawFile = makeRawFile({")"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token ')'"));

        rawFile = makeRawFile({":"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token ':'"));
    }

    SECTION("Test Undeclared Global Label") {
        const SourceFile rawFile = makeRawFile({".globl invalid"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Global label 'invalid' "
                                         "referenced without declaration"));
    }

    SECTION("Test Invalid Global Label") {
        SourceFile rawFile = makeRawFile({".globl"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Invalid global label declaration"));

        rawFile = makeRawFile({".globl 1"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Invalid global label declaration"));
    }

    SECTION("Test Invalid Eqv") {
        SourceFile rawFile = makeRawFile({".eqv"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid eqv declaration"));

        rawFile = makeRawFile({".eqv hello"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid eqv declaration"));

        rawFile = makeRawFile({".eqv 1 1"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid eqv declaration"));
    }

    SECTION("Test Base Addressing") {
        SourceFile rawFile = makeRawFile({"($t0)"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unexpected token '('"));

        rawFile = makeRawFile({"lw $s1 2($t0"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Malformed parenthesis expression"));

        rawFile = makeRawFile({"lw $s1 2()"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Malformed parenthesis expression"));
    }

    SECTION("Test Malformed Macro Params") {
        SourceFile rawFile = makeRawFile({".macro macro %arg"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Malformed macro parameter declaration"));

        rawFile = makeRawFile({".macro macro (%arg"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:1 -> Malformed macro parameter declaration"));
    }

    SECTION("Test Malformed Macro Call") {
        SourceFile rawFile = makeRawFile(
                {".macro macro(%arg)", "addi $t0, $zero, %arg", ".end_macro", "macro(0, 1)"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:4 -> Invalid number of macro arguments"));

        rawFile = makeRawFile(
                {".macro macro(%arg)", "addi $t0, $zero, %bargg", ".end_macro", "macro(0)"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message(
                        "Syntax error at a.asm:2 -> Invalid macro parameter 'bargg'"));
    }

    SECTION("Test Malformed Macro Declaration") {
        SourceFile rawFile = makeRawFile({".macro"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid macro declaration"));

        rawFile = makeRawFile({".macro 1"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid macro declaration"));

        rawFile = makeRawFile({".macro macro(%arg)", "addi $t0, $zero, %bargg"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Unmatched macro declaration"));
    }

    SECTION("Test Invalid Include") {
        SourceFile rawFile = makeRawFile({".include"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid include directive"));

        rawFile = makeRawFile({".include 1"});
        REQUIRE_THROWS_MATCHES(
                Tokenizer::tokenize({rawFile}), MasmSyntaxError,
                Catch::Matchers::Message("Syntax error at a.asm:1 -> Invalid include directive"));
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


TEST_CASE("Test Tokenize Echo Interrupt") {
    const std::string test_case = "echointer";
    validateTokens({"test/fixtures/" + test_case + "/" + test_case + ".asm"},
                   "test/fixtures/" + test_case + "/" + test_case + ".tkn");
}
