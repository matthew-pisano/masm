//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include <map>
#include <optional>

#include "memory.h"
#include "tokenizer.h"

std::vector<uint8_t> stringToBytes(const std::string& string);


std::vector<uint8_t> intStringToBytes(const std::string& string);


using MemLayout = std::map<MemSection, std::vector<uint8_t>>;


std::vector<Token> filterList(const std::vector<Token>& listTokens);


bool tokenTypeMatch(const std::vector<TokenType>& pattern, const std::vector<Token>& tokens);


class Parser {

    std::map<std::string, uint32_t> labelMap;
    MemLayout memory;

    MemLayout parse(const std::vector<std::vector<Token>>& tokens);

    std::vector<uint8_t> parseDirective(const std::vector<Token>& dirTokens);

    std::vector<uint8_t> parseInstruction(const std::vector<Token>& instrTokens);

    static std::vector<uint8_t> parseRTypeInstruction(uint32_t opcode, uint32_t rs, uint32_t rt,
                                                      uint32_t rd, uint32_t shamt, uint32_t funct);

    static std::vector<uint8_t> parseITypeInstruction(uint32_t opcode, uint32_t rs, uint32_t rt,
                                                      uint32_t immediate);

    static std::vector<uint8_t> parseJTypeInstruction(uint32_t opcode, uint32_t address);
};

#endif // PARSER_H
