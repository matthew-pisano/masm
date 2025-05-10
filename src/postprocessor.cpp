//
// Created by matthew on 5/9/25.
//

#include "postprocessor.h"

#include <stdexcept>


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

        if (tokenizedFile[i][0] != globlToken)
            continue;

        if (tokenizedFile[i].size() != 2 || tokenizedFile[i][1].type != TokenType::LABEL_REF)
            throw std::runtime_error("Invalid global label declaration");

        globals.push_back(tokenizedFile[i][1].value);
        tokenizedFile.erase(tokenizedFile.begin() + i);
        i--;
    }
}
