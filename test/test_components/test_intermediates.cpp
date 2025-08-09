//
// Created by matthew on 7/30/25.
//


#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include "../testing_utilities.h"
#include "debug/intermediates.h"
#include "interpreter/memory.h"
#include "io/fileio.h"
#include "runtime.h"


TEST_CASE("Test Stringify Layout") {
    const std::string fixturePath = "test/fixtures/hello_world/hello_world.asm";
    Parser parser;
    const MemLayout layout = loadLayoutFromSource({fixturePath}, parser);
    const std::string layoutString = stringifyLayout(layout, parser.getLabels());
    const std::string expectedString = readFile(fixturePath + ".i");

    REQUIRE(layoutString == expectedString);
}


TEST_CASE("Test Save Layout") {
    const MemLayout layout = {{{MemSection::TEXT, iV2bV({0x01, 0x02, 0x03})},
                               {MemSection::DATA, iV2bV({0x04, 0x05})},
                               {MemSection::KTEXT, iV2bV({0x07, 0x08, 0x09, 0x10, 0x11})},
                               {MemSection::KDATA, iV2bV({0x06})}},
                              {}};

    SECTION("All Sections") {
        const std::vector<std::byte> binary = saveLayout(layout);
        const std::vector<std::byte> expected = iV2bV({
                'M',  'A',  'S',  'M', // Binary identifier
                0x14, 0x00, 0x00, 0x00, // Text locator
                0x1C, 0x00, 0x00, 0x00, // Data locator
                0x24, 0x00, 0x00, 0x00, // KText locator
                0x30, 0x00, 0x00, 0x00, // KData locator
                0x03, 0x00, 0x00, 0x00, // Text size
                0x01, 0x02, 0x03, 0x00, // Text section
                0x02, 0x00, 0x00, 0x00, // Data size
                0x04, 0x05, 0x00, 0x00, // Data section
                0x05, 0x00, 0x00, 0x00, // KText size
                0x07, 0x08, 0x09, 0x10, 0x11, 0x00, 0x00, 0x00, // KText section
                0x01, 0x00, 0x00, 0x00, // KData size
                0x06, 0x00, 0x00, 0x00 // KData section
        });
        REQUIRE(expected == binary);
    }

    SECTION("Without Text") {
        const MemLayout noText = {{{MemSection::DATA, layout.data.at(MemSection::DATA)},
                                   {MemSection::KTEXT, layout.data.at(MemSection::KTEXT)},
                                   {MemSection::KDATA, layout.data.at(MemSection::KDATA)}},
                                  {}};
        const std::vector<std::byte> binary = saveLayout(noText);
        const std::vector<std::byte> expected = iV2bV({
                'M', 'A', 'S', 'M', // Binary identifier
                0x00, 0x00, 0x00, 0x00, // Text locator
                0x14, 0x00, 0x00, 0x00, // Data locator
                0x1C, 0x00, 0x00, 0x00, // KText locator
                0x28, 0x00, 0x00, 0x00, // KData locator
                // Text section missing
                0x02, 0x00, 0x00, 0x00, // Data size
                0x04, 0x05, 0x00, 0x00, // Data section
                0x05, 0x00, 0x00, 0x00, // KText size
                0x07, 0x08, 0x09, 0x10, 0x11, 0x00, 0x00, 0x00, // KText section
                0x01, 0x00, 0x00, 0x00, // KData size
                0x06, 0x00, 0x00, 0x00 // KData section
        });
        REQUIRE(expected == binary);
    }
}


