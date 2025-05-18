//
// Created by matthew on 5/9/25.
//

#include "postprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include "exceptions.h"
#include "utils.h"


void Postprocessor::mangleLabels(std::map<std::string, std::vector<SourceLine>>& programMap) {
    // Stores globals that have not yet been matched to declarations
    std::vector<std::string> globals;
    for (std::pair<const std::string, std::vector<SourceLine>>& programFile : programMap)
        collectGlobals(globals, programFile.second);

    std::vector undeclaredGlobals(globals);
    // Mangle labels and resolve globals
    for (std::pair<const std::string, std::vector<SourceLine>>& programFile : programMap) {
        for (SourceLine& line : programFile.second) {
            // Mangle the labels in the line
            std::string lineDeclaration = mangleLabelsInLine(globals, line, programFile.first);
            // If a label was declared, add it to the list of found declarations
            if (!lineDeclaration.empty())
                std::erase(undeclaredGlobals, lineDeclaration);
        }
    }

    // If any globals were declared without a matching label declaration
    if (!undeclaredGlobals.empty())
        throw MasmSyntaxError("Global label " + undeclaredGlobals[0] +
                              " referenced without declaration");
}


std::string Postprocessor::mangleLabelsInLine(std::vector<std::string>& globals,
                                              SourceLine& lineTokens, const std::string& fileId) {
    std::string lineDeclaration;
    for (Token& lineToken : lineTokens.tokens) {
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
                                   std::vector<SourceLine>& tokenizedFile) {
    const Token globlToken = {TokenType::META_DIRECTIVE, "globl"};
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        SourceLine& line = tokenizedFile[i];
        if (line.tokens[0] != globlToken)
            continue;

        if (line.tokens.size() != 2 || line.tokens[1].type != TokenType::LABEL_REF)
            throw MasmSyntaxError("Invalid global label declaration", line.lineno);

        globals.push_back(line.tokens[1].value);
        tokenizedFile.erase(tokenizedFile.begin() + i);
        i--;
    }
}


void Postprocessor::replaceEqv(std::vector<SourceLine>& tokenizedFile) {
    const Token eqvToken = {TokenType::META_DIRECTIVE, "eqv"};
    std::unordered_map<Token, SourceLine, Token::HashFunction> eqvMapping;
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        SourceLine& line = tokenizedFile[i];
        if (line.tokens[0] == eqvToken) {
            if (line.tokens.size() < 3 || line.tokens[1].type != TokenType::LABEL_REF)
                throw MasmSyntaxError("Invalid eqv declaration", line.lineno);

            const std::vector eqvValue(line.tokens.begin() + 2, line.tokens.end());
            eqvMapping[line.tokens[1]] = {line.lineno, eqvValue};
            tokenizedFile.erase(tokenizedFile.begin() + i);
            i--;
            continue;
        }

        for (size_t j = 0; j < line.tokens.size(); j++) {
            if (line.tokens[j].type != TokenType::LABEL_REF)
                continue;

            auto it = eqvMapping.find(line.tokens[j]);
            if (it != eqvMapping.end()) {
                line.tokens.erase(line.tokens.begin() + j);
                line.tokens.insert(line.tokens.begin() + j, it->second.tokens.begin(),
                                   it->second.tokens.end());
                j += it->second.tokens.size() - 1;
            }
        }
    }
}


void Postprocessor::processBaseAddressing(std::vector<SourceLine>& tokenizedFile) {
    for (SourceLine& tokenLine : tokenizedFile) {
        const auto openParen =
                std::ranges::find(tokenLine.tokens, Token{TokenType::OPEN_PAREN, "("});

        // Skip instances or parens outside of instructions
        if (tokenLine.tokens[0].type != TokenType::INSTRUCTION ||
            openParen == tokenLine.tokens.end())
            continue;

        // Ensure there is space before and after the open paren
        if (openParen == tokenLine.tokens.begin() || tokenLine.tokens.size() < 4)
            throw MasmSyntaxError("Malformed parenthesis expression", tokenLine.lineno);
        // A vector containing the last three elements of tokenLine
        std::vector<Token> lastFour = {};
        while (lastFour.size() < 4) {
            lastFour.insert(lastFour.begin(), tokenLine.tokens.back());
            tokenLine.tokens.pop_back();
        }

        // Insert 0 when no immediate value precedes parentheses
        if (lastFour[0].type != TokenType::IMMEDIATE) {
            tokenLine.tokens.push_back(lastFour[0]);
            lastFour[0] = {TokenType::IMMEDIATE, "0"};
        }

        const std::vector pattern = {TokenType::IMMEDIATE, TokenType::OPEN_PAREN,
                                     TokenType::REGISTER, TokenType::CLOSE_PAREN};
        if (!tokenTypeMatch(pattern, lastFour))
            throw MasmSyntaxError("Malformed parenthesis expression", tokenLine.lineno);
        // Replace with target pattern
        tokenLine.tokens.push_back(lastFour[2]);
        tokenLine.tokens.push_back({TokenType::SEPERATOR, ","});
        tokenLine.tokens.push_back(lastFour[0]);
    }
}


std::vector<Token> Postprocessor::parseMacroParams(const SourceLine& line) {
    // If the macro has no parameters
    if (line.tokens.size() < 3)
        return {};

    if (line.tokens[2].type != TokenType::OPEN_PAREN ||
        line.tokens[line.tokens.size() - 1].type != TokenType::CLOSE_PAREN)
        throw MasmSyntaxError("Malformed macro parameter declaration", line.lineno);

    const std::vector rawParams(line.tokens.begin() + 3, line.tokens.end() - 1);
    std::vector<Token> params = filterTokenList(rawParams, {TokenType::MACRO_PARAM});
    return params;
}


