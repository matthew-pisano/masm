//
// Created by matthew on 4/16/25.
//


#include <catch2/catch_test_macros.hpp>

#include "utils.h"


TEST_CASE("Test Is Signed Integer") {
    SECTION("Test Valid Integer") {
        std::string intString = "0";
        REQUIRE(isSignedInteger(intString));

        intString = "-5";
        REQUIRE(isSignedInteger(intString));

        intString = "3647";
        REQUIRE(isSignedInteger(intString));
    }

    SECTION("Test Invalid Integer") {
        std::string intString = "abdc";
        REQUIRE_FALSE(isSignedInteger(intString));

        intString = "-45.6";
        REQUIRE_FALSE(isSignedInteger(intString));

        intString = "abc123def";
        REQUIRE_FALSE(isSignedInteger(intString));
    }
}


TEST_CASE("Test String to Bytes") {
    SECTION("Test Single Char") {
        std::vector expectedBytes = {std::byte{0x61}};
        std::vector<std::byte> actualBytes = stringToBytes("a", true, true);
        REQUIRE(expectedBytes == actualBytes);
    }
    SECTION("Test Multiple Char") {
        std::vector expectedBytes = {std::byte{0x61}, std::byte{0x20}, std::byte{0x62},
                                     std::byte{0x47}, std::byte{0x2f}, std::byte{0x3f},
                                     std::byte{0x2e}, std::byte{0x31}};
        std::vector<std::byte> actualBytes = stringToBytes("a bG/?.1", true, true);
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test Integer String to Bytes") {
    SECTION("Test Positive") {
        std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                     std::byte{0x00}};
        std::vector<std::byte> actualBytes = intStringToBytes("0");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
        actualBytes = intStringToBytes("1");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {std::byte{0x00}, std::byte{0x05}, std::byte{0x7c}, std::byte{0x3a}};
        actualBytes = intStringToBytes("359482");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {std::byte{0x7f}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}};
        actualBytes = intStringToBytes("2147483647");
        REQUIRE(expectedBytes == actualBytes);

        REQUIRE_THROWS(intStringToBytes("2147483648"));
    }

    SECTION("Test Negative") {
        std::vector expectedBytes = {std::byte{0xff}, std::byte{0xff}, std::byte{0xff},
                                     std::byte{0xff}};
        std::vector<std::byte> actualBytes = intStringToBytes("-1");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {std::byte{0xff}, std::byte{0xfa}, std::byte{0x83}, std::byte{0xc6}};
        actualBytes = intStringToBytes("-359482");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {std::byte{0x80}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
        actualBytes = intStringToBytes("-2147483648");
        REQUIRE(expectedBytes == actualBytes);

        REQUIRE_THROWS(intStringToBytes("-2147483649"));
    }
}


TEST_CASE("Test Filter Token List") {
    Token reg = {TokenType::REGISTER, "reg"};
    Token sep = {TokenType::SEPERATOR, ","};

    SECTION("Test Valid Token Lists") {
        std::vector tokens = {reg};
        std::vector expectedTokens = {reg};
        std::vector<Token> actualTokens = filterTokenList(tokens);
        REQUIRE(expectedTokens == actualTokens);

        tokens = {reg, sep, reg, sep, reg};
        expectedTokens = {reg, reg, reg};
        actualTokens = filterTokenList(tokens);
        REQUIRE(expectedTokens == actualTokens);
    }

    SECTION("Test Invalid Token Lists") {
        std::vector tokens = {sep};
        REQUIRE_THROWS_AS(filterTokenList(tokens), std::runtime_error);

        tokens = {sep, reg};
        REQUIRE_THROWS_AS(filterTokenList(tokens), std::runtime_error);

        tokens = {reg, sep};
        REQUIRE_THROWS_AS(filterTokenList(tokens), std::runtime_error);

        tokens = {reg, sep, reg, sep};
        REQUIRE_THROWS_AS(filterTokenList(tokens), std::runtime_error);
    }
}


TEST_CASE("Test Token Type Match") {
    SECTION("Test Match") {
        std::vector<TokenType> pattern = {};
        std::vector<Token> tokens = {};
        REQUIRE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER};
        tokens = {{TokenType::REGISTER, "reg"}};
        REQUIRE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER, TokenType::REGISTER};
        tokens = {{TokenType::REGISTER, "reg"}, {TokenType::REGISTER, "reg2"}};
        REQUIRE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER, TokenType::LABELREF, TokenType::IMMEDIATE};
        tokens = {{TokenType::REGISTER, "reg"},
                  {TokenType::LABELREF, "label"},
                  {TokenType::IMMEDIATE, "42"}};
        REQUIRE(tokenTypeMatch(pattern, tokens));
    }

    SECTION("Test No Match") {
        std::vector pattern = {TokenType::REGISTER};
        std::vector<Token> tokens = {};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));

        pattern = {};
        tokens = {{TokenType::REGISTER, "reg"}};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER};
        tokens = {{TokenType::LABELREF, "label"}};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER};
        tokens = {{TokenType::REGISTER, "reg"}, {TokenType::LABELREF, "label"}};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));
    }
}
