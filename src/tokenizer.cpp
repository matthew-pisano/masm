//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>


/**
 * An array storing the names of memory directives
 */
constexpr std::array<const char*, 10> tokenTypeNames = {
        "UNKNOWN",     "MEMDIRECTIVE", "DIRECTIVE", "LABEL",     "LABELREF",
        "INSTRUCTION", "REGISTER",     "IMMEDIATE", "SEPERATOR", "STRING",
};


std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


std::vector<std::vector<Token>> Tokenizer::tokenize(const std::vector<std::string>& rawLines) {
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

        for (std::vector line : tokenizedLines)
            if (!line.empty())
                tokenizedFile.push_back(line);
    }

    return tokenizedFile;
}


std::vector<std::vector<Token>> Tokenizer::tokenizeLine(const std::string& rawLine) {
    std::array<std::string, 2> memDirectives = {"data", "text"};
    std::vector<std::vector<Token>> tokens = {{}};
    std::string currentToken;
    TokenType currentType = TokenType::UNKNOWN;
    char prevChar = '\0';

    // Append space to ensure all characters are tokenized
    for (const char c : rawLine + ' ') {
        // When in a string, gather all characters into a single token
        if (currentType == TokenType::STRING) {
            // Terminate the string when reaching unescaped quote
            if (c == '"' && prevChar != '\\') {
                currentType = TokenType::UNKNOWN;
                tokens[tokens.size() - 1].push_back({TokenType::STRING, currentToken});
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
        if (isspace(c) || c == ',' || c == ':') {
            // Assign the current token as a label definition if a colon follows
            if (c == ':')
                currentType = TokenType::LABEL;

            // Reassign directive as memory directive if directive is data, text, etc.
            if (currentType == TokenType::DIRECTIVE &&
                std::ranges::find(memDirectives, currentToken) != memDirectives.end())
                currentType = TokenType::MEMDIRECTIVE;

            // Add the current token to the vector and reset
            if (!currentToken.empty()) {
                tokens[tokens.size() - 1].push_back({currentType, currentToken});
                currentToken.clear();
                currentType = TokenType::UNKNOWN;
            }

            // Start a new line if a label was given
            if (c == ':')
                tokens.emplace_back();
            else if (c == ',')
                tokens[tokens.size() - 1].push_back({TokenType::SEPERATOR, ","});

            continue;
        }
        // A preceding dot marks the token as a directive
        if (c == '.' && currentType == TokenType::UNKNOWN) {
            currentType = TokenType::DIRECTIVE;
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
            if (tokens[tokens.size() - 1].empty())
                currentType = TokenType::INSTRUCTION;
            else
                currentType = TokenType::LABELREF;
        }

        // Accumulate character into the current token
        currentToken += c;
        prevChar = c;
    }

    if (!currentToken.empty())
        throw std::runtime_error("Unexpected EOL while parsing token " + currentToken);

    return tokens;
}