Postprocessor::Macro Postprocessor::mangleMacroLabels(const Macro& macro, const size_t pos) {
    Macro mangledMacro(macro);
    std::vector<Token> macroLabelDefs;
    const std::string posStr = std::to_string(pos);

    // Gather and mangle label definitions first
    for (SourceLine& bodyLine : mangledMacro.body)
        for (Token& bodyToken : bodyLine.tokens)
            if (bodyToken.type == TokenType::LABEL_DEF) {
                macroLabelDefs.emplace_back(TokenType::LABEL_REF, bodyToken.value);
                bodyToken.value = bodyToken.value + "@" + mangledMacro.name + "_" + posStr;
            }

    // Next mangle label references
    for (SourceLine& bodyLine : mangledMacro.body)
        for (Token& bodyToken : bodyLine.tokens)
            if (bodyToken.type == TokenType::LABEL_REF) {
                auto it = std::ranges::find(macroLabelDefs, bodyToken);
                // If label is from outside macro
                if (it == macroLabelDefs.end())
                    continue;

                bodyToken.value = bodyToken.value + "@" + mangledMacro.name + "_" + posStr;
            }

    return mangledMacro;
}


void Postprocessor::expandMacro(const Macro& macro, size_t& pos,
                                std::vector<SourceLine>& tokenizedFile) {
    std::vector<Token> macroArgs;
    if (tokenizedFile[pos].tokens.size() > 1)
        macroArgs = filterTokenList(std::vector(tokenizedFile[pos].tokens.begin() + 2,
                                                tokenizedFile[pos].tokens.end() - 1));

    if (macroArgs.size() != macro.params.size())
        throw MasmSyntaxError("Invalid number of macro arguments", tokenizedFile[pos].lineno);

    const size_t macroEndIdx = pos + macro.body.size();

    Macro mangledMacro = mangleMacroLabels(macro, pos);
    tokenizedFile.insert(tokenizedFile.begin() + pos, mangledMacro.body.begin(),
                         mangledMacro.body.end());
    tokenizedFile.erase(tokenizedFile.begin() + pos + mangledMacro.body.size());

    // Replace macro parameters with arguments
    while (pos < macroEndIdx) {
        for (auto& token : tokenizedFile.at(pos).tokens) {
            if (token.type != TokenType::MACRO_PARAM)
                continue;
            const auto it = std::ranges::find(macro.params, token);
            if (it == macro.params.end())
                throw MasmSyntaxError("Invalid macro parameter " + token.value,
                                      tokenizedFile.at(pos).lineno);
            const size_t paramIdx = std::distance(macro.params.begin(), it);
            // Replace token with argument
            token = macroArgs[paramIdx];
        }
        pos++;
    }
    pos--; // Adjust for the increment at the end of the loop
}


void Postprocessor::processMacros(std::vector<SourceLine>& tokenizedFile) {
    std::unordered_map<std::string, Macro> macroMap;
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        SourceLine& line = tokenizedFile[i];
        if (line.tokens[0].type == TokenType::META_DIRECTIVE && line.tokens[0].value == "macro") {
            const size_t macroStart = i;
            if (line.tokens.size() < 2 || line.tokens[1].type != TokenType::LABEL_REF)
                throw MasmSyntaxError("Invalid macro declaration", line.lineno);

            Macro macro = {line.tokens[1].value, {}, {}};
            macro.params = parseMacroParams(line);
            while (true) {
                i++;
                if (i >= tokenizedFile.size())
                    throw MasmSyntaxError("Unmatched macro declaration", line.lineno);

                if (tokenizedFile[i].tokens[0].type == TokenType::META_DIRECTIVE &&
                    tokenizedFile[i].tokens[0].value == "end_macro")
                    break;

                if (tokenizedFile[i].tokens[0].type == TokenType::LABEL_REF &&
                    macroMap.contains(tokenizedFile[i].tokens[0].value))
                    expandMacro(macroMap[tokenizedFile[i].tokens[0].value], i, tokenizedFile);
            }
            macro.body =
                    std::vector(tokenizedFile.begin() + macroStart + 1, tokenizedFile.begin() + i);
            macroMap[macro.name] = macro;
            tokenizedFile.erase(tokenizedFile.begin() + macroStart, tokenizedFile.begin() + i + 1);
            i = macroStart - 1;
        } else if (line.tokens[0].type == TokenType::LABEL_REF &&
                   macroMap.contains(line.tokens[0].value))
            expandMacro(macroMap[line.tokens[0].value], i, tokenizedFile);
    }
}


void Postprocessor::processIncludes(std::map<std::string, std::vector<SourceLine>>& rawProgramMap) {
    const Token includeToken = {TokenType::META_DIRECTIVE, "include"};

    for (auto& [fileName, tokenizedFile] : rawProgramMap) {
        for (size_t i = 0; i < tokenizedFile.size(); i++) {
            SourceLine& line = tokenizedFile[i];
            if (line.tokens[0] != includeToken)
                continue;
            if (line.tokens.size() != 2 || line.tokens[1].type != TokenType::STRING)
                throw MasmSyntaxError("Invalid include directive", line.lineno);

            std::string includeName = line.tokens[1].value;
            const std::vector<SourceLine>& includeFile = rawProgramMap[includeName];
            // Insert contents of included file into location at including file
            tokenizedFile.insert(tokenizedFile.begin() + i, includeFile.begin(), includeFile.end());
            // Remove the include line
            tokenizedFile.erase(tokenizedFile.begin() + i + includeFile.size());
        }
    }
}
