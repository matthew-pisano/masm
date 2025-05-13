//
// Created by matthew on 5/9/25.
//

#include "postprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include "utils.h"


void Postprocessor::mangleLabels(
        std::map<std::string, std::vector<std::vector<Token>>>& programMap) {

    // Stores globals that have not yet been matched to declarations
    std::vector<std::string> globals;
    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap) {
        if (programFile.first.empty())
            throw std::runtime_error("File ID is empty");
        collectGlobals(globals, programFile.second);
    }

    std::vector undeclaredGlobals(globals);
    // Mangle labels and resolve globals
    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap) {
        for (std::vector<Token>& line : programFile.second) {
            // Mangle the labels in the line
            std::string lineDeclaration = mangleLabelsInLine(globals, line, programFile.first);
            // If a label was declared, add it to the list of found declarations
            if (!lineDeclaration.empty())
                std::erase(undeclaredGlobals, lineDeclaration);
        }
    }

    // If any globals were declared without a matching label declaration
    if (!undeclaredGlobals.empty())
        throw std::runtime_error("Global label " + undeclaredGlobals[0] +
                                 " referenced without declaration");
}


std::string Postprocessor::mangleLabelsInLine(std::vector<std::string>& globals,
                                              std::vector<Token>& lineTokens,
                                              const std::string& fileId) {
    std::string lineDeclaration;
    for (Token& lineToken : lineTokens) {
        if (lineToken.type != TokenType::LABEL_DEF && lineToken.type != TokenType::LABEL_REF)
            continue;

        // If declaring a label, remove it from the remaining available declarations
        if (lineToken.type == TokenType::LABEL_DEF)
            lineDeclaration = lineToken.value;

        // If the label is not a global, mangle it
        if (std::ranges::find(globals, lineToken.value) == globals.end())
            lineToken.value = lineToken.value + "@" + fileId;
    }

    return lineDeclaration;
}


void Postprocessor::collectGlobals(std::vector<std::string>& globals,
                                   std::vector<std::vector<Token>>& tokenizedFile) {
    const Token globlToken = {TokenType::META_DIRECTIVE, "globl"};
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
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
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        std::vector<Token>& line = tokenizedFile[i];
        if (line[0] == eqvToken) {
            if (line.size() < 3 || line[1].type != TokenType::LABEL_REF)
                throw std::runtime_error("Invalid eqv declaration");

            eqvMapping[line[1]] = std::vector(line.begin() + 2, line.end());
            tokenizedFile.erase(tokenizedFile.begin() + i);
            i--;
            continue;
        }

        for (size_t j = 0; j < line.size(); j++) {
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


std::vector<Token> Postprocessor::parseMacroParams(const std::vector<Token>& line) {
    // If the macro has no parameters
    if (line.size() < 3)
        return {};

    if (line[2].type != TokenType::OPEN_PAREN)
        throw std::runtime_error("Malformed macro parameter declaration");
    if (line[line.size() - 1].type != TokenType::CLOSE_PAREN)
        throw std::runtime_error("Malformed macro parameter declaration");

    const std::vector rawParams(line.begin() + 3, line.end() - 1);
    std::vector<Token> params = filterTokenList(rawParams, {TokenType::MACRO_PARAM});
    return params;
}


Postprocessor::Macro Postprocessor::mangleMacroLabels(const Macro& macro, const size_t pos) {
    Macro mangledMacro(macro);
    std::vector<Token> macroLabelDefs;
    const std::string posStr = std::to_string(pos);

    for (std::vector<Token>& bodyLine : mangledMacro.body) {
        for (Token& bodyToken : bodyLine) {
            if (bodyToken.type == TokenType::LABEL_DEF) {
                macroLabelDefs.emplace_back(TokenType::LABEL_REF, bodyToken.value);
                bodyToken.value = bodyToken.value + "@" + mangledMacro.name + "_" + posStr;
            } else if (bodyToken.type == TokenType::LABEL_REF) {
                auto it = std::ranges::find(macroLabelDefs, bodyToken);
                if (it == macroLabelDefs.end())
                    continue;

                bodyToken.value = bodyToken.value + "@" + mangledMacro.name + "_" + posStr;
            }
        }
    }

    return mangledMacro;
}


void Postprocessor::expandMacro(const Macro& macro, size_t& pos,
                                std::vector<std::vector<Token>>& tokenizedFile) {
    std::vector<Token> macroArgs;
    if (tokenizedFile[pos].size() > 1)
        macroArgs = filterTokenList(
                std::vector(tokenizedFile[pos].begin() + 2, tokenizedFile[pos].end() - 1));

    if (macroArgs.size() != macro.params.size())
        throw std::runtime_error("Invalid number of macro arguments");

    const size_t macroEndIdx = pos + macro.body.size();

    Macro mangledMacro = mangleMacroLabels(macro, pos);
    tokenizedFile.insert(tokenizedFile.begin() + pos, mangledMacro.body.begin(),
                         mangledMacro.body.end());
    tokenizedFile.erase(tokenizedFile.begin() + pos + mangledMacro.body.size());

    // Replace macro parameters with arguments
    while (pos < macroEndIdx - 1) {
        for (auto& token : tokenizedFile[pos]) {
            if (token.type != TokenType::MACRO_PARAM)
                continue;
            const auto it = std::ranges::find(macro.params, token);
            if (it == macro.params.end())
                throw std::runtime_error("Invalid macro parameter " + token.value);
            const size_t paramIdx = std::distance(macro.params.begin(), it);
            // Replace token with argument
            token = macroArgs[paramIdx];
        }
        pos++;
    }
}


void Postprocessor::processMacros(std::vector<std::vector<Token>>& tokenizedFile) {
    std::unordered_map<std::string, Macro> macroMap;
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        std::vector<Token>& line = tokenizedFile[i];
        if (line[0].type == TokenType::META_DIRECTIVE && line[0].value == "macro") {
            const size_t macroStart = i;
            if (line.size() < 2 || line[1].type != TokenType::LABEL_REF)
                throw std::runtime_error("Invalid macro declaration");

            Macro macro = {line[1].value, {}, {}};
            macro.params = parseMacroParams(line);
            while (true) {
                i++;
                if (i >= tokenizedFile.size())
                    throw std::runtime_error("Unmatched macro declaration");

                if (tokenizedFile[i][0].type == TokenType::META_DIRECTIVE &&
                    tokenizedFile[i][0].value == "end_macro")
                    break;

                if (tokenizedFile[i][0].type == TokenType::LABEL_REF &&
                    macroMap.contains(tokenizedFile[i][0].value))
                    expandMacro(macroMap[tokenizedFile[i][0].value], i, tokenizedFile);
                else
                    macro.body.push_back(tokenizedFile[i]);
            }
            macroMap[macro.name] = macro;
            tokenizedFile.erase(tokenizedFile.begin() + macroStart, tokenizedFile.begin() + i + 1);
            i = macroStart - 1;
        } else if (line[0].type == TokenType::LABEL_REF && macroMap.contains(line[0].value))
            expandMacro(macroMap[line[0].value], i, tokenizedFile);
    }
}
