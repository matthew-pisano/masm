//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include <map>
#include <optional>

#include "tokenizer.h"


struct Word {
    uint32_t data{};
    std::optional<std::string> label;
};


std::vector<Word> stringToWords(std::string string);


enum class MemSection { DATA, TEXT };


MemSection nameToMemSection(const std::string& name);

using MemLayout = std::map<MemSection, std::vector<Word>>;


std::vector<Token> filterList(const std::vector<Token>& listTokens);


class Parser {

    MemLayout parse(std::vector<std::vector<Token>> tokens);

    static std::vector<Word> parseDirective(const std::vector<Token>& dirTokens);

    Word parseInstruction(std::vector<Token> intrTokens);

    bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens);
};

#endif // PARSER_H
