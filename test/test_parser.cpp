//
// Created by matthew on 4/18/25.
//


#include <catch2/catch_test_macros.hpp>

#include "parser.h"


TEST_CASE("Test Parse Labels") {
    SECTION("Test labeled String") {
        Parser parser{};
        const std::vector<std::vector<Token>> program = {
                {{TokenType::MEMDIRECTIVE, "data"}},
                {{TokenType::LABEL, "label"}},
                {{TokenType::DIRECTIVE, "asciiz"}, {TokenType::STRING, "hello"}},
                {{TokenType::MEMDIRECTIVE, "text"}},
                {{TokenType::INSTRUCTION, "la"},
                 {TokenType::REGISTER, "s1"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::LABELREF, "label"}}};
        MemLayout actualMem = parser.parse(program);
        MemLayout expectedMem = {
                {MemSection::DATA, {'h', 'e', 'l', 'l', 'o'}},
                {MemSection::TEXT, {0x3c, 0x01, 0x10, 0x01, 0x34, 0x31, 0x00, 0x00}}};
        REQUIRE(expectedMem == actualMem);
    }
}
