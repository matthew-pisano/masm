//
// Created by matthew on 4/13/25.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <array>
#include <map>
#include <string>
#include <vector>


/**
 * All valid types for tokens
 */
enum class TokenType {
    UNKNOWN,
    SEC_DIRECTIVE, // Directives that denote the state of a memory section
    ALLOC_DIRECTIVE, // Directives that allocate memory
    META_DIRECTIVE, // Directives that affect the structure of the program (globals, macros, etc.)
    LABEL_DEF,
    LABEL_REF,
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

    struct HashFunction {
        size_t operator()(const Token& token) const {
            const size_t typeHash = std::hash<int>()(static_cast<int>(token.type));
            const size_t valueHash = std::hash<std::string>()(token.value) << 1;
            return typeHash ^ valueHash;
        }
    };
};

bool operator==(const Token& lhs, const Token& rhs);
std::ostream& operator<<(std::ostream& os, const Token& t);


/**
 * Class to tokenize incoming source code lines into parsable tokens
 */
class Tokenizer {

    const static std::array<std::string, 2> secDirectives;
    const static std::array<std::string, 4> metaDirectives;

    /**
     * A helper function that terminates the current token and starts a new one
     * @param c The character that terminated the token
     * @param currentType The type of the current token
     * @param currentToken The current token to terminate
     * @param tokens The vector of tokens to add the current token to
     */
    static void terminateToken(char c, TokenType& currentType, std::string& currentToken,
                               std::vector<std::vector<Token>>& tokens);

    /**
     * A helper function that tokenizes single lines.  Multiple token lines may be produced
     * @param rawLine The line of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating line
     */
    static std::vector<std::vector<Token>> tokenizeLine(const std::string& rawLine);

public:
    /**
     * Tokenizes incoming source code lines into parsable tokens
     * @param rawLines The lines of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating file
     */
    static std::vector<std::vector<Token>> tokenizeFile(const std::vector<std::string>& rawLines);

    /**
     * Tokenizes incoming source code lines from multiple files into parsable tokens
     * @param rawFiles The lines of source code to tokenize
     * @return A vector of vectors of tokens, where each vector represents a tokenized line
     * @throw runtime_error When encountering a malformed or early terminating file
     */
    [[nodiscard]] static std::vector<std::vector<Token>>
    tokenize(const std::vector<std::vector<std::string>>& rawFiles);
};

#endif // TOKENIZER_H
