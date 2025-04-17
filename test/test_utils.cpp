//
// Created by matthew on 4/16/25.
//


#include <catch2/catch_test_macros.hpp>

#include "utils.h"


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
