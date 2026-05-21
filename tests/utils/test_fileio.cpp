//
// Created by matthew on 8/9/25.
//


#include <catch2/catch_test_macros.hpp>

#include <masm/io/fileio.h>


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
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/*.asm"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.asm"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));

        inputPaths = {"tests/fixtures/arithmetic/*"};
        expectedPaths = {"tests/fixtures/arithmetic/arithmetic.asm", "tests/fixtures/arithmetic/arithmetic.pse",
                         "tests/fixtures/arithmetic/arithmetic.tkn", "tests/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }
    SECTION("Test Multiple Wildcard") {
        std::vector<std::string> inputPaths = {"tests/fixtures/arithmetic/*.t*"};
        std::vector<std::string> expectedPaths = {"tests/fixtures/arithmetic/arithmetic.tkn",
                                                  "tests/fixtures/arithmetic/arithmetic.txt"};
        REQUIRE(expectedPaths == resolveWildcards(inputPaths));
    }
}
