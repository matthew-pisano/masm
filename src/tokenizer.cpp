//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>


constexpr std::array<const char*, 9> tokenTypeNames = {
        "UNKNOWN",  "DIRECTIVE", "LABEL",     "LABELREF", "INSTRUCTION",
        "REGISTER", "IMMEDIATE", "SEPERATOR", "STRING",
};

std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


std::vector<std::vector<Token>> Tokenizer::tokenize() const {
    std::vector<std::vector<Token>> tokenizedFile = {};

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& rawLine = lines[i];
        std::vector<std::vector<Token>> tokenizedLines;
        try {
            tokenizedLines = tokenizeLine(rawLine);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error in line " + std::to_string(i + 1) + ": " + e.what());
        }
        if (tokenizedLines.empty())
            continue;

        tokenizedFile.insert(tokenizedFile.end(), tokenizedLines.begin(), tokenizedLines.end());
    }

    return tokenizedFile;
}


std::vector<std::vector<Token>> Tokenizer::tokenizeLine(const std::string& line) {
    std::vector<std::vector<Token>> tokens = {{}};
    std::string currentToken;
    TokenType currentType = TokenType::UNKNOWN;
    char prevChar = '\0';

    for (char c : (line + ' ')) {
        if (currentType == TokenType::STRING) {
            if (c == '"' && prevChar != '\\') {
                currentType = TokenType::UNKNOWN;
                tokens[tokens.size() - 1].push_back({TokenType::STRING, currentToken});
                currentToken.clear();
            } else {
                currentToken += c;
            }

            prevChar = c;
            continue;
        }

        if (isspace(c) || c == ',' || c == ':') {
            if (c == ':')
                currentType = TokenType::LABEL;

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

            c = '\0';
        } else if (c == '.' && currentType == TokenType::UNKNOWN) {
            currentType = TokenType::DIRECTIVE;
            c = '\0';
        } else if (c == '$' && currentType == TokenType::UNKNOWN) {
            currentType = TokenType::REGISTER;
            c = '\0';
        } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::IMMEDIATE) &&
                   (isdigit(c) || (c == '-' && currentToken.empty()) ||
                    (c == '.' && !currentToken.empty()))) {
            currentType = TokenType::IMMEDIATE;
        } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::INSTRUCTION) &&
                   isalpha(c)) {
            if (tokens[tokens.size() - 1].empty())
                currentType = TokenType::INSTRUCTION;
            else
                currentType = TokenType::LABELREF;
        } else if (c == '"') {
            currentType = TokenType::STRING;
            continue;
        } else if (c == '#') {
            // Handle comments
            break;
        }

        if (c != '\0') {
            currentToken += c;
            prevChar = c;
        }
    }

    if (!currentToken.empty())
        throw std::runtime_error("Unexpected EOL while parsing token " + currentToken);

    if (tokens[tokens.size() - 1].empty())
        tokens.pop_back();

    return tokens;
}
