//
// Created by matthew on 4/15/25.
//

#ifndef UTILS_H
#define UTILS_H
#include <cstdint>
#include <string>
#include <tokenizer.h>
#include <vector>

bool isSignedInteger(const std::string& str);

std::vector<uint8_t> stringToBytes(const std::string& string);

std::vector<uint8_t> intStringToBytes(const std::string& string);

std::vector<Token> filterList(const std::vector<Token>& listTokens);

bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens);

#endif // UTILS_H
