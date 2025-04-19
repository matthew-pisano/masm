//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

enum class TokenType {
    UNKNOWN,
    MEMDIRECTIVE,
    DIRECTIVE,
    LABEL,
    LABELREF,
    INSTRUCTION,
    REGISTER,
    IMMEDIATE,
    SEPERATOR,
    STRING,
};


std::string tokenTypeToString(TokenType t);


struct Token {
    TokenType type;
    std::string value;
};


bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


class Tokenizer {
    std::vector<std::string> lines;

public:
    [[nodiscard]] static std::vector<std::vector<Token>>
    tokenize(const std::vector<std::string>& lines);

    static std::vector<std::vector<Token>> tokenizeLine(const std::string& line);
};

#endif // TOKENIZER_H
