//
// Created by matthew on 4/26/25.
//

#include "parser/labels.h"

#include <algorithm>
#include <stdexcept>

#include "exceptions.h"
#include "interpreter/memory.h"
#include "parser/directive.h"
#include "parser/instruction.h"
#include "utils.h"


void LabelMap::resolveLabels(std::vector<Token>& instructionArgs) {
    for (Token& arg : instructionArgs)
        if (arg.type == TokenType::LABEL_REF) {
            if (!labelMap.contains(arg.value))
                throw std::runtime_error("Unknown label '" + arg.value + "'");
            arg = {TokenType::IMMEDIATE, std::to_string(labelMap[arg.value])};
        }
}


std::string LabelMap::lookupLabel(const uint32_t address) const {
    for (const std::pair<const std::string, uint32_t>& pair : labelMap) {
        if (pair.second == address)
            return pair.first;
    }
    throw std::runtime_error("No label found for address " + std::to_string(address));
}


void LabelMap::populateLabelMap(const std::vector<LineTokens>& tokens) {
    MemSection currSection = MemSection::TEXT;
    std::map<MemSection, uint32_t> memSizes = {{currSection, 0}};
    std::vector<std::string> pendingLabels;

    for (const LineTokens& line : tokens) {
        if (line.tokens.empty())
            continue;

        try {
            const Token& firstToken = line.tokens[0];
            const std::vector unfilteredArgs(line.tokens.begin() + 1, line.tokens.end());
            std::vector<Token> args = filterTokenList(unfilteredArgs);
            switch (firstToken.type) {
                case TokenType::SEC_DIRECTIVE: {
                    currSection = nameToMemSection(firstToken.value);
                    // Init memory section if it is not already in the mapping
                    if (!memSizes.contains(currSection))
                        memSizes[currSection] = 0;
                    break;
                }
                case TokenType::ALLOC_DIRECTIVE: {
                    const std::tuple<std::vector<std::byte>, size_t> alloc =
                            parsePaddedAllocDirective(memSizes[currSection], firstToken, args);
                    // Assign labels to the following byte allocation plus the section offset
                    for (const std::string& label : pendingLabels)
                        labelMap[label] = memSectionOffset(currSection) + memSizes[currSection] +
                                          std::get<1>(alloc);
                    pendingLabels.clear();
                    memSizes[currSection] += std::get<0>(alloc).size();
                    break;
                }
                case TokenType::INSTRUCTION:
                    // Assign labels to the following byte allocation plus the section offset
                    for (const std::string& label : pendingLabels)
                        labelMap[label] = memSectionOffset(currSection) + memSizes[currSection];
                    pendingLabels.clear();
                    // Get size of instruction from map without parsing
                    memSizes[currSection] += nameToInstructionOp(firstToken.value).size;
                    break;
                case TokenType::LABEL_DEF: {
                    if (labelMap.contains(firstToken.value) ||
                        std::ranges::find(pendingLabels, firstToken.value) != pendingLabels.end())
                        throw std::runtime_error("Duplicate label '" + firstToken.value + "'");
                    // Add to pending labels (address resolved to next instruction/directive)
                    pendingLabels.push_back(firstToken.value);
                    break;
                }
                default:
                    break;
            }
        } catch (const std::runtime_error& e) {
            throw MasmSyntaxError(e.what(), line.filename, line.lineno);
        }
    }
}
