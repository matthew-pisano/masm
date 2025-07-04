//
// Created by matthew on 5/9/25.
//

#include "tokenizer/postprocessor.h"

#include <algorithm>
#include <stdexcept>
#include <unordered_map>

#include "exceptions.h"
#include "utils.h"


std::string mangleLabel(const std::string& label, const std::string& filename) {
    return std::format("{}@masm_mangle_file_{}", label, filename);
}


std::string mangleMacroLabel(const std::string& label, const std::string& filename,
                             const std::string& macroname, size_t pos) {
    return std::format("{}@masm_mangle_file_{}:{}:{}", label, filename, macroname, pos);
}


std::string unmangleLabel(const std::string& mangledLabel) {
    // Find the last '@' in the mangled label
    const size_t atPos = mangledLabel.find_last_of('@');
    if (atPos == std::string::npos)
        return mangledLabel; // No mangling found, return original label

    // Return the label part before the '@'
    return mangledLabel.substr(0, atPos);
}


void Postprocessor::mangleLabels(std::map<std::string, std::vector<LineTokens>>& programMap) {
    // Stores globals that have not yet been matched to declarations
    std::vector<std::pair<std::string, LineTokens>> globals;
    for (std::pair<const std::string, std::vector<LineTokens>>& programFile : programMap)
        collectGlobals(globals, programFile.second);

    // Create vector of just names for globals
    std::vector<std::string> globalNames = {};
    globalNames.reserve(globals.size());
    for (const std::pair<std::string, LineTokens>& global : globals)
        globalNames.push_back(global.first);

    std::vector undeclaredGlobals(globals);
    // Mangle labels and resolve globals
    for (std::pair<const std::string, std::vector<LineTokens>>& programFile : programMap) {
        for (LineTokens& line : programFile.second) {
            // Mangle the labels in the line
            std::string lineDeclaration = mangleLabelsInLine(globalNames, line, programFile.first);
            // If a label was declared, add it to the list of found declarations
            if (!lineDeclaration.empty())
                for (size_t i = 0; i < undeclaredGlobals.size(); i++) {
                    if (std::get<0>(undeclaredGlobals[i]) == lineDeclaration) {
                        undeclaredGlobals.erase(undeclaredGlobals.begin() + i);
                        break;
                    }
                }
        }
    }

    // If any globals were declared without a matching label declaration
    if (!undeclaredGlobals.empty()) {
        const std::string labelName = std::get<0>(undeclaredGlobals[0]);
        const std::string filename = std::get<1>(undeclaredGlobals[0]).filename;
        const size_t lineno = std::get<1>(undeclaredGlobals[0]).lineno;
        throw MasmSyntaxError("Global label '" + labelName + "' referenced without declaration",
                              filename, lineno);
    }
}


std::string Postprocessor::mangleLabelsInLine(std::vector<std::string>& globals,
                                              LineTokens& lineTokens, const std::string& fileId) {
    std::string lineDeclaration;
    for (Token& lineToken : lineTokens.tokens) {
        if (lineToken.category != TokenCategory::LABEL_DEF &&
            lineToken.category != TokenCategory::LABEL_REF)
            continue;

        // If declaring a label, remove it from the remaining available declarations
        if (lineToken.category == TokenCategory::LABEL_DEF)
            lineDeclaration = lineToken.value;

        // If the label is not a global, mangle it
        if (std::ranges::find(globals, lineToken.value) == globals.end())
            lineToken.value = mangleLabel(lineToken.value, fileId);
    }

    return lineDeclaration;
}


void Postprocessor::collectGlobals(std::vector<std::pair<std::string, LineTokens>>& globals,
                                   std::vector<LineTokens>& tokenizedFile) {
    const Token globlToken = {TokenCategory::META_DIRECTIVE, "globl"};
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        LineTokens& line = tokenizedFile[i];
        if (line.tokens[0] != globlToken)
            continue;

        if (line.tokens.size() != 2 || line.tokens[1].category != TokenCategory::LABEL_REF)
            throw MasmSyntaxError("Invalid global label declaration", line.filename, line.lineno);

        globals.emplace_back(line.tokens[1].value, line);
        // Remove the global declaration line from the file
        tokenizedFile.erase(tokenizedFile.begin() + i);
        i--;
    }
}


