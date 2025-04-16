//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include <map>
#include <optional>

#include "tokenizer.h"

std::vector<uint8_t> stringToBytes(const std::string& string);


std::vector<uint8_t> intStringToBytes(const std::string& string);


enum class MemSection { DATA, TEXT };


MemSection nameToMemSection(const std::string& name);

using MemLayout = std::map<MemSection, std::vector<uint8_t>>;


std::vector<Token> filterList(const std::vector<Token>& listTokens);


bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens);


class Parser {

    static MemLayout parse(const std::vector<std::vector<Token>>& tokens);

    static std::vector<uint8_t> parseDirective(const std::vector<Token>& dirTokens);

    static std::vector<uint8_t> parseInstruction(const std::vector<Token>& instrTokens);
};

#endif // PARSER_H
