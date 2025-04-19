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
                {{TokenType::DIRECTIVE, "asciiz"}, {TokenType::STRING, "hello there"}},
                {{TokenType::MEMDIRECTIVE, "text"}},
                {{TokenType::INSTRUCTION, "lui"},
                 {TokenType::REGISTER, "zero"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::REGISTER, "at"},
                 {TokenType::SEPERATOR, ","},
                 {TokenType::LABELREF, "label"}}};
        MemLayout actualMem = parser.parse(program);
        MemLayout expectedMem = {};
        REQUIRE(expectedMem == actualMem);
    }
}
