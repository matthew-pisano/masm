//
// Created by matthew on 5/22/25.
//

#include "testing_utilities.h"

#include <catch2/catch_test_macros.hpp>


std::vector<std::byte> iV2bV(const std::vector<uint8_t>& intVec) {
    std::vector<std::byte> byteVec(intVec.size());
    for (size_t i = 0; i < intVec.size(); ++i)
        byteVec[i] = static_cast<std::byte>(intVec[i]);
    return byteVec;
}


std::vector<uint8_t> bV2iV(const std::vector<std::byte>& byteVec) {
    std::vector<uint8_t> intVec(byteVec.size());
    for (size_t i = 0; i < byteVec.size(); ++i)
        intVec[i] = static_cast<uint8_t>(byteVec[i]);
    return intVec;
}


SourceFile makeRawFile(const std::vector<std::string>& lines) {
    std::string source;
    for (const std::string& line : lines) {
        if (!source.empty())
            source += '\n';
        source += line;
    }

    return {"a.asm", source};
}


void validateTokenLines(const std::vector<std::vector<Token>>& expectedTokens,
                        const std::vector<LineTokens>& actualTokens) {
    if (expectedTokens.size() != actualTokens.size())
        throw std::runtime_error("Expected " + std::to_string(expectedTokens.size()) +
                                 " tokens, but got " + std::to_string(actualTokens.size()));
    for (size_t i = 0; i < expectedTokens.size(); ++i)
        for (size_t j = 0; j < expectedTokens[i].size(); ++j)
            if (expectedTokens[i][j] != actualTokens[i].tokens[j])
                throw std::runtime_error("Expected token " + std::to_string(i) + " to be " +
                                         expectedTokens[i][j].value + ", but got " +
                                         actualTokens[i].tokens[j].value);
}
