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

    void resolveLabels(std::vector<Token>& instructionArgs);

    static std::vector<uint8_t> parseDirective(const Token& dirToken,
                                               const std::vector<Token>& args);

    std::vector<uint8_t> parseInstruction(uint32_t loc, const Token& instrToken,
                                          std::vector<Token>& args);

    static std::vector<uint8_t> parseRTypeInstruction(uint32_t rs, uint32_t rt, uint32_t rd,
                                                      uint32_t shamt, uint32_t funct);

    static std::vector<uint8_t> parseITypeInstruction(uint32_t opcode, uint32_t rs, uint32_t rt,
                                                      uint32_t immediate);

    static std::vector<uint8_t> parseShortITypeInstruction(uint32_t opcode, uint32_t rt,
                                                           uint32_t immediate);

    static std::vector<uint8_t> parseJTypeInstruction(uint32_t loc, uint32_t opcode,
                                                      uint32_t address);

    static std::vector<uint8_t> parseSyscallInstruction();

    std::vector<uint8_t> parsePseudoInstruction(uint32_t loc, const std::string& instructionName,
                                                std::vector<Token>& args);

    std::vector<uint8_t> parseBranchPseudoInstruction(uint32_t loc, const Token& reg1,
                                                      const Token& reg2, const Token& label,
                                                      bool checkLt, bool checkEq);

    void populateLabels(const std::vector<std::vector<Token>>& tokens);

public:
    MemLayout parse(const std::vector<std::vector<Token>>& tokens);
};

#endif // PARSER_H
