//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <ostream>
#include <stdexcept>
#include <string>
#include <vector>


std::string tokenTypeToString(TokenType t) { return tokenTypeNames.at(static_cast<int>(t)); }


std::ostream& operator<<(std::ostream& os, const Token& t) {
    return os << "<" << tokenTypeToString(t.type) << ", \"" << t.value << "\">";
}


std::vector<std::vector<Token>> Tokenizer::tokenize() const {
    std::vector<std::vector<Token>> tokenizedLines(lines.size());

    for (size_t i = 0; i < lines.size(); ++i) {
        const std::string& line = lines[i];
        try {
            tokenizedLines[i] = tokenizeLine(line);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Error in line " + std::to_string(i + 1) + ": " + e.what());
        }
    }

    return tokenizedLines;
}


std::vector<Token> Tokenizer::tokenizeLine(const std::string& line) {
    std::vector<Token> tokens;
    std::string currentToken;
    TokenType currentType = TokenType::UNKNOWN;
    char prevChar = '\0';

    for (char c : (line + ' ')) {
        if (currentType == TokenType::STRING) {
            if (c == '"' && prevChar != '\\') {
                currentType = TokenType::UNKNOWN;
                tokens.push_back({TokenType::STRING, currentToken});
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
                tokens.push_back({currentType, currentToken});
                currentToken.clear();
                currentType = TokenType::UNKNOWN;
            }
            if (c == ',')
                tokens.push_back({TokenType::SEPERATOR, ","});

            c = '\0';
        } else if (c == '.') {
            currentType = TokenType::DIRECTIVE;
        } else if (c == '$') {
            currentType = TokenType::REGISTER;
        } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::IMMEDIATE) &&
                   (isdigit(c) || (c == '-' && currentToken.empty()))) {
            currentType = TokenType::IMMEDIATE;
        } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::INSTRUCTION) &&
                   isalpha(c)) {
            if (tokens.empty())
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

    return tokens;
}