TEST_CASE("Test Load Layout") {
    const std::vector<std::byte> binary = iV2bV({
            'M',  'A',  'S',  'M', // Binary identifier
            0x14, 0x00, 0x00, 0x00, // Text locator
            0x1C, 0x00, 0x00, 0x00, // Data locator
            0x24, 0x00, 0x00, 0x00, // KText locator
            0x30, 0x00, 0x00, 0x00, // KData locator
            0x03, 0x00, 0x00, 0x00, // Text size
            0x01, 0x02, 0x03, 0x00, // Text section
            0x02, 0x00, 0x00, 0x00, // Data size
            0x04, 0x05, 0x00, 0x00, // Data section
            0x05, 0x00, 0x00, 0x00, // KText size
            0x07, 0x08, 0x09, 0x10, 0x11, 0x00, 0x00, 0x00, // KText section
            0x01, 0x00, 0x00, 0x00, // KData size
            0x06, 0x00, 0x00, 0x00 // KData section
    });

    SECTION("All Sections") {
        const MemLayout layout = loadLayout(binary);
        const MemLayout expected = {{{MemSection::TEXT, iV2bV({0x01, 0x02, 0x03})},
                                     {MemSection::DATA, iV2bV({0x04, 0x05})},
                                     {MemSection::KTEXT, iV2bV({0x07, 0x08, 0x09, 0x10, 0x11})},
                                     {MemSection::KDATA, iV2bV({0x06})}},
                                    {}};
        REQUIRE(expected.data == layout.data);
    }

    SECTION("Without Text") {
        const std::vector<std::byte> noText = iV2bV({
                'M',  'A',  'S',  'M', // Binary identifier
                0x00, 0x00, 0x00, 0x00, // Text locator
                0x14, 0x00, 0x00, 0x00, // Data locator
                0x1C, 0x00, 0x00, 0x00, // KText locator
                0x28, 0x00, 0x00, 0x00, // KData locator
                0x02, 0x00, 0x00, 0x00, // Data size
                0x04, 0x05, 0x00, 0x00, // Data section
                0x05, 0x00, 0x00, 0x00, // KText size
                0x07, 0x08, 0x09, 0x10, 0x11, 0x00, 0x00, 0x00, // KText section
                0x01, 0x00, 0x00, 0x00, // KData size
                0x06, 0x00, 0x00, 0x00 // KData section
        });
        const MemLayout layout = loadLayout(noText);
        const MemLayout expected = {{{MemSection::DATA, iV2bV({0x04, 0x05})},
                                     {MemSection::KTEXT, iV2bV({0x07, 0x08, 0x09, 0x10, 0x11})},
                                     {MemSection::KDATA, iV2bV({0x06})}},
                                    {}};
        REQUIRE(expected.data == layout.data);
    }

    SECTION("Without Identifier") {
        std::vector<std::byte> malformed = iV2bV({'M', 'A', 'S'});
        REQUIRE_THROWS_MATCHES(loadLayout(malformed), std::runtime_error,
                               Catch::Matchers::Message("Invalid MASM binary format"));
        malformed = iV2bV({'M', 'A', 'S', 'A'});
        REQUIRE_THROWS_MATCHES(loadLayout(malformed), std::runtime_error,
                               Catch::Matchers::Message("Invalid MASM binary format"));
    }

    SECTION("Truncated") {
        std::vector<std::byte> malformed = iV2bV({'M', 'A', 'S', 'M', 0x00, 0x00, 0x00, 0x00});
        REQUIRE_THROWS_MATCHES(loadLayout(malformed), std::out_of_range,
                               Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring(
                                       "vector::_M_range_check")));
        malformed = iV2bV({
                'M',  'A',  'S',  'M', // Binary identifier
                0x00, 0x00, 0x00, 0x00, // Text locator
                0x14, 0x00, 0x00, 0x00, // Data locator
                0x1C, 0x00, 0x00, 0x00, // KText locator
                0x28, 0x00, 0x00, 0x00, // KData locator
                0x02, 0x00, // Data size (truncated)
        });
        REQUIRE_THROWS_MATCHES(loadLayout(malformed), std::out_of_range,
                               Catch::Matchers::MessageMatches(Catch::Matchers::ContainsSubstring(
                                       "vector::_M_range_check")));
    }
}
