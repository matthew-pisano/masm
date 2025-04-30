//
// Created by matthew on 4/26/25.
//

#include "labels.h"

#include <stdexcept>

#include "instruction.h"
#include "memory.h"
#include "utils.h"


void LabelMap::resolveLabels(std::vector<Token>& instructionArgs) {
    for (Token& arg : instructionArgs)
        if (arg.type == TokenType::LABELREF) {
            if (!labelMap.contains(arg.value))
                throw std::runtime_error("Unknown label " + arg.value);
            arg = {TokenType::IMMEDIATE, std::to_string(labelMap[arg.value])};
        }
}


void LabelMap::populateLabelMap(const std::vector<std::vector<Token>>& tokens) {
    MemSection currSection = MemSection::TEXT;
    std::map<MemSection, uint32_t> memSizes = {{currSection, 0}};

    for (const std::vector<Token>& line : tokens) {
        if (line.empty())
            continue;

        const Token& firstToken = line[0];
        const std::vector unfilteredArgs(line.begin() + 1, line.end());
        std::vector<Token> args = filterTokenList(unfilteredArgs);
        switch (firstToken.type) {
            case TokenType::MEMDIRECTIVE: {
                currSection = nameToMemSection(firstToken.value);
                if (!memSizes.contains(currSection))
                    memSizes[currSection] = 0;
                break;
            }
            case TokenType::DIRECTIVE: {
                // Get size of affected memory without parsing
                if (firstToken.value == "space") {
                    if (args.size() != 1)
                        throw std::runtime_error("space directive expects exactly one argument");
                    memSizes[currSection] += std::stoi(args[0].value);
                    break;
                }

                uint32_t size = 0;
                for (const Token& arg : args) {
                    if (arg.type == TokenType::IMMEDIATE)
                        size += 4;
                    else if (arg.type == TokenType::STRING) {
                        size += arg.value.size();
                        if (firstToken.value == "asciiz")
                            size++; // padding for null terminator
                    } else
                        throw std::runtime_error("Invalid argument type for directive " +
                                                 firstToken.value);
                }
                memSizes[currSection] += size;
                break;
            }
            case TokenType::INSTRUCTION:
                // Get size of instruction from map without parsing
                memSizes[currSection] += nameToInstructionOp(firstToken.value).size;
                break;
            case TokenType::LABEL: {
                if (labelMap.contains(firstToken.value))
                    throw std::runtime_error("Duplicate label " + firstToken.value);
                // Assign label to the following byte allocation plus the section offset
                labelMap[firstToken.value] = memSectionOffset(currSection) + memSizes[currSection];
                break;
            }
            default:
                break;
        }
    }
}
