//
// Created by matthew on 5/22/25.
//

#include "testing_utilities.h"

#include <catch2/catch_test_macros.hpp>


LabelMap& DebugParser::getLabels() { return labelMap; }


State& DebugInterpreter::getState() { return state; }


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
        for (size_t j = 0; j < expectedTokens[i].size(); ++j)
            if (expectedTokens[i][j] != actualTokens[i].tokens[j])
                throw std::runtime_error("Expected token " + std::to_string(i) + " to be " +
                                         expectedTokens[i][j].value + ", but got " +
                                         actualTokens[i].tokens[j].value);
}
