//
// Created by matthew on 4/13/25.
//

#include "tokenizer.h"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>


enum class TokenType {
    UNKNOWN,
    DIRECTIVE,
    LABEL,
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    SEPERATOR,
    STRING,
};


struct Token {
    TokenType type;
    std::string value;
};


class Tokenizer {
    std::vector<std::string> lines;

    explicit Tokenizer(const std::vector<std::string>& lines) : lines(lines) {}

    std::vector<std::vector<Token>> tokenize() {
        std::vector<std::vector<Token>> tokenizedLines(lines.size());

        for (size_t i = 0; i < lines.size(); ++i) {
            const std::string& line = lines[i];
            try {
                tokenizedLines[i] = tokenizeLine(line);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Error in line " + std::to_string(i + 1) + ": " +
                                         e.what());
            }
        }

        return tokenizedLines;
    }

    static std::vector<Token> tokenizeLine(const std::string& line) {
        std::vector<Token> tokens;
        std::string currentToken;
        TokenType currentType = TokenType::UNKNOWN;
        char prevChar = '\0';

        for (const char c : (line + ' ')) {
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
            } else if (c == '.') {
                currentType = TokenType::DIRECTIVE;
            } else if (c == '$') {
                currentType = TokenType::REGISTER;
            } else if ((currentType == TokenType::UNKNOWN || currentType == TokenType::IMMEDIATE) &&
                       (isdigit(c) || (c == '-' && currentToken.empty()))) {
                currentType = TokenType::IMMEDIATE;
            } else if ((currentType == TokenType::UNKNOWN ||
                        currentType == TokenType::INSTRUCTION) &&
                       isalpha(c)) {
                currentType = TokenType::INSTRUCTION;
            } else if (c == '"') {
                currentType = TokenType::STRING;
                continue;
            } else if (c == '#') {
                // Handle comments
                break;
            }

            currentToken += c;
            prevChar = c;
        }

        if (!currentToken.empty())
            throw std::runtime_error("Unexpected EOL while parsing token " + currentToken);

        return tokens;
    }
};
