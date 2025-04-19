//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>

#include "parser.h"


TEST_CASE("Test Parse Labels") {
    Parser parser{};
    const std::vector<std::vector<Token>> program = {
            // .data
            {{TokenType::MEMDIRECTIVE, "data"}},
            // label: .asciiz "hello"
            {{TokenType::LABEL, "label"}},
            {{TokenType::DIRECTIVE, "asciiz"}, {TokenType::STRING, "hello"}},
            // .text
            {{TokenType::MEMDIRECTIVE, "text"}},
            // la $s1, label
            {{TokenType::INSTRUCTION, "la"},
             {TokenType::REGISTER, "s1"},
             {TokenType::SEPERATOR, ","},
             {TokenType::LABELREF, "label"}}};
    MemLayout actualMem = parser.parse(program);
    MemLayout expectedMem = {{MemSection::DATA, {'h', 'e', 'l', 'l', 'o'}},
                             {MemSection::TEXT,
                              {// lui $at, 0x00001001
                               0x3c, 0x01, 0x10, 0x01,
                               // ori $s1, $at, 0x00000000
                               0x34, 0x31, 0x00, 0x00}}};
    REQUIRE(expectedMem == actualMem);
}


TEST_CASE("Test Parse Arithmetic") {
    Parser parser{};
    const std::vector<std::vector<Token>> program = {
            // .data
            {{TokenType::MEMDIRECTIVE, "data"}},
            // small: .word 0
            {{TokenType::LABEL, "small"}},
            {{TokenType::DIRECTIVE, "word"}, {TokenType::IMMEDIATE, "0"}},
            // large: .word 65536
            {{TokenType::LABEL, "large"}},
            {{TokenType::DIRECTIVE, "word"}, {TokenType::IMMEDIATE, "65536"}},
            // .text
            {{TokenType::MEMDIRECTIVE, "text"}},
            // main: la $t0, large
            {{TokenType::LABEL, "main"}},
            {{TokenType::INSTRUCTION, "la"},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::LABELREF, "large"}},
            // lw $t1, ($t0)
            {{TokenType::INSTRUCTION, "lw"},
             {TokenType::REGISTER, "t1"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "0"}},
            // addi $t2, $t1, 1024
            {{TokenType::INSTRUCTION, "addi"},
             {TokenType::REGISTER, "t2"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t1"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "1024"}},
            // sub $t2, $t1, $t0
            {{TokenType::INSTRUCTION, "sub"},
             {TokenType::REGISTER, "t2"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t2"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t0"}}};
    MemLayout actualMem = parser.parse(program);
    MemLayout expectedMem = {{MemSection::DATA, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00}},
                             {MemSection::TEXT,
                              {// lui $at, 0x00001001
                               0x3c, 0x01, 0x10, 0x01,
                               // ori $t0, $at, 0x00000004
                               0x34, 0x28, 0x00, 0x04,
                               // lw $t1, 0x00000000($t0)
                               0x8d, 0x09, 0x00, 0x00,
                               // addi $t2, $t1, 0x00000400
                               0x21, 0x2a, 0x04, 0x00,
                               // sub $t2, $t2, $t0
                               0x01, 0x48, 0x50, 0x22}}};
    REQUIRE(expectedMem == actualMem);
}


TEST_CASE("Test Parse Syscall") {
    Parser parser{};
    const std::vector<std::vector<Token>> program = {
            // .data
            {{TokenType::MEMDIRECTIVE, "data"}},
            // string: .asciiz "hello"
            {{TokenType::LABEL, "string"}},
            {{TokenType::DIRECTIVE, "asciiz"}, {TokenType::STRING, "hello"}},
            // .text
            {{TokenType::MEMDIRECTIVE, "text"}},
            // main: li $v0, 4
            {{TokenType::LABEL, "main"}},
            {{TokenType::INSTRUCTION, "li"},
             {TokenType::REGISTER, "v0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "4"}},
            // la $a0, string
            {{TokenType::INSTRUCTION, "la"},
             {TokenType::REGISTER, "a0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::LABELREF, "string"}},
            // syscall
            {{TokenType::INSTRUCTION, "syscall"}}};
    MemLayout actualMem = parser.parse(program);
    MemLayout expectedMem = {{MemSection::DATA, {'h', 'e', 'l', 'l', 'o'}},
                             {MemSection::TEXT,
                              {// addiu $v0, $zero, 0x00000004
                               0x24, 0x02, 0x00, 0x04,
                               // lui $at, 0x00001001
                               0x3c, 0x01, 0x10, 0x01,
                               // ori $a0, $at, 0x00000000
                               0x34, 0x24, 0x00, 0x00,
                               // syscall
                               0x00, 0x00, 0x00, 0x0c}}};
    REQUIRE(expectedMem == actualMem);
}


TEST_CASE("Test Parse Loops") {
    Parser parser{};
    const std::vector<std::vector<Token>> program = {
            // main: li $t1, 24
            {{TokenType::LABEL, "main"}},
            {{TokenType::INSTRUCTION, "li"},
             {TokenType::REGISTER, "t1"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "24"}},
            // li $t0, -1
            {{TokenType::INSTRUCTION, "li"},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "-1"}},
            // addi $t0, $t0, 1
            {{TokenType::LABEL, "loop"}},
            {{TokenType::INSTRUCTION, "addi"},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "1"}},
            // bgt $t0, $t1, exit
            {{TokenType::INSTRUCTION, "bgt"},
             {TokenType::REGISTER, "t0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::REGISTER, "t1"},
             {TokenType::SEPERATOR, ","},
             {TokenType::LABELREF, "exit"}},
            // j loop
            {{TokenType::INSTRUCTION, "j"}, {TokenType::LABELREF, "loop"}},
            // exit: li $v0, 10
            {{TokenType::LABEL, "exit"}},
            {{TokenType::INSTRUCTION, "li"},
             {TokenType::REGISTER, "v0"},
             {TokenType::SEPERATOR, ","},
             {TokenType::IMMEDIATE, "10"}},
            // syscall
            {{TokenType::INSTRUCTION, "syscall"}}};
    MemLayout actualMem = parser.parse(program);
    MemLayout expectedMem = {{MemSection::TEXT,
                              {// addiu $t1, 0x00000018
                               0x24, 0x09, 0x00, 0x18,
                               // addiu $t0, $zero, 0xffffffff
                               0x24, 0x08, 0xff, 0xff,
                               // addi $t0, $t0, 0x00000001
                               0x21, 0x08, 0x00, 0x01,
                               // slt $at, $t1, $t0
                               0x01, 0x28, 0x08, 0x2a,
                               // bne $at, $zero, 0x00000001
                               0x14, 0x20, 0x00, 0x01,
                               // j 0x00400008
                               0x08, 0x10, 0x00, 0x02,
                               // addiu $v0, $zero, 0x0000000a
                               0x24, 0x02, 0x00, 0x0a,
                               // syscall
                               0x00, 0x00, 0x00, 0x0c}}};
    REQUIRE(expectedMem == actualMem);
}
