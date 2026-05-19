//
// Created by matthew on 8/9/25.
//


#include <catch2/catch_test_macros.hpp>

#include "io/fileio.h"


TEST_CASE("Test Wild Card Resolution") {
    SECTION("Test Generic Strings") {
        std::string test = "";
        REQUIRE(test == resolveWildcards({test})[0]);
        test = "hello there";
        REQUIRE(test == resolveWildcards({test})[0]);
        test = "hello/there.kenobi";
        REQUIRE(test == resolveWildcards({test})[0]);
    }
    SECTION("Test Single Wildcard") {
        std::vector<std::string> inputPaths = {"test/fixtures/arithmetic/*.asm"};
        std::vector<std::string> expectedPaths = {"test/fixtures/arithmetic/arithmetic.asm"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));

        inputPaths = {"test/fixtures/arithmetic/*"};
        expectedPaths = {"test/fixtures/arithmetic/arithmetic.asm",
                         "test/fixtures/arithmetic/arithmetic.pse",
                         "test/fixtures/arithmetic/arithmetic.tkn",
                         "test/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }
    SECTION("Test Multiple Wildcard") {
        std::vector<std::string> inputPaths = {"test/fixtures/arithmetic/*.t*"};
        std::vector<std::string> expectedPaths = {"test/fixtures/arithmetic/arithmetic.tkn",
                                                  "test/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }
}
