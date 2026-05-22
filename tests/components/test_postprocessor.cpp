//
// Created by matthew on 5/20/26.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <masm/assembler/tokenizer.hpp>

#include "libmasm/src/assembler/directive.hpp"
#include "libmasm/src/assembler/postprocessor.hpp"


TEST_CASE("Test Filter Token List") {
    Token reg = {TokenCategory::REGISTER, "t0"};
    Token label = {TokenCategory::LABEL_REF, "label"};
    Token param = {TokenCategory::MACRO_PARAM, "param"};
    Token sep = {TokenCategory::SEPERATOR, ","};

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
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens), std::runtime_error, Catch::Matchers::Message("Unexpected ','"));

        tokens = {sep, reg};
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens), std::runtime_error, Catch::Matchers::Message("Unexpected ','"));

        tokens = {reg, sep};
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens), std::runtime_error,
                               Catch::Matchers::Message("Unexpected ',' after token 't0'"));

        tokens = {reg, sep, reg, sep};
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens), std::runtime_error,
                               Catch::Matchers::Message("Unexpected ',' after token 't0'"));
    }

    SECTION("Test Invalid Elements") {
        std::vector tokens = {reg, sep, reg};
        REQUIRE_NOTHROW(filterTokenList(tokens, {TokenCategory::REGISTER}));

        tokens = {reg, sep, label};
        REQUIRE_NOTHROW(filterTokenList(tokens, {TokenCategory::REGISTER, TokenCategory::LABEL_REF}));

        tokens = {reg, sep, label};
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens, {TokenCategory::REGISTER}), std::runtime_error,
                               Catch::Matchers::Message("Invalid token 'label' of type 'LABEL_REF'"));

        tokens = {reg, sep, label, sep, param};
        REQUIRE_THROWS_MATCHES(filterTokenList(tokens, {TokenCategory::REGISTER, TokenCategory::LABEL_REF}),
                               std::runtime_error,
                               Catch::Matchers::Message("Invalid token 'param' of type 'MACRO_PARAM'"));
    }
}


TEST_CASE("Test Token Type Match") {
    SECTION("Test Match") {
        std::vector<TokenCategory> pattern = {};
        std::vector<Token> tokens = {};
        REQUIRE(tokenCategoryMatch(pattern, tokens));

        pattern = {TokenCategory::REGISTER};
        tokens = {{TokenCategory::REGISTER, "reg"}};
        REQUIRE(tokenCategoryMatch(pattern, tokens));

        pattern = {TokenCategory::REGISTER, TokenCategory::REGISTER};
        tokens = {{TokenCategory::REGISTER, "reg"}, {TokenCategory::REGISTER, "reg2"}};
        REQUIRE(tokenCategoryMatch(pattern, tokens));

        pattern = {TokenCategory::REGISTER, TokenCategory::LABEL_REF, TokenCategory::IMMEDIATE};
        tokens = {{TokenCategory::REGISTER, "reg"},
                  {TokenCategory::LABEL_REF, "label"},
                  {TokenCategory::IMMEDIATE, "42"}};
        REQUIRE(tokenCategoryMatch(pattern, tokens));
    }

    SECTION("Test No Match") {
        std::vector pattern = {TokenCategory::REGISTER};
        std::vector<Token> tokens = {};
        REQUIRE_FALSE(tokenCategoryMatch(pattern, tokens));

        pattern = {};
        tokens = {{TokenCategory::REGISTER, "reg"}};
        REQUIRE_FALSE(tokenCategoryMatch(pattern, tokens));

        pattern = {TokenCategory::REGISTER};
        tokens = {{TokenCategory::LABEL_REF, "label"}};
        REQUIRE_FALSE(tokenCategoryMatch(pattern, tokens));

        pattern = {TokenCategory::REGISTER};
        tokens = {{TokenCategory::REGISTER, "reg"}, {TokenCategory::LABEL_REF, "label"}};
        REQUIRE_FALSE(tokenCategoryMatch(pattern, tokens));
    }
}


TEST_CASE("Test Escape String") {
    REQUIRE(escapeString("Hello") == "Hello");
    REQUIRE(escapeString(R"(Hello\r\nWorld)") == "Hello\r\nWorld");
    REQUIRE(escapeString(R"(Hello\tWorld)") == "Hello\tWorld");
    REQUIRE(escapeString(R"(\"Hello\\World\")") == "\"Hello\\World\"");
    REQUIRE(escapeString(R"(\a\b\f\v)") == "\a\b\f\v");
}