void Postprocessor::replaceEqv(std::vector<LineTokens>& tokenizedFile) {
    const Token eqvToken = {TokenCategory::META_DIRECTIVE, "eqv"};
    std::unordered_map<Token, LineTokens, Token::HashFunction> eqvMapping;
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        LineTokens& line = tokenizedFile[i];
        if (line.tokens[0] == eqvToken) {
            if (line.tokens.size() < 3 || line.tokens[1].category != TokenCategory::LABEL_REF)
                throw MasmSyntaxError("Invalid eqv declaration", line.filename, line.lineno);

            const std::vector eqvValue(line.tokens.begin() + 2, line.tokens.end());
            eqvMapping[line.tokens[1]] = {line.filename, line.lineno, eqvValue};
            // Remove the eqv declaration line from the file
            tokenizedFile.erase(tokenizedFile.begin() + i);
            i--;
            continue;
        }

        // Replace any label references with their eqv values
        for (size_t j = 0; j < line.tokens.size(); j++) {
            if (line.tokens[j].category != TokenCategory::LABEL_REF)
                continue;

            auto it = eqvMapping.find(line.tokens[j]);
            if (it != eqvMapping.end()) {
                // Replace the label reference with the eqv value
                line.tokens.erase(line.tokens.begin() + j);
                line.tokens.insert(line.tokens.begin() + j, it->second.tokens.begin(),
                                   it->second.tokens.end());
                j += it->second.tokens.size() - 1;
            }
        }
    }
}


void Postprocessor::processBaseAddressing(std::vector<LineTokens>& tokenizedFile) {
    for (LineTokens& tokenLine : tokenizedFile) {
        const auto openParen =
                std::ranges::find(tokenLine.tokens, Token{TokenCategory::OPEN_PAREN, "("});

        // Skip instances or parens outside of instructions
        if (tokenLine.tokens[0].category != TokenCategory::INSTRUCTION ||
            openParen == tokenLine.tokens.end())
            continue;

        // Ensure there is space before and after the open paren
        if (openParen == tokenLine.tokens.begin() || tokenLine.tokens.size() < 4)
            throw MasmSyntaxError("Malformed parenthesis expression", tokenLine.filename,
                                  tokenLine.lineno);
        // A vector containing the last three elements of tokenLine
        std::vector<Token> lastFour = {};
        while (lastFour.size() < 4) {
            lastFour.insert(lastFour.begin(), tokenLine.tokens.back());
            tokenLine.tokens.pop_back();
        }

        // Insert 0 when no immediate value precedes parentheses
        if (lastFour[0].category != TokenCategory::IMMEDIATE) {
            tokenLine.tokens.push_back(lastFour[0]);
            lastFour[0] = {TokenCategory::IMMEDIATE, "0"};
        }

        const std::vector pattern = {TokenCategory::IMMEDIATE, TokenCategory::OPEN_PAREN,
                                     TokenCategory::REGISTER, TokenCategory::CLOSE_PAREN};
        if (!tokenCategoryMatch(pattern, lastFour))
            throw MasmSyntaxError("Malformed parenthesis expression", tokenLine.filename,
                                  tokenLine.lineno);
        // Replace with target pattern
        tokenLine.tokens.push_back(lastFour[2]);
        tokenLine.tokens.push_back({TokenCategory::SEPERATOR, ","});
        tokenLine.tokens.push_back(lastFour[0]);
    }
}


std::vector<Token> Postprocessor::parseMacroParams(const LineTokens& line) {
    // If the macro has no parameters
    if (line.tokens.size() < 3)
        return {};

    if (line.tokens[2].category != TokenCategory::OPEN_PAREN ||
        line.tokens[line.tokens.size() - 1].category != TokenCategory::CLOSE_PAREN)
        throw MasmSyntaxError("Malformed macro parameter declaration", line.filename, line.lineno);

    const std::vector rawParams(line.tokens.begin() + 3, line.tokens.end() - 1);
    std::vector<Token> params = filterTokenList(rawParams, {TokenCategory::MACRO_PARAM});
    return params;
}


Postprocessor::Macro Postprocessor::mangleMacroLabels(const Macro& macro, const size_t pos) {
    Macro mangledMacro(macro);
    std::vector<Token> macroLabelDefs;

    // Gather and mangle label definitions first
    for (LineTokens& bodyLine : mangledMacro.body)
        for (Token& bodyToken : bodyLine.tokens)
            if (bodyToken.category == TokenCategory::LABEL_DEF) {
                macroLabelDefs.emplace_back(TokenCategory::LABEL_REF, bodyToken.value);
                bodyToken.value = mangleMacroLabel(bodyToken.value, mangledMacro.filename,
                                                   mangledMacro.name, pos);
            }

    // Next mangle label references
    for (LineTokens& bodyLine : mangledMacro.body)
        for (Token& bodyToken : bodyLine.tokens)
            if (bodyToken.category == TokenCategory::LABEL_REF) {
                auto it = std::ranges::find(macroLabelDefs, bodyToken);
                // If label is from outside macro
                if (it == macroLabelDefs.end())
                    continue;

                bodyToken.value = mangleMacroLabel(bodyToken.value, mangledMacro.filename,
                                                   mangledMacro.name, pos);
            }

    return mangledMacro;
}


