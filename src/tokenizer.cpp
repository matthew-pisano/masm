//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"


/**
 * An array storing the names of memory directives
 */
constexpr std::array<const char*, 13> tokenTypeNames = {
        "UNKNOWN",    "SEC_DIRECTIVE", "ALLOC_DIRECTIVE", "META_DIRECTIVE", "LABEL_DEF",
        "LABEL_REF",  "INSTRUCTION",   "REGISTER",        "IMMEDIATE",      "SEPERATOR",
        "OPEN_PAREN", "CLOSE_PAREN",   "STRING",
};


std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


std::vector<std::vector<Token>>
Tokenizer::tokenize(const std::vector<std::vector<std::string>>& rawFilesLines) {
    std::map<std::string, std::vector<std::vector<Token>>> programMap;
    for (int i = 0; i < rawFilesLines.size(); ++i)
        programMap["masm_mangle_file_" + std::to_string(i)] = tokenizeFile(rawFilesLines[i]);

    // Mangle labels if there is more than one file
    if (programMap.size() > 1)
        mangleLabels(programMap);

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


void Tokenizer::processCloseParen(std::vector<Token>& tokenLine) {
    const auto openParen = std::ranges::find(tokenLine, Token{TokenType::OPEN_PAREN, "("});
    // Ensure there is space before and after the open paren
    if (openParen == tokenLine.begin() || openParen == tokenLine.end())
        throw std::runtime_error("Malformed parenthesis expression");
    if (tokenLine.size() < 3)
        throw std::runtime_error("Malformed parenthesis expression");
    // A vector containing the last three elements of tokenLine
    std::vector<Token> lastThree = {};
    while (lastThree.size() < 3) {
        lastThree.insert(lastThree.begin(), tokenLine.back());
        tokenLine.pop_back();
    }

    // Insert 0 when no immediate value precedes parentheses
    if (lastThree[0].type != TokenType::IMMEDIATE) {
        tokenLine.push_back(lastThree[0]);
        lastThree[0] = {TokenType::IMMEDIATE, "0"};
    }

    if (!tokenTypeMatch({TokenType::IMMEDIATE, TokenType::OPEN_PAREN, TokenType::REGISTER},
                        lastThree))
        throw std::runtime_error("Malformed parenthesis expression");
    // Replace with target pattern
    tokenLine.push_back(lastThree[2]);
    tokenLine.push_back({TokenType::SEPERATOR, ","});
    tokenLine.push_back(lastThree[0]);
}


void Tokenizer::mangleLabels(std::map<std::string, std::vector<std::vector<Token>>>& programMap) {
    // Stores globals that have not yet been matched to declarations
    std::vector availableLabels(globals);

    for (std::pair<const std::string, std::vector<std::vector<Token>>>& programFile : programMap) {
        if (programFile.first.empty())
            throw std::runtime_error("File ID is empty");

        for (std::vector<Token>& line : programFile.second)
            // Mangle the labels in the line
            mangleLabelsInLine(availableLabels, line, programFile.first);
    }

    // If any globals were declared without a matching label declaration
    if (!availableLabels.empty())
        throw std::runtime_error("Global label " + availableLabels[0] +
                                 " referenced without declaration");
}

void Tokenizer::mangleLabelsInLine(std::vector<std::string>& availableLabels,
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


std::vector<std::vector<Token>> Tokenizer::tokenizeLine(const std::string& rawLine) {
    std::array<std::string, 2> secDirectives = {"data", "text"};
    const std::string globlDirective = "globl";
    // If the last token was a global directive and a label is needed
    bool needsGlobl = false;

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
            continue;
        }

        // Skip remainder of line when reaching a comment
        if (c == '#') {
            break;
        }

        // When reaching a natural token terminator
        if (isspace(c) || c == ',' || c == ':' || c == '(' || c == ')') {
            // Assign the current token as a label definition if a colon follows
            if (c == ':')
                currentType = TokenType::LABEL_DEF;

            // Reassign directive as memory directive if directive is data, text, etc.
            if (currentType == TokenType::ALLOC_DIRECTIVE &&
                std::ranges::find(secDirectives, currentToken) != secDirectives.end())
                currentType = TokenType::SEC_DIRECTIVE;
            else if (currentType == TokenType::ALLOC_DIRECTIVE && currentToken == globlDirective) {
                needsGlobl = true;
                currentToken.clear();
                currentType = TokenType::UNKNOWN;
                continue;
            }

            // If marking a label as global, skip token and add to globals
            if (currentType == TokenType::INSTRUCTION && needsGlobl) {
                globals.push_back(currentToken);
                needsGlobl = false;
                currentToken.clear();
                currentType = TokenType::UNKNOWN;
                continue;
            }

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
                processCloseParen(tokenLine);
            continue;
        }
        // A preceding dot marks the token as a directive
        if (c == '.' && currentType == TokenType::UNKNOWN) {
            currentType = TokenType::ALLOC_DIRECTIVE;
            continue;
        }
        // A preceding dollar sign marks the token as a register
        if (c == '$' && currentType == TokenType::UNKNOWN) {
            currentType = TokenType::REGISTER;
            continue;
        }
        // Begin a string at an open quote
        if (c == '"') {
            currentType = TokenType::STRING;
            continue;
        }

        // Handle Immediates, Instructions, and Label references
        if ((currentType == TokenType::UNKNOWN || currentType == TokenType::IMMEDIATE) &&
            (isdigit(c) || (c == '-' && currentToken.empty()) ||
             (c == '.' && !currentToken.empty()))) {
            currentType = TokenType::IMMEDIATE;
        } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::INSTRUCTION) &&
                   isalpha(c)) {
            // Set first token in line as an instruction, otherwise as a label reference
            if (tokenLine.empty())
                currentType = TokenType::INSTRUCTION;
            else
                currentType = TokenType::LABEL_REF;
        }

        // Accumulate character into the current token
        currentToken += c;
        prevChar = c;
    }

    if (!currentToken.empty())
        throw std::runtime_error("Unexpected EOL while parsing token " + currentToken);

    if (needsGlobl)
        throw std::runtime_error("Global directive must be followed by a label");

    return tokens;
}
