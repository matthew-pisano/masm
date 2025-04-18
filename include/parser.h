//
// Created by matthew on 4/14/25.
//

#ifndef PARSER_H
#define PARSER_H
#include <cstdint>
#include <map>

#include "memory.h"
#include "tokenizer.h"


using MemLayout = std::map<MemSection, std::vector<uint8_t>>;


class Parser {

    std::map<std::string, uint32_t> labelMap;
    MemLayout memory;

    MemLayout parse(const std::vector<std::vector<Token>>& tokens);

    void resolveLabels(std::vector<Token>& instructionArgs);

    static std::vector<uint8_t> parseDirective(const Token& dirToken,
                                               const std::vector<Token>& args);

    std::vector<uint8_t> parseInstruction(const Token& instrToken, std::vector<Token>& args);

    static std::vector<uint8_t> parseRTypeInstruction(uint32_t rs, uint32_t rt, uint32_t rd,
                                                      uint32_t shamt, uint32_t funct);

    static std::vector<uint8_t> parseITypeInstruction(uint32_t opcode, uint32_t rs, uint32_t rt,
                                                      uint32_t immediate);

    static std::vector<uint8_t> parseJTypeInstruction(uint32_t opcode, uint32_t address);
};

#endif // PARSER_H
