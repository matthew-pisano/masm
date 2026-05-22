//
// Created by matthew on 5/20/26.
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "libmasm/src/util/conversion.hpp"


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


TEST_CASE("Test i32 to Bytes") {
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
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
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
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
    std::vector expectedBytes = {std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
                                 std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00}};
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
