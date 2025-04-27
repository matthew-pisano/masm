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
    OPEN_PAREN,
    CLOSE_PAREN,
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

    /**
     * All global labels within the file
     */
    std::vector<std::string> globals;

    /**
     * Modifies the give token line to replace the pattern of y($xx) with $xx, y to match MIPS
     * addressing mode when a close paren is reached
     * @param tokenLine The line of tokens to modify
     */
    static void processCloseParen(std::vector<Token>& tokenLine);

    /**
     * Name mangels tokens in the given line that are not globals
     * @param lineTokens A line of tokens
     * @param fileId The name of the file being tokenized
     * @throw runtime_error When the file ID is empty
     */
    void mangleLabels(std::vector<Token>& lineTokens, const std::string& fileId);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param rawLine The line of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating line
     */
    std::vector<std::vector<Token>> tokenizeLine(const std::string& rawLine);

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param rawLines The lines of source code to tokenize
     * @param fileName The name of the file being tokenized, used for tracing and name mangling
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating file
     */
    [[nodiscard]] std::vector<std::vector<Token>> tokenize(const std::vector<std::string>& rawLines,
                                                           const std::string& fileName = "");
};

#endif // TOKENIZER_H
