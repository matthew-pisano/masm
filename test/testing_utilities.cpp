//
// Created by matthew on 5/22/25.
//

#include "testing_utilities.h"

#include <catch2/catch_test_macros.hpp>


std::vector<std::byte> intVec2ByteVec(const std::vector<int>& intVec) {
    std::vector<std::byte> byteVec(intVec.size());
    for (size_t i = 0; i < intVec.size(); ++i)
        byteVec[i] = static_cast<std::byte>(intVec[i]);
    return byteVec;
}


RawFile makeRawFile(const std::vector<std::string>& lines) { return {"a.asm", lines}; }


void validateTokenLines(const std::vector<std::vector<Token>>& expectedTokens,
                        const std::vector<SourceLine>& actualTokens) {
    if (expectedTokens.size() != actualTokens.size())
        throw std::runtime_error("Expected " + std::to_string(expectedTokens.size()) +
                                 " tokens, but got " + std::to_string(actualTokens.size()));
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        if (expectedTokens[i] != actualTokens[i].tokens)
            throw std::runtime_error("Expected tokens " + std::to_string(i) + " to be " +
                                     expectedTokens[i][0].value + ", but got " +
                                     actualTokens[i].tokens[0].value);
}
