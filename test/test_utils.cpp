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
        std::vector<uint8_t> expectedBytes = {0x61};
        std::vector<uint8_t> actualBytes = stringToBytes("a");
        REQUIRE(expectedBytes == actualBytes);
    }
    SECTION("Test Multiple Char") {
        std::vector<uint8_t> expectedBytes = {0x61, 0x20, 0x62, 0x47, 0x2f, 0x3f, 0x2e, 0x31};
        std::vector<uint8_t> actualBytes = stringToBytes("a bG/?.1");
        REQUIRE(expectedBytes == actualBytes);
    }
}


TEST_CASE("Test Integer String to Bytes") {
    SECTION("Test Positive") {
        std::vector<uint8_t> expectedBytes = {0x00, 0x00, 0x00, 0x00};
        std::vector<uint8_t> actualBytes = intStringToBytes("0");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {0x00, 0x00, 0x00, 0x01};
        actualBytes = intStringToBytes("1");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {0x00, 0x05, 0x7c, 0x3a};
        actualBytes = intStringToBytes("359482");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {0x7f, 0xff, 0xff, 0xff};
        actualBytes = intStringToBytes("2147483647");
        REQUIRE(expectedBytes == actualBytes);

        REQUIRE_THROWS(intStringToBytes("2147483648"));
    }

    SECTION("Test Negative") {
        std::vector<uint8_t> expectedBytes = {0xff, 0xff, 0xff, 0xff};
        std::vector<uint8_t> actualBytes = intStringToBytes("-1");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {0xff, 0xfa, 0x83, 0xc6};
        actualBytes = intStringToBytes("-359482");
        REQUIRE(expectedBytes == actualBytes);

        expectedBytes = {0x80, 0x00, 0x00, 0x00};
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
