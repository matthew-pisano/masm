//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>


/**
 * All valid types for tokens
 */
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


/**
 * Returns a string representation of the given token type
 * @param t The token type to parse
 * @return The string representation
 */
std::string tokenTypeToString(TokenType t);


/**
 * Class containing the type and text value of a token
 */
struct Token {
    TokenType type;
    std::string value;
};

bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


/**
 * Class to tokenize incoming source code lines into parsable tokens
 */
class Tokenizer {

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param rawLines The lines of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a line
     * @throw runtime_error When encountering a malformed or early terminating file
     */
    [[nodiscard]] static std::vector<std::vector<Token>>
    tokenize(const std::vector<std::string>& rawLines);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param rawLine The line of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating line
     */
    static std::vector<std::vector<Token>> tokenizeLine(const std::string& rawLine);
};

#endif // TOKENIZER_H
