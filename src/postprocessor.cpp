//
// Created by matthew on 5/9/25.
//

#include "postprocessor.h"

#include <stdexcept>
#include <unordered_map>

#include "utils.h"


void Postprocessor::mangleLabels(
        std::map<std::string, std::vector<std::vector<Token>>>& programMap) {

    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap) {
        if (programFile.first.empty())
            throw std::runtime_error("File ID is empty");
        collectGlobals(programFile.second);
    }

    // Stores globals that have not yet been matched to declarations
    std::vector availableLabels(globals);
    // Mangle labels and resolve globals
    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap) {
        for (std::vector<Token>& line : programFile.second)
            // Mangle the labels in the line
            mangleLabelsInLine(availableLabels, line, programFile.first);
    }

    // If any globals were declared without a matching label declaration
    if (!availableLabels.empty())
        throw std::runtime_error("Global label " + availableLabels[0] +
                                 " referenced without declaration");
}


void Postprocessor::mangleLabelsInLine(std::vector<std::string>& availableLabels,
                                       std::vector<Token>& lineTokens, const std::string& fileId) {
    for (Token& lineToken : lineTokens) {
        if (lineToken.type != TokenType::LABEL_DEF && lineToken.type != TokenType::LABEL_REF)
            continue;

        // If declaring a label, remove it from the remaining available declarations
        if (lineToken.type == TokenType::LABEL_DEF)
            std::erase(availableLabels, lineToken.value);

        // If the label is not a global, mangle it
        if (std::ranges::find(globals, lineToken.value) == globals.end())
            lineToken.value = lineToken.value + "@" + fileId;
    }
}


void Postprocessor::collectGlobals(std::vector<std::vector<Token>>& tokenizedFile) {
    const Token globlToken = {TokenType::META_DIRECTIVE, "globl"};
    for (int i = 0; i < tokenizedFile.size(); i++) {
        std::vector<Token>& line = tokenizedFile[i];
        if (line[0] != globlToken)
            continue;

        if (line.size() != 2 || line[1].type != TokenType::LABEL_REF)
            throw std::runtime_error("Invalid global label declaration");

        globals.push_back(line[1].value);
        tokenizedFile.erase(tokenizedFile.begin() + i);
        i--;
    }
}


void Postprocessor::replaceEqv(std::vector<std::vector<Token>>& tokenizedFile) {
    const Token eqvToken = {TokenType::META_DIRECTIVE, "eqv"};
    std::unordered_map<Token, std::vector<Token>, Token::HashFunction> eqvMapping;
    for (int i = 0; i < tokenizedFile.size(); i++) {
        std::vector<Token>& line = tokenizedFile[i];
        if (line[0] == eqvToken) {
            if (line.size() < 3 || line[1].type != TokenType::LABEL_REF)
                throw std::runtime_error("Invalid eqv declaration");

            eqvMapping[line[1]] = std::vector(line.begin() + 2, line.end());
            tokenizedFile.erase(tokenizedFile.begin() + i);
            i--;
            continue;
        }

        for (int j = 0; j < line.size(); j++) {
            if (line[j].type != TokenType::LABEL_REF)
                continue;

            auto it = eqvMapping.find(line[j]);
            if (it != eqvMapping.end()) {
                line.erase(line.begin() + j);
                line.insert(line.begin() + j, it->second.begin(), it->second.end());
                j += it->second.size() - 1;
            }
        }
    }
}


void Postprocessor::processBaseAddressing(std::vector<std::vector<Token>>& tokenizedFile) {
    for (std::vector<Token>& tokenLine : tokenizedFile) {
        // Skip instances or parens outside of instructions
        if (tokenLine[0].type != TokenType::INSTRUCTION ||
            tokenLine[tokenLine.size() - 1].type != TokenType::CLOSE_PAREN)
            continue;

        const auto openParen = std::ranges::find(tokenLine, Token{TokenType::OPEN_PAREN, "("});
        // Ensure there is space before and after the open paren
        if (openParen == tokenLine.begin() || openParen == tokenLine.end())
            throw std::runtime_error("Malformed parenthesis expression");
        if (tokenLine.size() < 4)
            throw std::runtime_error("Malformed parenthesis expression");
        // A vector containing the last three elements of tokenLine
        std::vector<Token> lastFour = {};
        while (lastFour.size() < 4) {
            lastFour.insert(lastFour.begin(), tokenLine.back());
            tokenLine.pop_back();
        }

        // Insert 0 when no immediate value precedes parentheses
        if (lastFour[0].type != TokenType::IMMEDIATE) {
            tokenLine.push_back(lastFour[0]);
            lastFour[0] = {TokenType::IMMEDIATE, "0"};
        }

        if (!tokenTypeMatch({TokenType::IMMEDIATE, TokenType::OPEN_PAREN, TokenType::REGISTER,
                             TokenType::CLOSE_PAREN},
                            lastFour))
            throw std::runtime_error("Malformed parenthesis expression");
        // Replace with target pattern
        tokenLine.push_back(lastFour[2]);
        tokenLine.push_back({TokenType::SEPERATOR, ","});
        tokenLine.push_back(lastFour[0]);
    }
}