void Postprocessor::expandMacro(const Macro& macro, size_t& pos,
                                std::vector<LineTokens>& tokenizedFile) {
    std::vector<Token> macroArgs;
    if (tokenizedFile[pos].tokens.size() > 1)
        macroArgs = filterTokenList(std::vector(tokenizedFile[pos].tokens.begin() + 2,
                                                tokenizedFile[pos].tokens.end() - 1));

    if (macroArgs.size() != macro.params.size())
        throw MasmSyntaxError("Invalid number of macro arguments", tokenizedFile[pos].filename,
                              tokenizedFile[pos].lineno);

    const size_t macroEndIdx = pos + macro.body.size();

    Macro mangledMacro = mangleMacroLabels(macro, pos);
    // Insert the mangled macro body into the tokenized file
    tokenizedFile.insert(tokenizedFile.begin() + pos, mangledMacro.body.begin(),
                         mangledMacro.body.end());
    // Remove the macro call line
    tokenizedFile.erase(tokenizedFile.begin() + pos + mangledMacro.body.size());

    // Replace macro parameters with arguments
    while (pos < macroEndIdx) {
        for (auto& token : tokenizedFile.at(pos).tokens) {
            if (token.category != TokenCategory::MACRO_PARAM)
                continue;
            const auto it = std::ranges::find(macro.params, token);
            if (it == macro.params.end())
                throw MasmSyntaxError("Invalid macro parameter '" + token.value + "'",
                                      tokenizedFile.at(pos).filename, tokenizedFile.at(pos).lineno);
            const size_t paramIdx = std::distance(macro.params.begin(), it);
            // Replace token with argument
            token = macroArgs[paramIdx];
        }
        pos++;
    }
    pos--; // Adjust for the increment at the end of the loop
}


void Postprocessor::processMacros(std::vector<LineTokens>& tokenizedFile) {
    std::unordered_map<std::string, Macro> macroMap;
    for (size_t i = 0; i < tokenizedFile.size(); i++) {
        LineTokens& line = tokenizedFile[i];
        // If the line is a macro declaration
        if (line.tokens[0].category == TokenCategory::META_DIRECTIVE &&
            line.tokens[0].value == "macro") {
            const size_t macroStart = i;
            if (line.tokens.size() < 2 || line.tokens[1].category != TokenCategory::LABEL_REF)
                throw MasmSyntaxError("Invalid macro declaration", line.filename, line.lineno);

            Macro macro = {line.tokens[1].value, {}, {}, line.filename};
            // If the macro has parameters, parse them
            macro.params = parseMacroParams(line);
            // Process the macro body until the end_macro directive
            while (true) {
                i++;
                // If we reach the end of the file without finding an end_macro directive
                if (i >= tokenizedFile.size())
                    throw MasmSyntaxError("Unmatched macro declaration", line.filename,
                                          line.lineno);

                // If we find an end_macro directive, break out of the loop
                if (tokenizedFile[i].tokens[0].category == TokenCategory::META_DIRECTIVE &&
                    tokenizedFile[i].tokens[0].value == "end_macro")
                    break;

                // If we find a label reference that matches a macro, expand it
                if (tokenizedFile[i].tokens[0].category == TokenCategory::LABEL_REF &&
                    macroMap.contains(tokenizedFile[i].tokens[0].value))
                    expandMacro(macroMap[tokenizedFile[i].tokens[0].value], i, tokenizedFile);
            }
            // Store the macro in the map and remove the macro declaration from the file
            macro.body =
                    std::vector(tokenizedFile.begin() + macroStart + 1, tokenizedFile.begin() + i);
            macroMap[macro.name] = macro;
            tokenizedFile.erase(tokenizedFile.begin() + macroStart, tokenizedFile.begin() + i + 1);
            i = macroStart - 1;
        }
        // If the line is a label reference that matches a macro, expand it
        else if (line.tokens[0].category == TokenCategory::LABEL_REF &&
                 macroMap.contains(line.tokens[0].value))
            expandMacro(macroMap[line.tokens[0].value], i, tokenizedFile);
    }
}


void Postprocessor::processIncludes(std::map<std::string, std::vector<LineTokens>>& rawProgramMap) {
    const Token includeToken = {TokenCategory::META_DIRECTIVE, "include"};

    for (auto& [fileName, tokenizedFile] : rawProgramMap) {
        for (size_t i = 0; i < tokenizedFile.size(); i++) {
            LineTokens& line = tokenizedFile[i];
            if (line.tokens[0] != includeToken)
                continue;
            if (line.tokens.size() != 2 || line.tokens[1].category != TokenCategory::STRING)
                throw MasmSyntaxError("Invalid include directive", line.filename, line.lineno);

            std::string includeName = line.tokens[1].value;
            const std::vector<LineTokens>& includeFile = rawProgramMap[includeName];
            // Insert contents of included file into location at including file
            tokenizedFile.insert(tokenizedFile.begin() + i, includeFile.begin(), includeFile.end());
            // Remove the include line
            tokenizedFile.erase(tokenizedFile.begin() + i + includeFile.size());
        }
    }
}
