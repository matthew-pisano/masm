//
// Created by matthew on 4/14/25.
//


#include <catch2/catch_test_macros.hpp>

#include "tokenizer.h"

TEST_CASE("Test Single Tokens") {
    SECTION("Test Directive") {
        const std::string line = ".asciiz";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::DIRECTIVE, "asciiz"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Memory Directive") {
        const std::string line = ".data";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::MEMDIRECTIVE, "data"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Label Declaration") {
        const std::string line = "label:";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL, "label"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Label Reference") {
        const std::string line = "j label";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::LABELREF, "label"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Instruction") {
        const std::string line = "addi";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "addi"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Register") {
        const std::string line = "$v0";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::REGISTER, "v0"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Immediate") {
        std::string line = "42";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::IMMEDIATE, "42"}}};
        REQUIRE(expectedTokens == actualTokens);

        line = "-42";
        actualTokens = Tokenizer::tokenizeLine(line);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42"}}};
        REQUIRE(expectedTokens == actualTokens);

        line = "-42.0";
        actualTokens = Tokenizer::tokenizeLine(line);
        expectedTokens = {{{TokenType::IMMEDIATE, "-42.0"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Seperator") {
        const std::string line = ",";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::SEPERATOR, ","}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test String") {
        const std::string line = R"("'ello \n\"There\"")";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::STRING, R"('ello \n\"There\")"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
}


TEST_CASE("Test Lines") {
    SECTION("Test Labeled String") {
        const std::string line = R"(out_string: .asciiz "\nHello, World!\n")";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::LABEL, "out_string"}},
                {{TokenType::DIRECTIVE, "asciiz"}, {TokenType::STRING, R"(\nHello, World!\n)"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Immediate Pseudo-Instruction") {
        const std::string line = R"(li $v0, 4)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "li"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::IMMEDIATE, "4"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test I-Type Instruction") {
        const std::string line = R"(lw $v0, $t1)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "lw"},
                                                           {TokenType::REGISTER, "v0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::REGISTER, "t1"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test R-Type Instruction") {
        const std::string line = R"(add $t0, $t1, $t2)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "add"},
                                                           {TokenType::REGISTER, "t0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::REGISTER, "t1"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::REGISTER, "t2"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Labeled R-Type Instruction") {
        const std::string line = R"(label: add $t0, $t1, $t2)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::LABEL, "label"}},
                                                          {{TokenType::INSTRUCTION, "add"},
                                                           {TokenType::REGISTER, "t0"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::REGISTER, "t1"},
                                                           {TokenType::SEPERATOR, ","},
                                                           {TokenType::REGISTER, "t2"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test J-Type Instruction") {
        const std::string line = R"(j 2048)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::IMMEDIATE, "2048"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Syscall") {
        const std::string line = R"(syscall)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {{{TokenType::INSTRUCTION, "syscall"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Comment") {
        const std::string line = R"(# addi $v0, $t1, 200)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Inline Comment") {
        const std::string line = R"(j 2048 # addi $v0, $t1, 200)";
        std::vector<std::vector<Token>> actualTokens = Tokenizer::tokenizeLine(line);
        std::vector<std::vector<Token>> expectedTokens = {
                {{TokenType::INSTRUCTION, "j"}, {TokenType::IMMEDIATE, "2048"}}};
        REQUIRE(expectedTokens == actualTokens);
    }
    SECTION("Test Unexpected EOL") {
        const std::string line = R"(unterminated: .asciiz "incomplet)";
        REQUIRE_THROWS_AS(Tokenizer::tokenizeLine(line), std::runtime_error);
    }
}
