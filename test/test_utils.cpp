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


TEST_CASE("Test Escape String") {
    REQUIRE(escapeString("Hello") == "Hello");
    REQUIRE(escapeString(R"(Hello\r\nWorld)") == "Hello\r\nWorld");
    REQUIRE(escapeString(R"(Hello\tWorld)") == "Hello\tWorld");
    REQUIRE(escapeString(R"(\"Hello\\World\")") == "\"Hello\\World\"");
    REQUIRE(escapeString(R"(\a\b\f\v)") == "\a\b\f\v");
}


TEST_CASE("Test String to Bytes") {
    SECTION("Test Single Char") {
        std::vector expectedBytes = {std::byte{0x61}, std::byte{0x00}};
        std::vector<std::byte> actualBytes = stringToBytes("a", true);
        REQUIRE(expectedBytes == actualBytes);
    }
    SECTION("Test Multiple Char") {
        std::vector expectedBytes = {std::byte{0x61}, std::byte{0x20}, std::byte{0x62},
                                     std::byte{0x47}, std::byte{0x2f}, std::byte{0x3f},
                                     std::byte{0x2e}, std::byte{0x31}, std::byte{0x00}};
        std::vector<std::byte> actualBytes = stringToBytes("a bG/?.1", true);
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

        pattern = {TokenType::REGISTER, TokenType::LABEL_REF, TokenType::IMMEDIATE};
        tokens = {{TokenType::REGISTER, "reg"},
                  {TokenType::LABEL_REF, "label"},
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
        tokens = {{TokenType::LABEL_REF, "label"}};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));

        pattern = {TokenType::REGISTER};
        tokens = {{TokenType::REGISTER, "reg"}, {TokenType::LABEL_REF, "label"}};
        REQUIRE_FALSE(tokenTypeMatch(pattern, tokens));
    }
}


TEST_CASE("Test i32 to Bytes") {
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                 std::byte{0x00}};
    std::vector<std::byte> actualBytes = i32ToBEByte(0);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x01}};
    actualBytes = i32ToBEByte(1);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x00}, std::byte{0x05}, std::byte{0x7c}, std::byte{0x3a}};
    actualBytes = i32ToBEByte(359482);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}};
    actualBytes = i32ToBEByte(-1);
    REQUIRE(expectedBytes == actualBytes);
}


TEST_CASE("Test f32 to Bytes") {
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                 std::byte{0x00}};
    std::vector<std::byte> actualBytes = f32ToBEByte(0.0f);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x3f}, std::byte{0x80}, std::byte{0x00}, std::byte{0x00}};
    actualBytes = f32ToBEByte(1.0f);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x41}, std::byte{0x24}, std::byte{0x00}, std::byte{0x00}};
    actualBytes = f32ToBEByte(10.25f);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0xbf}, std::byte{0xc5}, std::byte{0x1e}, std::byte{0xb8}};
    actualBytes = f32ToBEByte(-1.54f);
    REQUIRE(expectedBytes == actualBytes);
}


TEST_CASE("Test f64 to Bytes") {
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                 std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                 std::byte{0x00}, std::byte{0x00}};
    std::vector<std::byte> actualBytes = f64ToBEByte(0.0);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x3f}, std::byte{0xf8}, std::byte{0x00}, std::byte{0x00},
                     std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
    actualBytes = f64ToBEByte(1.5);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0x40}, std::byte{0x24}, std::byte{0x80}, std::byte{0x00},
                     std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
    actualBytes = f64ToBEByte(10.25);
    REQUIRE(expectedBytes == actualBytes);

    expectedBytes = {std::byte{0xbf}, std::byte{0xf8}, std::byte{0xa3}, std::byte{0xd7},
                     std::byte{0x0a}, std::byte{0x3d}, std::byte{0x70}, std::byte{0xa4}};
    actualBytes = f64ToBEByte(-1.54);
    REQUIRE(expectedBytes == actualBytes);
}
