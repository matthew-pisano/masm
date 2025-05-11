//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "instruction.h"
#include "postprocessor.h"
#include "utils.h"


/**
 * An array storing the names of memory directives
 */
constexpr std::array<const char*, 14> tokenTypeNames = {
        "UNKNOWN",    "SEC_DIRECTIVE", "ALLOC_DIRECTIVE", "META_DIRECTIVE", "LABEL_DEF",
        "LABEL_REF",  "INSTRUCTION",   "REGISTER",        "IMMEDIATE",      "SEPERATOR",
        "OPEN_PAREN", "CLOSE_PAREN",   "STRING",          "MACRO_PARAM"};


std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


const std::array<std::string, 2> Tokenizer::secDirectives = {"data", "text"};
const std::array<std::string, 4> Tokenizer::metaDirectives = {"globl", "eqv", "macro", "end_macro"};


std::vector<std::vector<Token>>
Tokenizer::tokenize(const std::vector<std::vector<std::string>>& rawFiles) {
    std::map<std::string, std::vector<std::vector<Token>>> programMap;
    Postprocessor postprocessor;
    for (int i = 0; i < rawFiles.size(); ++i) {
        std::vector<std::vector<Token>> fileTokens = tokenizeFile(rawFiles[i]);
        postprocessor.replaceEqv(fileTokens);
        postprocessor.processBaseAddressing(fileTokens);
        postprocessor.processMacros(fileTokens);
        programMap["masm_mangle_file_" + std::to_string(i)] = fileTokens;
    }

    // Mangle labels if there is more than one file
    if (programMap.size() > 1)
        postprocessor.mangleLabels(programMap);

    std::vector<std::vector<Token>> program;
    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap)
        program.insert(program.end(), programFile.second.begin(), programFile.second.end());

    return program;
}


std::vector<std::vector<Token>> Tokenizer::tokenizeFile(const std::vector<std::string>& rawLines) {
    std::vector<std::vector<Token>> tokenizedFile = {};

    for (size_t i = 0; i < rawLines.size(); ++i) {
        const std::string& rawLine = rawLines[i];
        std::vector<std::vector<Token>> tokenizedLines;
        try {
            tokenizedLines = tokenizeLine(rawLine);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error near line " + std::to_string(i + 1) + ": " + e.what());
        }
        // Skip empty or comment lines
        if (tokenizedLines.empty())
            continue;

        for (std::vector<Token>& line : tokenizedLines) {
            if (line.empty())
                continue;

            tokenizedFile.push_back(line);
        }
    }

    return tokenizedFile;
}


std::vector<std::vector<Token>> Tokenizer::tokenizeLine(const std::string& rawLine) {
    const Token eqvToken = {TokenType::META_DIRECTIVE, "eqv"};

    std::vector<std::vector<Token>> tokens = {{}};
    std::string currentToken;
    TokenType currentType = TokenType::UNKNOWN;
    char prevChar = '\0';

    // Append space to ensure all characters are tokenized
    for (const char c : rawLine + ' ') {
        std::vector<Token>& tokenLine = tokens[tokens.size() - 1];
        // When in a string, gather all characters into a single token
        if (currentType == TokenType::STRING) {
            // Terminate the string when reaching unescaped quote
            if (c == '"' && prevChar != '\\') {
                currentType = TokenType::UNKNOWN;
                tokenLine.push_back({TokenType::STRING, currentToken});
                currentToken.clear();
            } else {
                // Add character to the string token
                currentToken += c;
            }

            prevChar = c;
        }
        // Begin a string at an open quote
        else if (c == '"') {
            currentType = TokenType::STRING;
            if (!currentToken.empty())
                throw std::runtime_error("Unexpected token " + currentToken);
        }
        // Skip remainder of line when reaching a comment
        else if (c == '#') {
            break;
        }
        // When reaching a natural token terminator
        else if (isspace(c) || c == ',' || c == ':' || c == '(' || c == ')') {
            terminateToken(c, currentType, currentToken, tokens);
        }
        // When the token type has already been decided
        else if (currentType != TokenType::UNKNOWN) {
            // Accumulate character into the current token
            currentToken += c;
        }
        // A preceding dot marks the token as a directive
        else if (c == '.') {
            currentType = TokenType::ALLOC_DIRECTIVE;
        }
        // A preceding dollar sign marks the token as a register
        else if (c == '$') {
            currentType = TokenType::REGISTER;
        }
        // A preceding percentage sign marks the token as a macro parameter
        else if (c == '%') {
            currentType = TokenType::MACRO_PARAM;
        }
        // Handle Immediates, Instructions, and Label references
        else if (isdigit(c) || c == '-') {
            currentType = TokenType::IMMEDIATE;
            currentToken += c;
        } else {
            // Set first token in line as an instruction, otherwise as a label reference
            // If the third token in an eqv directive line, also set as instruction
            if (tokenLine.empty() || (tokenLine.size() == 2 && tokenLine[0] == eqvToken))
                currentType = TokenType::INSTRUCTION;
            else
                currentType = TokenType::LABEL_REF;
            currentToken += c;
        }
    }

    if (!currentToken.empty())
        throw std::runtime_error("Unexpected EOL while parsing token " + currentToken);

    return tokens;
}


void Tokenizer::terminateToken(const char c, TokenType& currentType, std::string& currentToken,
                               std::vector<std::vector<Token>>& tokens) {
    std::vector<Token>& tokenLine = tokens[tokens.size() - 1];

    // Assign the current token as a label definition if a colon follows
    if (c == ':')
        currentType = TokenType::LABEL_DEF;

    // Reassign directive as section directive if directive is data, text, etc.
    if (currentType == TokenType::ALLOC_DIRECTIVE &&
        std::ranges::find(secDirectives, currentToken) != secDirectives.end())
        currentType = TokenType::SEC_DIRECTIVE;
    // Reassign directive as meta directive if directive is .globl, .macro, etc.
    else if (currentType == TokenType::ALLOC_DIRECTIVE &&
             std::ranges::find(metaDirectives, currentToken) != metaDirectives.end())
        currentType = TokenType::META_DIRECTIVE;

    // Correct for labels put at beginning of line
    if (currentType == TokenType::INSTRUCTION && !isInstruction(currentToken))
        currentType = TokenType::LABEL_REF;

    // Add the current token to the vector and reset
    if (!currentToken.empty()) {
        tokenLine.push_back({currentType, currentToken});
        currentToken.clear();
        currentType = TokenType::UNKNOWN;
    }

    // Start a new line if a label was given
    if (c == ':')
        tokens.emplace_back();
    else if (c == ',')
        tokenLine.push_back({TokenType::SEPERATOR, ","});
    else if (c == '(')
        tokenLine.push_back({TokenType::OPEN_PAREN, "("});
    else if (c == ')')
        tokenLine.push_back({TokenType::CLOSE_PAREN, ")"});
}
