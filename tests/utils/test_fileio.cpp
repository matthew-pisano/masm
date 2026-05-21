//
// Created by matthew on 8/9/25.
//


#include <catch2/catch_test_macros.hpp>

#include "shared/fileio.h"


TEST_CASE("Test Wild Card Resolution") {
    SECTION("Test Generic Strings") {
        std::string test = "";
        REQUIRE(test == resolveWildcards({test})[0]);
        test = "hello there";
        REQUIRE(test == resolveWildcards({test})[0]);
        test = "hello/there.kenobi";
        REQUIRE(test == resolveWildcards({test})[0]);
    }

    SECTION("Test Tilde Resolution") {
        std::string inputPath = "~/hello/there";
        std::vector<std::string> expectedPaths = {std::string(getenv("HOME")) + "/hello/there"};
        REQUIRE(expectedPaths == resolveWildcards({inputPath}));
    }

    SECTION("Test Single Wildcard") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/*.asm"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.asm"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));

        inputPaths = {"tests/fixtures/arithmetic/*"};
        expectedPaths = {"tests/fixtures/arithmetic/arithmetic.asm", "tests/fixtures/arithmetic/arithmetic.pse",
                         "tests/fixtures/arithmetic/arithmetic.tkn", "tests/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }

    SECTION("Test Multiple Wildcard") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/*.t*", "tests/fixtures/arithmetic/*.a*"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.tkn",
                                                  "tests/fixtures/arithmetic/arithmetic.txt",
                                                  "tests/fixtures/arithmetic/arithmetic.asm"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }

    SECTION("Test Question Mark Wildcard") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/arithmeti?.txt"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }

    SECTION("Test Mixed Wildcard") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/arithmeti?.t*"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.tkn",
                                                  "tests/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }

    SECTION("Test No Matches") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/arithmetic.g*"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.g*"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }
}
